#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
CI/CD integration for AST proof system
Supports GitHub Actions, GitLab CI, Jenkins, and local Git hooks
"""

import os
import sys
import json
import subprocess
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from datetime import datetime

class CIIntegration:
    """CI/CD integration for proof verification"""
    
    def __init__(self, project_root: str = "."):
        self.project_root = Path(project_root).resolve()
        self.ast_proof_dir = self.project_root / "ast_proof_system"
        
    def generate_github_action(self) -> str:
        """Generate GitHub Actions workflow"""
        workflow = """name: AST Safety Verification

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  verify-safety:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Set up Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'
    
    - name: Install Clang
      run: |
        sudo apt-get update
        sudo apt-get install -y clang-14
    
    - name: Install dependencies
      run: |
        python -m pip install --upgrade pip
        pip install z3-solver
    
    - name: Run AST verification
      run: |
        cd ast_proof_system
        python3 ci_integration.py --ci github --verify-all
    
    - name: Upload proof artifacts
      if: always()
      uses: actions/upload-artifact@v3
      with:
        name: proof-results
        path: |
          ast_proof_system/proof_results.json
          ast_proof_system/safety_report.html
          ast_proof_system/ast_safety_report.md
    
    - name: Comment PR with results
      if: github.event_name == 'pull_request'
      uses: actions/github-script@v6
      with:
        script: |
          const fs = require('fs');
          const results = JSON.parse(fs.readFileSync('ast_proof_system/proof_results.json', 'utf8'));
          
          let comment = '## 🔍 AST Safety Verification Results\\n\\n';
          
          if (results.overall_status === 'safe') {
            comment += '✅ **All safety checks passed!**\\n\\n';
          } else {
            comment += '❌ **Safety issues detected!**\\n\\n';
            comment += `Found ${results.total_issues} dangerous operations in ${results.unsafe_files.length} files.\\n\\n`;
            
            if (results.critical_issues > 0) {
              comment += `⚠️ **${results.critical_issues} CRITICAL issues require immediate attention!**\\n\\n`;
            }
          }
          
          comment += '<details><summary>View detailed report</summary>\\n\\n';
          comment += '```\\n' + results.summary + '\\n```\\n';
          comment += '</details>';
          
          github.rest.issues.createComment({
            issue_number: context.issue.number,
            owner: context.repo.owner,
            repo: context.repo.repo,
            body: comment
          });
    
    - name: Fail if unsafe
      run: |
        python3 ast_proof_system/ci_integration.py --check-results
"""
        return workflow
    
    def generate_gitlab_ci(self) -> str:
        """Generate GitLab CI configuration"""
        config = """stages:
  - verify

variables:
  PIP_CACHE_DIR: "$CI_PROJECT_DIR/.cache/pip"

cache:
  paths:
    - .cache/pip
    - ast_proof_system/.ast_proof_cache

ast-safety-verification:
  stage: verify
  image: python:3.9
  
  before_script:
    - apt-get update -qq && apt-get install -y clang
    - pip install z3-solver
  
  script:
    - cd ast_proof_system
    - python3 ci_integration.py --ci gitlab --verify-all
    - python3 ci_integration.py --generate-badge
  
  artifacts:
    when: always
    paths:
      - ast_proof_system/proof_results.json
      - ast_proof_system/safety_report.html
      - ast_proof_system/safety_badge.svg
    reports:
      junit: ast_proof_system/junit_report.xml
  
  coverage: '/Safety Coverage: \d+\.\d+%/'
"""
        return config
    
    def generate_pre_commit_hook(self) -> str:
        """Generate Git pre-commit hook"""
        hook = """#!/bin/bash
# AST Safety Pre-commit Hook

echo "Running AST safety verification..."

# Only check staged C files
STAGED_FILES=$(git diff --cached --name-only --diff-filter=ACM | grep '\.c$')

if [ -z "$STAGED_FILES" ]; then
    echo "No C files to verify"
    exit 0
fi

# Run incremental verification
python3 ast_proof_system/incremental_verifier.py --files $STAGED_FILES

if [ $? -ne 0 ]; then
    echo "❌ Safety verification failed!"
    echo "Fix the issues before committing."
    exit 1
fi

