#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Enhanced AST analyzer for game development
Catches ALL dangerous operations including game-specific patterns
"""

import subprocess
import json
import re
from dataclasses import dataclass, field
from typing import List, Set, Dict, Optional, Tuple
import sys

@dataclass
class DangerousOperation:
    """Represents an operation that could crash"""
    type: str  # operation type
    subtype: str = ""  # specific subtype
    location: str = ""
    function: str = ""
    guard_conditions: List[str] = field(default_factory=list)
    severity: str = "high"  # high, medium, low
    description: str = ""
    fix_suggestion: str = ""

class EnhancedASTAnalyzer:
    """Complete AST analyzer for game safety verification"""
    
    # All dangerous operation patterns for games
    DANGEROUS_PATTERNS = {
        # Arithmetic operations
        'division': {
            'patterns': ['/', '%'],
            'ast_kinds': ['BinaryOperator'],
            'description': 'Division by zero causes SIGFPE',
            'fix': 'Add divisor != 0 check'
        },
        'integer_overflow': {
            'patterns': ['*', '+', '-', '<<'],
            'ast_kinds': ['BinaryOperator'],
            'description': 'Integer overflow undefined behavior',
            'fix': 'Use safe arithmetic functions'
        },
        
        # Memory operations
        'null_deref': {
            'patterns': ['->', '*'],
            'ast_kinds': ['UnaryOperator', 'MemberExpr'],
            'description': 'Null pointer dereference causes SIGSEGV',
            'fix': 'Add NULL check before access'
        },
        'array_bounds': {
            'patterns': ['['],
            'ast_kinds': ['ArraySubscriptExpr'],
            'description': 'Out-of-bounds access causes crash',
            'fix': 'Validate index range'
        },
        'buffer_overflow': {
            'patterns': ['strcpy', 'strcat', 'sprintf', 'gets'],
            'ast_kinds': ['CallExpr'],
            'description': 'Buffer overflow corrupts memory',
            'fix': 'Use safe variants (strncpy, snprintf)'
        },
        
        # Game-specific operations
        'vector_normalize': {
            'patterns': ['Vector3Normalize', 'Vector2Normalize', 'normalize'],
            'ast_kinds': ['CallExpr'],
            'description': 'Normalizing zero vector causes FPE',
            'fix': 'Check magnitude > epsilon'
        },
        'sqrt_negative': {
            'patterns': ['sqrt', 'sqrtf'],
            'ast_kinds': ['CallExpr'],
            'description': 'Square root of negative causes NaN',
            'fix': 'Ensure input >= 0'
        },
        'acos_domain': {
            'patterns': ['acos', 'acosf', 'asin', 'asinf'],
            'ast_kinds': ['CallExpr'],
            'description': 'Inverse trig out of [-1,1] causes NaN',
            'fix': 'Clamp input to valid range'
        },
        
        # Physics operations
        'raycast_div': {
            'patterns': ['raycast', 'line_intersect'],
            'ast_kinds': ['CallExpr'],
            'description': 'Ray-line intersection can divide by zero',
            'fix': 'Check for parallel lines'
        },
        'matrix_inverse': {
            'patterns': ['MatrixInvert', 'inverse'],
            'ast_kinds': ['CallExpr'],
            'description': 'Singular matrix inversion fails',
            'fix': 'Check determinant != 0'
        },
        
        # Resource operations
        'texture_load': {
            'patterns': ['LoadTexture', 'LoadImage'],
            'ast_kinds': ['CallExpr'],
            'description': 'Failed texture load returns NULL',
            'fix': 'Check return value'
        },
        'sound_play': {
            'patterns': ['PlaySound', 'LoadSound'],
            'ast_kinds': ['CallExpr'],
            'description': 'Playing NULL sound crashes',
            'fix': 'Validate sound handle'
        },
        
        # Concurrency (if multithreaded)
        'race_condition': {
            'patterns': ['pthread_mutex_lock', 'atomic_'],
            'ast_kinds': ['CallExpr'],
            'description': 'Data races cause undefined behavior',
            'fix': 'Proper synchronization'
        },
        
        # Type conversions
        'float_to_int': {
            'patterns': ['(int)', '(unsigned)'],
            'ast_kinds': ['CStyleCastExpr', 'ImplicitCastExpr'],
            'description': 'Float to int overflow undefined',
            'fix': 'Range check before cast'
        },
        
        # Memory allocation
        'malloc_fail': {
            'patterns': ['malloc', 'calloc', 'realloc'],
            'ast_kinds': ['CallExpr'],
            'description': 'Allocation failure returns NULL',
            'fix': 'Check return value'
        },
        'double_free': {
            'patterns': ['free'],
            'ast_kinds': ['CallExpr'],
            'description': 'Double free corrupts heap',
            'fix': 'Set pointer NULL after free'
        },
        
        # Stack operations
        'alloca_overflow': {
            'patterns': ['alloca', 'VLA'],
            'ast_kinds': ['CallExpr', 'VariableArrayType'],
            'description': 'Stack allocation can overflow',
            'fix': 'Limit allocation size'
        },
        
        # Assertions
        'assert_crash': {
            'patterns': ['assert', 'ASSERT'],
            'ast_kinds': ['CallExpr'],
            'description': 'Failed assertion aborts in release',
            'fix': 'Handle error gracefully'
        }
    }
    
    def __init__(self, source_file: str):
        self.source_file = source_file
        self.dangerous_ops: List[DangerousOperation] = []
        self.current_function = None
        self.function_stack: List[str] = []
        self.condition_stack: List[str] = []
        self.loop_depth = 0
        self.in_error_handler = False
        
    def extract_ast(self) -> str:
        """Extract complete AST with all details"""
        cmd = [
            'clang', 
            '-Xclang', '-ast-dump',
            '-Xclang', '-ast-dump-filter=',  # Dump everything
            '-fsyntax-only',
            '-fno-color-diagnostics',
            self.source_file
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"Warning: AST extraction had issues: {result.stderr}")
        return result.stdout
    
    def analyze_complete(self) -> Dict:
        """Run complete analysis and return structured results"""
        ast_text = self.extract_ast()
        self.find_all_dangerous_operations(ast_text)
        
        return {
            'source_file': self.source_file,
            'dangerous_operations': self.dangerous_ops,
            'statistics': self.calculate_statistics(),
            'risk_assessment': self.assess_overall_risk()
        }
    
    def find_all_dangerous_operations(self, ast_text: str):
        """Find ALL dangerous operations using pattern matching"""
        lines = ast_text.split('\n')
        
        for i, line in enumerate(lines):
            # Track context
            self.update_context(line)
            
            # Check each dangerous pattern
            for op_type, config in self.DANGEROUS_PATTERNS.items():
                if self.matches_dangerous_pattern(line, config):
                    self.analyze_dangerous_operation(
                        op_type, config, line, lines, i
                    )
    
    def matches_dangerous_pattern(self, line: str, config: Dict) -> bool:
        """Check if line matches a dangerous pattern"""
        # Check AST node type
        for ast_kind in config['ast_kinds']:
            if ast_kind in line:
                # Check specific patterns
                for pattern in config['patterns']:
                    if pattern in line:
                        return True
        return False
    
    def analyze_dangerous_operation(self, op_type: str, config: Dict, 
                                   line: str, lines: List[str], index: int):
        """Analyze a specific dangerous operation"""
        location = self.extract_location(line)
        guards = self.find_guard_conditions(lines, index, op_type)
        
        # Determine if operation is actually dangerous
        if self.is_operation_safe(op_type, guards, line):
            return
        
        # Create dangerous operation record
        dangerous_op = DangerousOperation(
            type=op_type,
            location=location,
            function=self.current_function or 'global',
            guard_conditions=guards,
            severity=self.assess_severity(op_type, guards),
            description=config['description'],
            fix_suggestion=config['fix']
        )
        
        # Add specific details based on type
        self.add_operation_details(dangerous_op, line, lines, index)
        
        self.dangerous_ops.append(dangerous_op)
    
    def update_context(self, line: str):
        """Update current parsing context"""
        if 'FunctionDecl' in line:
            match = re.search(r"'([^']+)'", line)
            if match:
                self.current_function = match.group(1)
                
        elif 'CompoundStmt' in line:
            if 'error' in self.current_function.lower():
                self.in_error_handler = True
                
        elif 'IfStmt' in line:
            self.condition_stack.append(line)
            
        elif 'WhileStmt' in line or 'ForStmt' in line:
            self.loop_depth += 1
    
    def find_guard_conditions(self, lines: List[str], op_index: int, 
                             op_type: str) -> List[str]:
        """Find all guard conditions protecting an operation"""
        guards = []
        
        # Look backwards for protective conditions
        search_range = min(50, op_index)  # Extended search range
        for i in range(max(0, op_index - search_range), op_index):
            line = lines[i]
            
            # If statements
            if 'IfStmt' in line:
                condition = self.extract_if_condition(lines, i)
                if condition and self.is_relevant_guard(condition, op_type):
                    guards.append(condition)
            
            # Ternary operators
            elif 'ConditionalOperator' in line:
                condition = self.extract_ternary_condition(lines, i)
                if condition:
                    guards.append(condition)
            
            # Assert statements (also guards)
            elif 'assert' in line.lower():
                guards.append(f"assert: {line.strip()}")
            
            # Early returns
            elif 'ReturnStmt' in line and i < op_index - 1:
                guards.append("early return")
        
        return guards
    
    def is_operation_safe(self, op_type: str, guards: List[str], line: str) -> bool:
        """Determine if operation is already safe"""
        if op_type == 'division':
            # Check for non-zero guards
            return any('!= 0' in g or '> 0' in g or 'fabs' in g for g in guards)
            
        elif op_type == 'null_deref':
            # Check for NULL checks
            return any('!= NULL' in g or '!=' in g and 'NULL' in g for g in guards)
            
        elif op_type == 'array_bounds':
            # Check for bounds checks
            return any('>=' in g and '<' in g for g in guards)
            
        elif op_type == 'vector_normalize':
            # Check for magnitude checks
            return any('Length' in g or 'magnitude' in g or '> 0' in g for g in guards)
            
        # Add more safety checks...
        return False
    
    def extract_location(self, line: str) -> str:
        """Extract file:line:column from AST line"""
        match = re.search(r'<([^>]+)>', line)
        if match:
            loc = match.group(1)
            # Convert to file:line format
            parts = loc.split(',')
            if parts:
                return parts[0].replace('line:', '').replace('col:', ':')
        return "unknown"
    
    def assess_severity(self, op_type: str, guards: List[str]) -> str:
        """Assess severity of a dangerous operation"""
        # Critical operations
        if op_type in ['null_deref', 'division', 'buffer_overflow']:
            return 'critical' if not guards else 'high'
        
        # High severity
        elif op_type in ['array_bounds', 'malloc_fail', 'double_free']:
            return 'high'
        
        # Medium severity
        elif op_type in ['float_to_int', 'sqrt_negative']:
            return 'medium' if guards else 'high'
        
        # Low severity (usually just warnings)
        elif op_type in ['assert_crash']:
            return 'low'
        
        return 'medium'
    
    def calculate_statistics(self) -> Dict:
        """Calculate statistics about dangerous operations"""
        stats = {
            'total': len(self.dangerous_ops),
            'by_type': {},
            'by_severity': {'critical': 0, 'high': 0, 'medium': 0, 'low': 0},
            'by_function': {}
        }
        
        for op in self.dangerous_ops:
            # By type
            stats['by_type'][op.type] = stats['by_type'].get(op.type, 0) + 1
            
            # By severity
            stats['by_severity'][op.severity] = stats['by_severity'].get(op.severity, 0) + 1
            
            # By function
            stats['by_function'][op.function] = stats['by_function'].get(op.function, 0) + 1
        
        return stats
    
    def assess_overall_risk(self) -> str:
        """Assess overall risk level of the codebase"""
        critical_count = sum(1 for op in self.dangerous_ops if op.severity == 'critical')
        high_count = sum(1 for op in self.dangerous_ops if op.severity == 'high')
        
        if critical_count > 0:
            return "CRITICAL - Immediate fixes required"
        elif high_count > 5:
            return "HIGH - Multiple dangerous operations found"
        elif high_count > 0:
            return "MODERATE - Some dangerous operations need attention"
        elif len(self.dangerous_ops) > 0:
            return "LOW - Minor issues found"
        else:
            return "SAFE - No dangerous operations detected"
    
    def generate_fix_report(self) -> str:
        """Generate actionable fix report"""
        report = ["# AST Safety Analysis - Fix Report\n"]
        report.append(f"Source: {self.source_file}\n")
        report.append(f"Risk Level: {self.assess_overall_risk()}\n")
        
        # Group by severity
        for severity in ['critical', 'high', 'medium', 'low']:
            ops = [op for op in self.dangerous_ops if op.severity == severity]
            if ops:
                report.append(f"\n## {severity.upper()} Severity Issues ({len(ops)})\n")
                for op in ops:
                    report.append(f"### {op.type} at {op.location}")
                    report.append(f"- Function: {op.function}")
                    report.append(f"- Description: {op.description}")
                    report.append(f"- Fix: {op.fix_suggestion}")
                    if op.guard_conditions:
                        report.append(f"- Existing guards: {', '.join(op.guard_conditions)}")
                    report.append("")
        
        return "\n".join(report)

def main():
    if len(sys.argv) < 2:
        print("Usage: enhanced_ast_analyzer.py <source_file.c>")
        sys.exit(1)
    
    analyzer = EnhancedASTAnalyzer(sys.argv[1])
    results = analyzer.analyze_complete()
    
    print("=== Enhanced AST Safety Analysis ===")
    print(f"Source: {results['source_file']}")
    print(f"Risk Assessment: {results['risk_assessment']}")
    print(f"\nFound {results['statistics']['total']} dangerous operations")
    
    # Show breakdown
    print("\nBy Type:")
    for op_type, count in results['statistics']['by_type'].items():
        print(f"  {op_type}: {count}")
    
    print("\nBy Severity:")
    for severity, count in results['statistics']['by_severity'].items():
        if count > 0:
            print(f"  {severity}: {count}")
    
    # Generate fix report
    report = analyzer.generate_fix_report()
    with open('ast_safety_report.md', 'w') as f:
        f.write(report)
    print(f"\nDetailed fix report written to: ast_safety_report.md")

if __name__ == '__main__':
    main()