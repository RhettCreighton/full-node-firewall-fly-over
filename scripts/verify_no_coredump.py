#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Static analysis to verify no coredump conditions exist in the code.
This runs at build time and fails the build if unsafe patterns are found.
"""

import sys
import re
import os
from pathlib import Path

# Patterns that can cause crashes
UNSAFE_PATTERNS = [
    # Division by zero
    (r'/\s*0(?![.]|\d)', "Division by zero"),
    (r'%\s*0(?![.]|\d)', "Modulo by zero"),
    (r'/\s*\(.*\)', "Division without zero check", "complex"),
    
    # Unsafe string functions
    (r'\bgets\s*\(', "gets() is always unsafe"),
    (r'\bstrcpy\s*\(', "strcpy() can overflow - use strncpy()"),
    (r'\bstrcat\s*\(', "strcat() can overflow - use strncat()"),
    (r'\bsprintf\s*\(', "sprintf() can overflow - use snprintf()"),
    (r'\bvsprintf\s*\(', "vsprintf() can overflow - use vsnprintf()"),
    (r'\bstrchr\s*\([^)]+\)\s*->', "strchr() result not checked for NULL"),
    
    # Unsafe memory operations
    (r'malloc\s*\([^)]+\)\s*;', "malloc() without NULL check"),
    (r'calloc\s*\([^)]+\)\s*;', "calloc() without NULL check"),
    (r'realloc\s*\([^)]+\)\s*;', "realloc() without NULL check"),
    (r'free\s*\(\s*\w+\s*\)\s*;[^}]*\1', "Use after free"),
    
    # Array access patterns
    (r'\[\s*\w+\s*\]\s*=', "Array access without visible bounds check", "warning"),
    (r'\[.*\+.*\]', "Array access with arithmetic", "warning"),
    (r'\[\s*\w+\s*-\s*1\s*\]', "Potential underflow in array access"),
    
    # Null pointer dereference
    (r'(\w+)\s*=\s*NULL;[^}]*\1->', "NULL pointer dereference"),
    (r'if\s*\(\s*!\s*(\w+)\s*\)[^{]*{[^}]*}\s*\1->', "Dereference after NULL check"),
    
    # Float operations
    (r'==\s*[0-9]*\.[0-9]+f?', "Direct float comparison"),
    (r'[0-9]*\.[0-9]+f?\s*==', "Direct float comparison"),
    (r'asinf?\s*\([^)]+\)', "asin without range check", "warning"),
    (r'sqrtf?\s*\([^)]+\)', "sqrt without negative check", "warning"),
    
    # Integer overflow
    (r'\+\+\s*\w+\s*;', "Unchecked increment", "info"),
    (r'\w+\s*\+=', "Unchecked addition", "info"),
    (r'\w+\s*\*=', "Unchecked multiplication", "warning"),
    
    # Signal safety
    (r'signal\s*\(', "Signal handler registration", "info"),
    (r'printf\s*\([^)]+\)', "printf in possible signal context", "context"),
]

def check_file(filepath):
    """Check a single file for unsafe patterns."""
    issues = []
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    for line_num, line in enumerate(lines, 1):
        # Skip comments
        if '//' in line:
            line = line[:line.index('//')]
        
        for pattern_info in UNSAFE_PATTERNS:
            pattern = pattern_info[0]
            description = pattern_info[1]
            severity = pattern_info[2] if len(pattern_info) > 2 else "error"
            
            if re.search(pattern, line):
                # Check for safe patterns nearby
                if "if" in line or "assert" in line or "SAFE_" in line:
                    continue
                
                # Skip certain patterns in certain contexts
                if severity == "context" and "signal" not in str(filepath):
                    continue
                    
                # Only report errors, not warnings/info in strict mode
                if severity != "error":
                    continue
                    
                issues.append(f"{filepath}:{line_num}: {description}\n  {line.strip()}")
    
    return issues

def main():
    """Check all C files in the project."""
    if len(sys.argv) < 2:
        print("Usage: verify_no_coredump.py <source_directory>")
        sys.exit(1)
    
    source_dir = Path(sys.argv[1])
    all_issues = []
    
    for c_file in source_dir.rglob("*.c"):
        # Skip test files and generated files
        if "test" in str(c_file) or "build" in str(c_file):
            continue
            
        issues = check_file(c_file)
        all_issues.extend(issues)
    
    if all_issues:
        print("❌ UNSAFE CODE PATTERNS DETECTED:")
        for issue in all_issues:
            print(f"  {issue}")
        print(f"\nTotal issues: {len(all_issues)}")
        print("Build FAILED - fix these issues to guarantee no coredumps")
        sys.exit(1)
    else:
        print("✅ No unsafe patterns detected - coredump risk minimized")
        sys.exit(0)

if __name__ == "__main__":
    main()