echo "✅ All safety checks passed!"
exit 0
"""
        return hook
    
    def run_verification(self, ci_platform: str = "local", verify_all: bool = False) -> Dict:
        """Run verification for CI"""
        print(f"=== AST Safety Verification ({ci_platform}) ===")
        start_time = datetime.now()
        
        results = {
            'timestamp': start_time.isoformat(),
            'ci_platform': ci_platform,
            'overall_status': 'safe',
            'total_files': 0,
            'safe_files': [],
            'unsafe_files': [],
            'total_issues': 0,
            'critical_issues': 0,
            'by_type': {},
            'summary': ""
        }
        
        try:
            # Import verifier
            sys.path.insert(0, str(self.ast_proof_dir))
            from incremental_verifier import IncrementalVerifier
            
            verifier = IncrementalVerifier(self.project_root)
            
            # Determine what to verify
            if verify_all:
                verify_results = verifier.verify_directory("src")
            else:
                # In CI, verify changed files
                changed_files = self.get_changed_files(ci_platform)
                verify_results = self.verify_files(verifier, changed_files)
            
            # Process results
            results.update(verify_results)
            
            # Count issues by type and severity
            for file_path in verify_results['unsafe_files']:
                if file_path in verifier.cache.proofs:
                    proof = verifier.cache.proofs[file_path]
                    for op in proof.dangerous_ops:
                        op_type = op['type']
                        severity = op.get('severity', 'high')
                        
                        results['by_type'][op_type] = results['by_type'].get(op_type, 0) + 1
                        if severity == 'critical':
                            results['critical_issues'] += 1
            
            # Determine overall status
            if results['unsafe_files']:
                results['overall_status'] = 'unsafe'
            
            # Generate summary
            results['summary'] = self.generate_summary(results)
            
        except Exception as e:
            results['overall_status'] = 'error'
            results['error'] = str(e)
            print(f"Verification error: {e}")
        
        # Save results
        with open(self.ast_proof_dir / 'proof_results.json', 'w') as f:
            json.dump(results, f, indent=2)
        
        # Generate reports
        self.generate_html_report(results)
        self.generate_junit_report(results)
        
        elapsed = (datetime.now() - start_time).total_seconds()
        print(f"Verification completed in {elapsed:.1f}s")
        
        return results
    
    def get_changed_files(self, ci_platform: str) -> List[str]:
        """Get list of changed files based on CI platform"""
        changed_files = []
        
        if ci_platform == "github":
            # GitHub Actions
            base = os.environ.get('GITHUB_BASE_REF', 'main')
            cmd = ['git', 'diff', '--name-only', f'origin/{base}...HEAD']
            
        elif ci_platform == "gitlab":
            # GitLab CI
            base = os.environ.get('CI_MERGE_REQUEST_TARGET_BRANCH_NAME', 'main')
            cmd = ['git', 'diff', '--name-only', f'origin/{base}...HEAD']
            
        else:
            # Local or generic
            cmd = ['git', 'diff', '--cached', '--name-only']
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            changed_files = [f for f in result.stdout.strip().split('\n') if f.endswith('.c')]
        except:
            print("Could not determine changed files, verifying all")
            
        return changed_files
    
    def verify_files(self, verifier, files: List[str]) -> Dict:
        """Verify specific files"""
        results = {
            'safe_files': [],
            'unsafe_files': [],
            'total_issues': 0
        }
        
        for file_path in files:
            if os.path.exists(file_path):
                proof = verifier.verify_file(file_path)
                
                if proof.verification_status == 'safe':
                    results['safe_files'].append(file_path)
                else:
                    results['unsafe_files'].append(file_path)
                    results['total_issues'] += len(proof.dangerous_ops)
        
        results['total_files'] = len(files)
        return results
    
    def generate_summary(self, results: Dict) -> str:
        """Generate human-readable summary"""
        summary = []
        
        if results['overall_status'] == 'safe':
            summary.append("✅ All files passed safety verification!")
        else:
            summary.append(f"❌ Found {results['total_issues']} safety issues")
            
            if results['critical_issues'] > 0:
                summary.append(f"⚠️  {results['critical_issues']} CRITICAL issues!")
            
            summary.append("\nIssues by type:")
            for issue_type, count in sorted(results['by_type'].items()):
                summary.append(f"  - {issue_type}: {count}")
            
            summary.append(f"\nAffected files: {len(results['unsafe_files'])}")
            for file_path in results['unsafe_files'][:5]:  # Show first 5
                summary.append(f"  - {file_path}")
            
            if len(results['unsafe_files']) > 5:
                summary.append(f"  ... and {len(results['unsafe_files']) - 5} more")
        
        return "\n".join(summary)
    
    def generate_html_report(self, results: Dict):
        """Generate interactive HTML report"""
        html = f"""<!DOCTYPE html>
