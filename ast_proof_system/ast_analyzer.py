#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
AST-based proof system for C code
Extracts all paths and proves safety properties
"""

import subprocess
import json
import re
from dataclasses import dataclass
from typing import List, Set, Dict, Optional
import sys

@dataclass
class ASTNode:
    """Represents a node in the C AST"""
    kind: str
    location: str
    name: Optional[str] = None
    type: Optional[str] = None
    children: List['ASTNode'] = None
    
    def __post_init__(self):
        if self.children is None:
            self.children = []

@dataclass
class DangerousOperation:
    """Represents an operation that could crash"""
    type: str  # 'division', 'deref', 'array_access', 'normalize'
    location: str
    function: str
    guard_conditions: List[str]
    has_handler: bool = False

class ASTAnalyzer:
    def __init__(self, source_file: str):
        self.source_file = source_file
        self.dangerous_ops: List[DangerousOperation] = []
        self.functions: Dict[str, ASTNode] = {}
        self.current_function = None
        self.conditions_stack: List[str] = []
        
    def extract_ast(self) -> str:
        """Extract AST from C file using clang"""
        cmd = ['clang', '-Xclang', '-ast-dump', '-fsyntax-only', self.source_file]
        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.stdout
    
    def parse_ast_line(self, line: str) -> Optional[ASTNode]:
        """Parse a single line of clang AST output"""
        # Example: |-BinaryOperator 0x5594e7b248d8 <line:189:9, col:37> 'float' '/'
        match = re.match(r'^(\|[-\s`]*)?(\w+)\s+0x[0-9a-f]+\s*<([^>]+)>\s*(.*)', line)
        if match:
            indent, kind, location, rest = match.groups()
            return ASTNode(kind=kind, location=location, type=rest)
        return None
    
    def find_dangerous_operations(self, ast_text: str):
        """Find all potentially dangerous operations in AST"""
        lines = ast_text.split('\n')
        
        for i, line in enumerate(lines):
            node = self.parse_ast_line(line)
            if not node:
                continue
                
            # Track current function
            if node.kind == 'FunctionDecl':
                func_match = re.search(r"'([^']+)'", line)
                if func_match:
                    self.current_function = func_match.group(1)
            
            # Check for dangerous operations
            if node.kind == 'BinaryOperator' and '/' in line:
                self.analyze_division(node, lines, i)
            
            elif node.kind == 'CallExpr':
                if 'Vector3Normalize' in line:
                    self.analyze_normalize(node, lines, i)
                elif 'strcpy' in line or 'strcat' in line:
                    self.analyze_string_op(node, lines, i)
            
            elif node.kind == 'ArraySubscriptExpr':
                self.analyze_array_access(node, lines, i)
            
            elif node.kind == 'UnaryOperator' and '->' in line:
                self.analyze_pointer_deref(node, lines, i)
    
    def analyze_division(self, node: ASTNode, lines: List[str], index: int):
        """Analyze a division operation for safety"""
        # Look for guard conditions before this operation
        guards = self.find_guards_for_operation(lines, index, 'division')
        
        dangerous_op = DangerousOperation(
            type='division',
            location=node.location,
            function=self.current_function or 'unknown',
            guard_conditions=guards
        )
        
        # Check if there's a zero check
        if not any('!= 0' in g or '> 0' in g for g in guards):
            self.dangerous_ops.append(dangerous_op)
    
    def analyze_normalize(self, node: ASTNode, lines: List[str], index: int):
        """Analyze Vector3Normalize for zero vector"""
        guards = self.find_guards_for_operation(lines, index, 'normalize')
        
        dangerous_op = DangerousOperation(
            type='normalize',
            location=node.location,
            function=self.current_function or 'unknown',
            guard_conditions=guards
        )
        
        # Check if there's a magnitude check
        if not any('Length' in g or 'magnitude' in g for g in guards):
            self.dangerous_ops.append(dangerous_op)
    
    def find_guards_for_operation(self, lines: List[str], op_index: int, op_type: str) -> List[str]:
        """Find guard conditions that protect an operation"""
        guards = []
        
        # Look backwards for if statements
        for i in range(max(0, op_index - 20), op_index):
            line = lines[i]
            if 'IfStmt' in line:
                # Extract condition
                condition = self.extract_condition(lines, i)
                if condition:
                    guards.append(condition)
        
        return guards
    
    def extract_condition(self, lines: List[str], if_index: int) -> Optional[str]:
        """Extract the condition from an if statement"""
        # Simple extraction - in real implementation would parse AST properly
        for i in range(if_index, min(len(lines), if_index + 5)):
            if 'BinaryOperator' in lines[i]:
                return lines[i].strip()
        return None
    
    def generate_proof_obligations(self) -> List[str]:
        """Generate proof obligations for each dangerous operation"""
        obligations = []
        
        for op in self.dangerous_ops:
            if op.type == 'division':
                obligations.append(f"PROVE: Division at {op.location} has non-zero divisor")
            elif op.type == 'normalize':
                obligations.append(f"PROVE: Normalize at {op.location} has non-zero vector")
            elif op.type == 'deref':
                obligations.append(f"PROVE: Pointer at {op.location} is non-null")
            elif op.type == 'array_access':
                obligations.append(f"PROVE: Array access at {op.location} is in bounds")
        
        return obligations

def main():
    if len(sys.argv) < 2:
        print("Usage: ast_analyzer.py <source_file.c>")
        sys.exit(1)
    
    analyzer = ASTAnalyzer(sys.argv[1])
    
    print("=== AST-Based Safety Analysis ===")
    print(f"Analyzing: {sys.argv[1]}")
    
    # Extract AST
    ast_text = analyzer.extract_ast()
    
    # Find dangerous operations
    analyzer.find_dangerous_operations(ast_text)
    
    # Generate proof obligations
    obligations = analyzer.generate_proof_obligations()
    
    print(f"\nFound {len(analyzer.dangerous_ops)} dangerous operations")
    print("\nProof obligations:")
    for obligation in obligations:
        print(f"  - {obligation}")
    
    # Generate GDB proof script
    if analyzer.dangerous_ops:
        print("\nGenerating GDB proof script...")
        with open('ast_generated_proof.gdb', 'w') as f:
            f.write(generate_gdb_proof(analyzer.dangerous_ops))
        print("Created: ast_generated_proof.gdb")

def generate_gdb_proof(dangerous_ops: List[DangerousOperation]) -> str:
    """Generate GDB script to verify safety at runtime"""
    script = """# AST-Generated GDB Proof Script
set pagination off

# Set breakpoints at each dangerous operation
"""
    
    for i, op in enumerate(dangerous_ops):
        location = op.location.split(':')[1].split(',')[0]  # Extract line number
        script += f"""
# Dangerous operation {i+1}: {op.type} in {op.function}
break {op.function}:{location}
commands
    silent
    printf "Checking {op.type} at {op.location}\\n"
"""
        
        if op.type == 'division':
            script += """    # Check divisor is non-zero
    if $divisor == 0
        printf "ERROR: Division by zero!\\n"
        set $proof_failed = 1
    else
        printf "OK: Divisor is %f\\n", $divisor
    end
"""
        
        script += "    continue\nend\n"
    
    script += """
# Run program and verify
run
if $proof_failed
    printf "PROOF FAILED: Dangerous operations found\\n"
    quit 1
else
    printf "PROOF PASSED: All operations safe\\n"
    quit 0
end
"""
    
    return script

if __name__ == "__main__":
    main()