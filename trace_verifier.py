#!/usr/bin/env python3
"""Verify that code execution traces meet specifications."""

import json
import subprocess
import sys
from pathlib import Path

class TraceVerifier:
    def __init__(self, spec_file, source_file):
        self.spec_file = spec_file
        self.source_file = source_file
        self.traces = {}
        
    def load_specifications(self):
        """Load trace specifications from JSON."""
        with open(self.spec_file) as f:
            return json.load(f)
            
    def build_trace_binary(self, spec_id, enabled_points):
        """Build a binary that records trace of execution."""
        output = f"trace_{spec_id.lower()}"
        
        # Extract all code points
        all_points = self.extract_code_points()
        
        defines = ["-DTRACING_BUILD"]
        for point in all_points:
            if point in enabled_points:
                defines.append(f"-DPATH_{point}_EXISTS=1")
            else:
                defines.append(f"-DPATH_{point}_EXISTS=0")
                
        cmd = [
            "gcc", "-std=c99", "-I./include",
            *defines,
            self.source_file,
            "-o", output,
            "-lm"
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"Build failed: {result.stderr}")
            return None
            
        return output
        
    def extract_code_points(self):
        """Extract all SECURE_CODE_POINT names from source."""
        points = set()
        with open(self.source_file) as f:
            import re
            content = f.read()
            matches = re.findall(r'SECURE_CODE_POINT\((\w+),', content)
            points.update(matches)
        return sorted(list(points))
        
    def run_trace(self, binary, scenario):
        """Run binary and collect trace of secure points hit."""
        env = {"TEST_SCENARIO": scenario}
        result = subprocess.run(
            [f"./{binary}"], 
            env={**env, "TRACE_MODE": "1"},
            capture_output=True, 
            text=True
        )
        
        # Extract trace from output
        trace = []
        for line in result.stdout.split('\n'):
            if line.startswith("TRACE:"):
                parts = line.split(':')
                if len(parts) >= 3:
                    point = parts[1].strip()
                    sha3 = parts[2].strip()
                    trace.append({"point": point, "sha3": sha3})
            elif line.startswith("SECURE_EXIT:"):
                point = line.split(':')[1].strip()
                trace.append({"point": point, "exit": True})
                
        return trace
        
    def verify_specification(self, spec):
        """Verify a single specification."""
        spec_id = spec["id"]
        print(f"\nVerifying {spec_id}: {spec['description']}")
        
        # Get all required points
        required_points = [p["point"] for p in spec["required_trace"]]
        forbidden_points = [p["point"] for p in spec.get("forbidden_trace", [])]
        
        # Build binary with forbidden points disabled
        enabled_points = self.extract_code_points()
        for forbidden in forbidden_points:
            if forbidden in enabled_points:
                enabled_points.remove(forbidden)
                
        binary = self.build_trace_binary(spec_id, enabled_points)
        if not binary:
            return False
            
        # Run all test scenarios
        all_passed = True
        for scenario in spec["test_scenarios"]:
            print(f"  Testing scenario: {scenario}")
            trace = self.run_trace(binary, scenario)
            
            # Check required points
            trace_points = [t["point"] for t in trace]
            for required in spec["required_trace"]:
                if required["point"] == "*":
                    # Any secure exit required
                    if not any(t.get("exit") for t in trace):
                        print(f"    ✗ No secure exit reached")
                        all_passed = False
                elif required["point"] not in trace_points:
                    print(f"    ✗ Required point not hit: {required['point']}")
                    all_passed = False
                else:
                    print(f"    ✓ Hit required point: {required['point']}")
                    
            # Check forbidden points
            for forbidden in spec.get("forbidden_trace", []):
                if forbidden["point"] in trace_points:
                    print(f"    ✗ Hit forbidden point: {forbidden['point']}")
                    all_passed = False
                else:
                    print(f"    ✓ Avoided forbidden point: {forbidden['point']}")
                    
        # Clean up
        Path(binary).unlink(missing_ok=True)
        
        return all_passed
        
    def verify_all(self):
        """Verify all specifications."""
        specs = self.load_specifications()
        
        print("=== TRACE VERIFICATION ===")
        print(f"Verifying {len(specs['specifications'])} specifications")
        
        results = {}
        for spec in specs["specifications"]:
            results[spec["id"]] = self.verify_specification(spec)
            
        # Summary
        print("\n=== VERIFICATION SUMMARY ===")
        passed = sum(1 for v in results.values() if v)
        failed = len(results) - passed
        
        print(f"PASSED: {passed}")
        print(f"FAILED: {failed}")
        
        if failed > 0:
            print("\nFailed specifications:")
            for spec_id, result in results.items():
                if not result:
                    print(f"  - {spec_id}")
                    
        return failed == 0


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <spec_file.json> <source_file.c>")
        sys.exit(1)
        
    verifier = TraceVerifier(sys.argv[1], sys.argv[2])
    if verifier.verify_all():
        print("\nAll specifications verified!")
        sys.exit(0)
    else:
        print("\nSome specifications failed!")
        sys.exit(1)