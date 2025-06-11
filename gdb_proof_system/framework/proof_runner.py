#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
GDB Proof Runner - Orchestrates deterministic program verification
"""

import subprocess
import sys
import os
import json
import time
from pathlib import Path
from typing import List, Dict, Tuple

class ProofRunner:
    def __init__(self, binary_path: str):
        self.binary_path = Path(binary_path).absolute()
        self.proof_dir = Path(__file__).parent.parent / "proofs"
        self.results = []
        self.start_time = time.time()
        
    def run_proof(self, proof_file: Path) -> Tuple[bool, str]:
        """Run a single GDB proof and return (passed, output)"""
        print(f"\n[Running] {proof_file.name}")
        
        cmd = [
            "gdb",
            "-batch",
            "-x", str(Path(__file__).parent / "proof_base.gdb"),
            "-x", str(proof_file),
            str(self.binary_path)
        ]
        
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=30  # 30 second timeout per proof
            )
            
            output = result.stdout + result.stderr
            passed = result.returncode == 0
            
            return passed, output
            
        except subprocess.TimeoutExpired:
            return False, "TIMEOUT: Proof took too long"
        except Exception as e:
            return False, f"ERROR: {str(e)}"
    
    def run_all_proofs(self) -> bool:
        """Run all proofs in order"""
        proof_files = sorted(self.proof_dir.glob("*.gdb"))
        
        if not proof_files:
            print("ERROR: No proof files found")
            return False
            
        print(f"Found {len(proof_files)} proofs to run")
        print(f"Binary: {self.binary_path}")
        print("=" * 60)
        
        all_passed = True
        
        for proof_file in proof_files:
            passed, output = self.run_proof(proof_file)
            
            self.results.append({
                "proof": proof_file.name,
                "passed": passed,
                "output": output
            })
            
            if passed:
                print(f"✓ PASS: {proof_file.name}")
            else:
                print(f"✗ FAIL: {proof_file.name}")
                all_passed = False
                
                # Show failure details
                for line in output.split('\n'):
                    if 'FAIL' in line or 'ERROR' in line:
                        print(f"  {line}")
        
        return all_passed
    
    def generate_report(self) -> Dict:
        """Generate proof report"""
        duration = time.time() - self.start_time
        passed_count = sum(1 for r in self.results if r["passed"])
        
        report = {
            "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
            "binary": str(self.binary_path),
            "duration_seconds": round(duration, 2),
            "total_proofs": len(self.results),
            "passed": passed_count,
            "failed": len(self.results) - passed_count,
            "results": self.results,
            "verdict": "VERIFIED" if passed_count == len(self.results) else "FAILED"
        }
        
        return report
    
    def save_report(self, output_path: str = "proof_report.json"):
        """Save report to file"""
        report = self.generate_report()
        
        with open(output_path, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"\nReport saved to: {output_path}")
        return report

def main():
    if len(sys.argv) < 2:
        print("Usage: proof_runner.py <binary_path> [report_output]")
        sys.exit(1)
    
    binary_path = sys.argv[1]
    report_path = sys.argv[2] if len(sys.argv) > 2 else "proof_report.json"
    
    # Check binary exists
    if not Path(binary_path).exists():
        print(f"ERROR: Binary not found: {binary_path}")
        sys.exit(1)
    
    # Run proofs
    runner = ProofRunner(binary_path)
    all_passed = runner.run_all_proofs()
    
    # Generate report
    report = runner.save_report(report_path)
    
    # Print summary
    print("\n" + "=" * 60)
    print("PROOF SUMMARY")
    print("=" * 60)
    print(f"Total proofs: {report['total_proofs']}")
    print(f"Passed: {report['passed']}")
    print(f"Failed: {report['failed']}")
    print(f"Duration: {report['duration_seconds']}s")
    print(f"Verdict: {report['verdict']}")
    
    if report['verdict'] == "VERIFIED":
        print("\n✓ PROGRAM VERIFIED: No core dumps possible")
        sys.exit(0)
    else:
        print("\n✗ VERIFICATION FAILED: Program may core dump")
        sys.exit(1)

if __name__ == "__main__":
    main()