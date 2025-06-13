#!/usr/bin/env python3
"""Build unique binaries to prove code path reachability."""

import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

class SecureProofBuilder:
    def __init__(self, source_file, proof_manifest):
        self.source_file = source_file
        self.proof_manifest = proof_manifest
        self.build_dir = Path("proof_builds")
        self.build_dir.mkdir(exist_ok=True)
        
    def generate_sha3_tag(self, name):
        """Generate a unique 256-bit SHA3 tag for a code point."""
        sha3 = hashlib.sha3_256()
        sha3.update(name.encode('utf-8'))
        return sha3.hexdigest()
        
    def extract_all_paths(self):
        """Extract all SECURE_CODE_POINT names from source file."""
        paths = set()
        with open(self.source_file, 'r') as f:
            content = f.read()
            # Find all SECURE_CODE_POINT(NAME, ...) occurrences
            import re
            matches = re.findall(r'SECURE_CODE_POINT\((\w+),', content)
            paths.update(matches)
        return sorted(list(paths))
        
    def build_proof_binary(self, test_name, enabled_paths, disabled_paths):
        """Build a binary with specific paths enabled/disabled."""
        # Get all paths from source file
        all_paths = self.extract_all_paths()
        
        defines = [f"-DPROVING_BUILD"]
        
        # Define all paths (default to 0 if not mentioned)
        for path in all_paths:
            if path in enabled_paths:
                defines.append(f"-DPATH_{path}_EXISTS=1")
            elif path in disabled_paths:
                defines.append(f"-DPATH_{path}_EXISTS=0")
            else:
                # Default to disabled if not mentioned
                defines.append(f"-DPATH_{path}_EXISTS=0")
            
        # Generate SHA3 tags
        tags = {}
        for path in enabled_paths:
            tags[path] = self.generate_sha3_tag(path)
            
        # Build command
        output = self.build_dir / f"proof_{test_name}"
        cmd = [
            "gcc",
            "-std=c99",
            "-I./include",
            *defines,
            self.source_file,
            "-o", str(output),
            "-lm"
        ]
        
        print(f"Building proof binary: {test_name}")
        print(f"  Enabled paths: {enabled_paths}")
        print(f"  Disabled paths: {disabled_paths}")
        print(f"  Command: {' '.join(cmd)}")
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"Build failed:\n{result.stderr}")
            return None
            
        return {
            "binary": str(output),
            "enabled_paths": enabled_paths,
            "disabled_paths": disabled_paths,
            "tags": tags
        }
        
    def run_proof_binary(self, binary_info, inputs):
        """Run a proof binary with specific inputs."""
        binary = binary_info["binary"]
        
        print(f"\nRunning proof: {binary}")
        print(f"  Inputs: {inputs}")
        
        # Run with deterministic inputs
        env = os.environ.copy()
        for key, value in inputs.items():
            env[key] = str(value)
            
        result = subprocess.run([binary], env=env, capture_output=True, text=True)
        
        if "SECURE_EXIT:" in result.stdout:
            # Extract exit point
            for line in result.stdout.split('\n'):
                if line.startswith("SECURE_EXIT:"):
                    exit_point = line.split(":")[1].strip()
                    print(f"  Reached secure exit: {exit_point}")
                    return exit_point
                    
        print(f"  No secure exit reached")
        return None
        
    def generate_proof_index(self, results):
        """Generate index of what each secure exit implies."""
        index = {
            "proofs": [],
            "unreachable_paths": [],
            "reachable_paths": {}
        }
        
        for test_name, result in results.items():
            proof = {
                "test": test_name,
                "binary": result["binary"]["binary"],
                "inputs": result["inputs"],
                "exit_point": result["exit_point"],
                "enabled_paths": result["binary"]["enabled_paths"],
                "disabled_paths": result["binary"]["disabled_paths"],
                "implies": result.get("implies", "")
            }
            
            index["proofs"].append(proof)
            
            # Track reachability
            if result["exit_point"]:
                point = result["exit_point"]
                if point not in index["reachable_paths"]:
                    index["reachable_paths"][point] = []
                index["reachable_paths"][point].append(test_name)
                
        # Find universally unreachable paths
        all_paths = set()
        for proof in index["proofs"]:
            all_paths.update(proof["enabled_paths"])
            all_paths.update(proof["disabled_paths"])
            
        reached_paths = set(index["reachable_paths"].keys())
        index["unreachable_paths"] = list(all_paths - reached_paths)
        
        return index
        
    def run_proof_suite(self):
        """Run complete proof suite from manifest."""
        with open(self.proof_manifest) as f:
            manifest = json.load(f)
            
        results = {}
        
        for test in manifest["tests"]:
            test_name = test["name"]
            
            # Build binary
            binary_info = self.build_proof_binary(
                test_name,
                test.get("enabled_paths", []),
                test.get("disabled_paths", [])
            )
            
            if not binary_info:
                continue
                
            # Run with various inputs
            exit_point = None
            for input_set in test["inputs"]:
                exit_point = self.run_proof_binary(binary_info, input_set)
                if exit_point:
                    break
                    
            results[test_name] = {
                "binary": binary_info,
                "inputs": test["inputs"],
                "exit_point": exit_point,
                "implies": test.get("implies", "")
            }
            
        # Generate index
        index = self.generate_proof_index(results)
        
        # Save index
        with open(self.build_dir / "proof_index.json", "w") as f:
            json.dump(index, f, indent=2)
            
        print("\n=== PROOF SUMMARY ===")
        print(f"Total tests: {len(results)}")
        print(f"Reachable paths: {len(index['reachable_paths'])}")
        print(f"Unreachable paths: {len(index['unreachable_paths'])}")
        
        if index['unreachable_paths']:
            print("\nPROVEN UNREACHABLE:")
            for path in index['unreachable_paths']:
                print(f"  - {path}")
                
        return index


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <source_file> <proof_manifest.json>")
        sys.exit(1)
        
    builder = SecureProofBuilder(sys.argv[1], sys.argv[2])
    builder.run_proof_suite()