<html>
<head>
    <title>AST Safety Report</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .safe {{ color: green; }}
        .unsafe {{ color: red; }}
        .critical {{ color: darkred; font-weight: bold; }}
        .file-list {{ margin-left: 20px; }}
        .issue {{ margin: 10px 0; padding: 10px; background: #f0f0f0; }}
        .summary {{ padding: 20px; background: #e0e0e0; margin: 20px 0; }}
    </style>
</head>
<body>
    <h1>AST Safety Verification Report</h1>
    <div class="summary">
        <h2>Summary</h2>
        <p>Status: <span class="{results['overall_status']}">{results['overall_status'].upper()}</span></p>
        <p>Total Files: {results['total_files']}</p>
        <p>Safe Files: {len(results['safe_files'])}</p>
        <p>Unsafe Files: {len(results['unsafe_files'])}</p>
        <p>Total Issues: {results['total_issues']}</p>
        <p>Critical Issues: {results['critical_issues']}</p>
    </div>
    
    <h2>Issues by Type</h2>
    <ul>
        {"".join(f'<li>{t}: {c}</li>' for t, c in results['by_type'].items())}
    </ul>
    
    <h2>Unsafe Files</h2>
    <div class="file-list">
        {"".join(f'<div class="issue">{f}</div>' for f in results['unsafe_files'])}
    </div>
    
    <p>Generated: {results['timestamp']}</p>
</body>
</html>"""
        
        with open(self.ast_proof_dir / 'safety_report.html', 'w') as f:
            f.write(html)
    
    def generate_junit_report(self, results: Dict):
        """Generate JUnit XML report for CI systems"""
        from xml.etree.ElementTree import Element, SubElement, tostring
        
        testsuites = Element('testsuites')
        testsuite = SubElement(testsuites, 'testsuite', {
            'name': 'AST Safety Verification',
            'tests': str(results['total_files']),
            'failures': str(len(results['unsafe_files'])),
            'time': '0'
        })
        
        # Add test cases
        for file_path in results['safe_files']:
            testcase = SubElement(testsuite, 'testcase', {
                'name': file_path,
                'classname': 'SafetyCheck'
            })
        
        for file_path in results['unsafe_files']:
            testcase = SubElement(testsuite, 'testcase', {
                'name': file_path,
                'classname': 'SafetyCheck'
            })
            failure = SubElement(testcase, 'failure', {
                'message': 'Safety verification failed'
            })
            failure.text = f"Found dangerous operations in {file_path}"
        
        # Write XML
        xml_str = tostring(testsuites, encoding='unicode')
        with open(self.ast_proof_dir / 'junit_report.xml', 'w') as f:
            f.write(xml_str)
    
    def generate_badge(self, results: Dict):
        """Generate SVG badge for README"""
        color = 'green' if results['overall_status'] == 'safe' else 'red'
        status = 'passing' if results['overall_status'] == 'safe' else 'failing'
        
        svg = f"""<svg xmlns="http://www.w3.org/2000/svg" width="120" height="20">
  <linearGradient id="b" x2="0" y2="100%">
    <stop offset="0" stop-color="#bbb" stop-opacity=".1"/>
    <stop offset="1" stop-opacity=".1"/>
  </linearGradient>
  <rect rx="3" width="120" height="20" fill="#555"/>
  <rect rx="3" x="60" width="60" height="20" fill="{color}"/>
  <rect rx="3" width="120" height="20" fill="url(#b)"/>
  <g fill="#fff" text-anchor="middle" font-family="Arial" font-size="11">
    <text x="30" y="15">safety</text>
    <text x="90" y="15">{status}</text>
  </g>
</svg>"""
        
        with open(self.ast_proof_dir / 'safety_badge.svg', 'w') as f:
            f.write(svg)
    
    def check_results(self) -> int:
        """Check results and return exit code for CI"""
        try:
            with open(self.ast_proof_dir / 'proof_results.json', 'r') as f:
                results = json.load(f)
            
            if results['overall_status'] == 'safe':
                print("✅ All safety checks passed!")
                return 0
            else:
                print(f"❌ Safety verification failed!")
                print(results['summary'])
                return 1
                
        except Exception as e:
            print(f"Error checking results: {e}")
            return 2
    
    def setup_hooks(self):
        """Setup Git hooks"""
        git_dir = self.project_root / '.git' / 'hooks'
        if git_dir.exists():
            hook_path = git_dir / 'pre-commit'
            
            with open(hook_path, 'w') as f:
                f.write(self.generate_pre_commit_hook())
            
            os.chmod(hook_path, 0o755)
            print(f"✅ Installed pre-commit hook at {hook_path}")
        else:
            print("❌ .git directory not found")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='CI/CD integration for AST proofs')
    parser.add_argument('--ci', choices=['local', 'github', 'gitlab'], 
                       default='local', help='CI platform')
    parser.add_argument('--verify-all', action='store_true', 
                       help='Verify all files (not just changed)')
    parser.add_argument('--generate-workflow', choices=['github', 'gitlab'],
                       help='Generate CI workflow file')
    parser.add_argument('--setup-hooks', action='store_true',
                       help='Setup Git hooks')
    parser.add_argument('--check-results', action='store_true',
                       help='Check results and exit with appropriate code')
    parser.add_argument('--generate-badge', action='store_true',
                       help='Generate status badge')
    
    args = parser.parse_args()
    
    ci = CIIntegration()
    
    if args.generate_workflow:
        if args.generate_workflow == 'github':
            workflow = ci.generate_github_action()
            path = '.github/workflows/ast-safety.yml'
        else:
            workflow = ci.generate_gitlab_ci()
            path = '.gitlab-ci.yml'
        
        print(f"Generated {args.generate_workflow} workflow:")
        print(workflow)
        print(f"\nSave this to: {path}")
        
    elif args.setup_hooks:
        ci.setup_hooks()
        
    elif args.check_results:
        sys.exit(ci.check_results())
        
    else:
        results = ci.run_verification(args.ci, args.verify_all)
        
        if args.generate_badge:
            ci.generate_badge(results)
            print("Generated safety badge: ast_proof_system/safety_badge.svg")
        
        # Exit with appropriate code
        if results['overall_status'] != 'safe':
            sys.exit(1)

if __name__ == '__main__':
    main()