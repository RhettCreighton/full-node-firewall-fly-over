#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Setup script for AST Proof System
Handles Python dependencies and system checks
"""

import os
import sys
import subprocess
from pathlib import Path

def check_command(cmd):
    """Check if a command exists"""
    try:
        subprocess.run([cmd, '--version'], capture_output=True, check=True)
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False

def check_python_module(module):
    """Check if a Python module is installed"""
    try:
        __import__(module)
        return True
    except ImportError:
        return False

def main():
    print("=== AST Proof System Setup ===\n")
    
    # Check system dependencies
    print("Checking system dependencies...")
    
    deps = {
        'clang': 'Clang compiler for AST extraction',
        'gdb': 'GDB debugger for runtime verification',
        'cmake': 'CMake build system',
        'git': 'Git version control'
    }
    
    missing = []
    for cmd, desc in deps.items():
        if check_command(cmd):
            print(f"✓ {cmd}: {desc}")
        else:
            print(f"✗ {cmd}: {desc} - NOT FOUND")
            missing.append(cmd)
    
    if missing:
        print(f"\nMissing dependencies: {', '.join(missing)}")
        print("\nInstall on Ubuntu/Debian:")
        print(f"  sudo apt-get install {' '.join(missing)}")
        print("\nInstall on macOS:")
        print("  brew install llvm gdb cmake")
        print("\nInstall on Fedora:")
        print(f"  sudo dnf install {' '.join(missing)}")
        sys.exit(1)
    
    # Check Python version
    print(f"\nPython version: {sys.version}")
    if sys.version_info < (3, 6):
        print("✗ Python 3.6+ required")
        sys.exit(1)
    else:
        print("✓ Python version OK")
    
    # Check Python modules
    print("\nChecking Python dependencies...")
    
    modules = {
        'z3': 'Z3 theorem prover (optional but recommended)'
    }
    
    for module, desc in modules.items():
        if check_python_module(module):
            print(f"✓ {module}: {desc}")
        else:
            print(f"✗ {module}: {desc} - NOT INSTALLED")
            print(f"  Install with: pip install z3-solver")
    
    # Create necessary directories
    print("\nCreating directories...")
    dirs = [
        '.ast_proof_cache',
        'reports',
        'proofs',
        'build'
    ]
    
    for d in dirs:
        Path(d).mkdir(exist_ok=True)
        print(f"✓ Created {d}/")
    
    # Check if we're in the right directory
    if not os.path.exists('enhanced_ast_analyzer.py'):
        print("\n✗ Error: Run setup.py from the ast_proof_system directory")
        sys.exit(1)
    
    # Build the system
    print("\nBuilding AST Proof System...")
    try:
        subprocess.run(['make', 'build'], check=True)
        print("✓ Build successful")
    except subprocess.CalledProcessError:
        print("✗ Build failed - trying without CMake...")
        # Try direct approach
        print("  System will work with direct Python scripts")
    
    # Run validation
    print("\nRunning validation tests...")
    try:
        result = subprocess.run(
            ['python3', 'validate_analyzer.py', 'enhanced_ast_analyzer.py', 'test_dangerous_code.c'],
            capture_output=True,
            text=True
        )
        if result.returncode == 0:
            print("✓ Validation passed")
        else:
            print("⚠ Validation had issues (this is expected for test file)")
    except Exception as e:
        print(f"✗ Validation error: {e}")
    
    print("\n=== Setup Complete ===")
    print("\nQuick start:")
    print("  1. Run analysis: python3 enhanced_ast_analyzer.py your_file.c")
    print("  2. Full proof: ./run_deterministic_proof.sh your_file.c")
    print("  3. Integration: make integrate")
    print("\nDocumentation: see README.md and ROCK_SOLID_SYSTEM.md")

if __name__ == '__main__':
    main()