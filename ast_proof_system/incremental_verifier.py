#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Incremental verification system for fast development cycles
Only re-verifies changed code, uses caching for unchanged files
"""

import os
import json
import hashlib
import time
import subprocess
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass, asdict
import pickle

@dataclass
class FileProof:
    """Proof data for a single file"""
    file_path: str
    file_hash: str
    timestamp: float
    dangerous_ops: List[Dict]
    verification_status: str  # 'safe', 'unsafe', 'error'
    proof_hash: str
    dependencies: List[str]  # Other files this depends on

@dataclass
class ProofCache:
    """Cache of all file proofs"""
    version: str = "2.0"  # Bumped for SHA3 upgrade
    proofs: Dict[str, FileProof] = None
    last_full_verification: float = 0
    hash_algorithm: str = "sha3_256"  # Track hash algorithm
    
    def __post_init__(self):
        if self.proofs is None:
            self.proofs = {}

class IncrementalVerifier:
    """Fast incremental verification for game development"""
    
    def __init__(self, project_root: str, cache_dir: str = ".ast_proof_cache"):
        self.project_root = Path(project_root)
        self.cache_dir = self.project_root / cache_dir
        self.cache_dir.mkdir(exist_ok=True)
        
        self.cache_file = self.cache_dir / "proof_cache.json"
        self.cache = self.load_cache()
        
        # File watching
        self.modified_files: Set[str] = set()
        self.last_check_time = time.time()
        
        # Performance metrics
        self.stats = {
            'files_checked': 0,
            'files_verified': 0,
            'files_cached': 0,
            'time_saved': 0.0
        }
    
    def load_cache(self) -> ProofCache:
        """Load proof cache from disk"""
        if self.cache_file.exists():
            try:
                with open(self.cache_file, 'r') as f:
                    data = json.load(f)
                    cache = ProofCache()
                    cache.version = data.get('version', '1.0')
                    cache.last_full_verification = data.get('last_full_verification', 0)
                    cache.hash_algorithm = data.get('hash_algorithm', 'sha256')
                    cache.proofs = {}
                    
                    # Check for hash algorithm change
                    if cache.version < "2.0" or cache.hash_algorithm != "sha3_256":
                        print(f"Cache uses old hash algorithm ({cache.hash_algorithm}), invalidating...")
                        return ProofCache()  # Start fresh with SHA3
                    
                    for path, proof_data in data.get('proofs', {}).items():
                        cache.proofs[path] = FileProof(**proof_data)
                    
                    return cache
            except Exception as e:
                print(f"Cache load failed: {e}, starting fresh")
        
        return ProofCache()
    
    def save_cache(self):
        """Save proof cache to disk"""
        cache_data = {
            'version': self.cache.version,
            'hash_algorithm': self.cache.hash_algorithm,
            'last_full_verification': self.cache.last_full_verification,
            'proofs': {
                path: asdict(proof) for path, proof in self.cache.proofs.items()
            }
        }
        
        with open(self.cache_file, 'w') as f:
            json.dump(cache_data, f, indent=2)
    
    def compute_file_hash(self, file_path: str) -> str:
        """Compute hash of file contents using SHA3-256"""
        hasher = hashlib.sha3_256()
        with open(file_path, 'rb') as f:
            # Read in chunks for large files
            while chunk := f.read(65536):  # 64KB chunks
                hasher.update(chunk)
        return hasher.hexdigest()
    
    def find_dependencies(self, file_path: str) -> List[str]:
        """Find files that this file depends on"""
        deps = []
        
        with open(file_path, 'r') as f:
            content = f.read()
            
        # Find #include statements
        import re
        includes = re.findall(r'#include\s*["<]([^">]+)[">]', content)
        
        for inc in includes:
            # Convert to absolute path if local include
            if not inc.startswith('/') and '"' in content:
                inc_path = Path(file_path).parent / inc
                if inc_path.exists():
                    deps.append(str(inc_path))
        
        return deps
    
    def needs_verification(self, file_path: str) -> Tuple[bool, str]:
        """Check if file needs verification"""
        # Check if file exists
        if not os.path.exists(file_path):
            return True, "file not found"
        
        # Check cache
        if file_path not in self.cache.proofs:
            return True, "not in cache"
        
        cached_proof = self.cache.proofs[file_path]
        
        # Check file hash
        current_hash = self.compute_file_hash(file_path)
        if current_hash != cached_proof.file_hash:
            return True, "file modified"
        
        # Check dependencies
        for dep in cached_proof.dependencies:
            if os.path.exists(dep):
                dep_hash = self.compute_file_hash(dep)
                # Simple check - in production would track dep hashes
                dep_mtime = os.path.getmtime(dep)
                if dep_mtime > cached_proof.timestamp:
                    return True, f"dependency {dep} modified"
        
        return False, "cached"
    
    def verify_file(self, file_path: str, force: bool = False) -> FileProof:
        """Verify a single file"""
        self.stats['files_checked'] += 1
        
        # Check if verification needed
        if not force:
            needs_verify, reason = self.needs_verification(file_path)
            if not needs_verify:
                self.stats['files_cached'] += 1
                print(f"  ✓ {file_path} - using cache")
                return self.cache.proofs[file_path]
        
        print(f"  ⟳ {file_path} - verifying...")
        start_time = time.time()
        
        # Run AST analyzer
        from enhanced_ast_analyzer import EnhancedASTAnalyzer
        analyzer = EnhancedASTAnalyzer(file_path)
        results = analyzer.analyze_complete()
        
        # Create proof
        proof = FileProof(
            file_path=file_path,
            file_hash=self.compute_file_hash(file_path),
            timestamp=time.time(),
            dangerous_ops=[asdict(op) for op in results['dangerous_operations']],
            verification_status='safe' if not results['dangerous_operations'] else 'unsafe',
            proof_hash=hashlib.sha3_256(str(results).encode()).hexdigest(),
            dependencies=self.find_dependencies(file_path)
        )
        
        # Update cache
        self.cache.proofs[file_path] = proof
        self.stats['files_verified'] += 1
        
        elapsed = time.time() - start_time
        print(f"    Verified in {elapsed:.2f}s - {proof.verification_status}")
        
        return proof
    
    def verify_directory(self, directory: str, pattern: str = "*.c") -> Dict:
        """Verify all matching files in directory"""
        results = {
            'safe_files': [],
            'unsafe_files': [],
            'error_files': [],
            'total_issues': 0
        }
        
        print(f"\n=== Incremental Verification: {directory} ===")
        
        # Find all C files
        import glob
        files = glob.glob(os.path.join(directory, "**", pattern), recursive=True)
        
        for file_path in files:
            try:
                proof = self.verify_file(file_path)
                
                if proof.verification_status == 'safe':
                    results['safe_files'].append(file_path)
                else:
                    results['unsafe_files'].append(file_path)
                    results['total_issues'] += len(proof.dangerous_ops)
                    
            except Exception as e:
                print(f"  ✗ {file_path} - error: {e}")
                results['error_files'].append(file_path)
        
        # Save cache after verification
        self.save_cache()
        
        return results
    
    def watch_mode(self, directory: str, interval: float = 1.0):
        """Watch directory for changes and verify incrementally"""
        print(f"Watching {directory} for changes... (Ctrl+C to stop)")
        
        try:
            while True:
                # Find modified files
                modified = self.find_modified_files(directory)
                
                if modified:
                    print(f"\nDetected {len(modified)} file changes")
                    for file_path in modified:
                        self.verify_file(file_path, force=True)
                    self.save_cache()
                    print("Verification complete\n")
                
                time.sleep(interval)
                
        except KeyboardInterrupt:
            print("\nStopping watch mode")
            self.print_statistics()
    
    def find_modified_files(self, directory: str) -> List[str]:
        """Find files modified since last check"""
        modified = []
        
        import glob
        files = glob.glob(os.path.join(directory, "**", "*.c"), recursive=True)
        
        for file_path in files:
            mtime = os.path.getmtime(file_path)
            if mtime > self.last_check_time:
                modified.append(file_path)
        
        self.last_check_time = time.time()
        return modified
    
    def generate_incremental_report(self, results: Dict) -> str:
        """Generate incremental verification report"""
        report = []
        report.append("# Incremental Verification Report")
        report.append(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report.append("")
        
        # Summary
        total_files = len(results['safe_files']) + len(results['unsafe_files'])
        report.append("## Summary")
        report.append(f"- Total files verified: {self.stats['files_verified']}")
        report.append(f"- Files from cache: {self.stats['files_cached']}")
        report.append(f"- Safe files: {len(results['safe_files'])}")
        report.append(f"- Unsafe files: {len(results['unsafe_files'])}")
        report.append(f"- Total issues: {results['total_issues']}")
        report.append("")
        
        # Performance
        report.append("## Performance")
        cache_ratio = (self.stats['files_cached'] / self.stats['files_checked'] * 100) if self.stats['files_checked'] > 0 else 0
        report.append(f"- Cache hit rate: {cache_ratio:.1f}%")
        report.append(f"- Time saved: ~{self.stats['files_cached'] * 0.5:.1f}s")
        report.append("")
        
        # Unsafe files
        if results['unsafe_files']:
            report.append("## Files Requiring Attention")
            for file_path in results['unsafe_files']:
                proof = self.cache.proofs[file_path]
                report.append(f"\n### {file_path}")
                report.append(f"Issues found: {len(proof.dangerous_ops)}")
                
                # Group by type
                by_type = {}
                for op in proof.dangerous_ops:
                    op_type = op['type']
                    by_type[op_type] = by_type.get(op_type, 0) + 1
                
                for op_type, count in by_type.items():
                    report.append(f"- {op_type}: {count}")
        
        return "\n".join(report)
    
    def print_statistics(self):
        """Print verification statistics"""
        print("\n=== Verification Statistics ===")
        print(f"Files checked: {self.stats['files_checked']}")
        print(f"Files verified: {self.stats['files_verified']}")
        print(f"Files from cache: {self.stats['files_cached']}")
        
        if self.stats['files_checked'] > 0:
            cache_ratio = self.stats['files_cached'] / self.stats['files_checked'] * 100
            print(f"Cache hit rate: {cache_ratio:.1f}%")
            print(f"Estimated time saved: {self.stats['files_cached'] * 0.5:.1f}s")
    
    def clear_cache(self):
        """Clear the proof cache"""
        self.cache = ProofCache()
        if self.cache_file.exists():
            self.cache_file.unlink()
        print("Cache cleared")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Incremental AST verification')
    parser.add_argument('directory', help='Directory to verify')
    parser.add_argument('--watch', action='store_true', help='Watch mode')
    parser.add_argument('--force', action='store_true', help='Force re-verification')
    parser.add_argument('--clear-cache', action='store_true', help='Clear cache')
    parser.add_argument('--report', help='Output report file')
    
    args = parser.parse_args()
    
    verifier = IncrementalVerifier(os.getcwd())
    
    if args.clear_cache:
        verifier.clear_cache()
        return
    
    if args.watch:
        verifier.watch_mode(args.directory)
    else:
        results = verifier.verify_directory(args.directory)
        
        # Generate report
        report = verifier.generate_incremental_report(results)
        
        if args.report:
            with open(args.report, 'w') as f:
                f.write(report)
            print(f"\nReport written to: {args.report}")
        else:
            print("\n" + report)
        
        verifier.print_statistics()

if __name__ == '__main__':
    main()