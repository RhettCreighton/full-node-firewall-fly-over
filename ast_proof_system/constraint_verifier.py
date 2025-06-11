#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Constraint Verification System
Ensures all non-deterministic inputs are safely constrained
Makes proofs stronger by bounding the input space
"""

import re
from dataclasses import dataclass, field
from typing import List, Dict, Set, Optional, Tuple, Union
from enum import Enum
import json

class ConstraintType(Enum):
    """Types of constraints that can bound inputs"""
    RANGE = "range"              # min <= x <= max
    EQUALITY = "equality"        # x == value
    INEQUALITY = "inequality"    # x != value
    MODULO = "modulo"           # x % n == 0
    BITWISE = "bitwise"         # x & mask == value
    NULL_CHECK = "null_check"   # ptr != NULL
    ARRAY_BOUNDS = "array_bounds"  # 0 <= i < size
    CLAMPED = "clamped"         # clamp(x, min, max)
    VALIDATED = "validated"     # custom validation function
    UNCONSTRAINED = "unconstrained"  # DANGEROUS!

@dataclass
class Constraint:
    """Represents a constraint on a value"""
    type: ConstraintType
    expression: str
    min_value: Optional[float] = None
    max_value: Optional[float] = None
    valid_values: List[Union[int, float, str]] = field(default_factory=list)
    location: str = ""
    verified: bool = False

@dataclass
class InputConstraint:
    """Constraint information for a non-deterministic input"""
    variable_name: str
    input_source: str  # scanf, rand, time, etc.
    location: str
    function: str
    constraints: List[Constraint] = field(default_factory=list)
    propagated_to: Set[str] = field(default_factory=set)  # Variables affected
    safety_status: str = "unsafe"  # 'safe', 'partial', 'unsafe'

@dataclass
class SafetyViolation:
    """Represents an unconstrained dangerous operation"""
    operation_type: str
    location: str
    variable: str
    missing_constraint: str
    severity: str  # 'critical', 'high', 'medium'
    fix_suggestion: str

class ConstraintVerifier:
    """Verifies that all non-deterministic inputs are properly constrained"""
    
    # Patterns for constraint detection
    CONSTRAINT_PATTERNS = {
        # Range checks
        r'if\s*\(\s*(\w+)\s*>=\s*(\d+)\s*&&\s*\1\s*<=\s*(\d+)\s*\)': ConstraintType.RANGE,
        r'if\s*\(\s*(\w+)\s*>\s*(\d+)\s*&&\s*\1\s*<\s*(\d+)\s*\)': ConstraintType.RANGE,
        
        # Bounds checks
        r'if\s*\(\s*(\w+)\s*<\s*(\w+)\s*\)': ConstraintType.ARRAY_BOUNDS,
        r'if\s*\(\s*(\w+)\s*>=\s*0\s*&&\s*\1\s*<\s*(\w+)\s*\)': ConstraintType.ARRAY_BOUNDS,
        
        # NULL checks
        r'if\s*\(\s*(\w+)\s*!=\s*NULL\s*\)': ConstraintType.NULL_CHECK,
        r'if\s*\(\s*!\s*(\w+)\s*\)': ConstraintType.NULL_CHECK,
        
        # Clamping
        r'clamp\s*\(\s*(\w+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)': ConstraintType.CLAMPED,
        r'fmax\s*\(\s*fmin\s*\(\s*(\w+)\s*,\s*(\d+)\s*\)\s*,\s*(\d+)\s*\)': ConstraintType.CLAMPED,
        
        # Validation functions
        r'validate_\w+\s*\(\s*(\w+)\s*\)': ConstraintType.VALIDATED,
        r'is_valid_\w+\s*\(\s*(\w+)\s*\)': ConstraintType.VALIDATED,
    }
    
    # Required constraints for different operations
    REQUIRED_CONSTRAINTS = {
        'division': {
            'constraint': ConstraintType.INEQUALITY,
            'check': 'divisor != 0',
            'severity': 'critical'
        },
        'array_access': {
            'constraint': ConstraintType.ARRAY_BOUNDS,
            'check': '0 <= index < size',
            'severity': 'critical'
        },
        'pointer_deref': {
            'constraint': ConstraintType.NULL_CHECK,
            'check': 'ptr != NULL',
            'severity': 'critical'
        },
        'sqrt': {
            'constraint': ConstraintType.RANGE,
            'check': 'value >= 0',
            'severity': 'high'
        },
        'acos': {
            'constraint': ConstraintType.CLAMPED,
            'check': '-1 <= value <= 1',
            'severity': 'high'
        },
        'memory_alloc': {
            'constraint': ConstraintType.RANGE,
            'check': 'size > 0 && size < MAX_ALLOC',
            'severity': 'high'
        }
    }
    
    def __init__(self, source_file: str, determinism_report: Dict):
        self.source_file = source_file
        self.determinism_report = determinism_report
        self.input_constraints: List[InputConstraint] = []
        self.violations: List[SafetyViolation] = []
        self.constraint_coverage = 0.0
        
    def verify_constraints(self) -> Dict:
        """Main verification entry point"""
        print(f"=== Constraint Verification for {self.source_file} ===")
        
        # Load source code
        with open(self.source_file, 'r') as f:
            source_code = f.read()
        
        # Phase 1: Identify all non-deterministic inputs
        non_det_inputs = self.find_non_deterministic_inputs()
        
        # Phase 2: Find constraints for each input
        for input_info in non_det_inputs:
            self.find_input_constraints(input_info, source_code)
        
        # Phase 3: Trace constraint propagation
        self.trace_constraint_propagation(source_code)
        
        # Phase 4: Verify dangerous operations have constraints
        self.verify_operation_constraints()
        
        # Phase 5: Calculate safety metrics
        self.calculate_safety_metrics()
        
        return self.generate_verification_report()
    
    def find_non_deterministic_inputs(self) -> List[Dict]:
        """Extract non-deterministic inputs from determinism report"""
        inputs = []
        
        for source in self.determinism_report.get('input_sources', []):
            for func in source.get('affects', []):
                inputs.append({
                    'source': source['name'],
                    'type': source['type'],
                    'location': source['location'],
                    'function': func
                })
        
        return inputs
    
    def find_input_constraints(self, input_info: Dict, source_code: str):
        """Find constraints applied to a specific input"""
        input_constraint = InputConstraint(
            variable_name=f"{input_info['source']}_var",  # Simplified
            input_source=input_info['source'],
            location=input_info['location'],
            function=input_info['function']
        )
        
        # Extract function body
        func_body = self.extract_function_body(input_info['function'], source_code)
        if not func_body:
            return
        
        # Look for constraint patterns after the input
        lines = func_body.split('\n')
        input_line = self.find_input_line(input_info['source'], lines)
        
        if input_line >= 0:
            # Search for constraints in subsequent lines
            for i in range(input_line + 1, min(input_line + 20, len(lines))):
                line = lines[i]
                
                # Check each constraint pattern
                for pattern, constraint_type in self.CONSTRAINT_PATTERNS.items():
                    match = re.search(pattern, line)
                    if match:
                        constraint = self.parse_constraint(match, constraint_type, line)
                        if constraint:
                            input_constraint.constraints.append(constraint)
        
        # Determine safety status
        if not input_constraint.constraints:
            input_constraint.safety_status = "unsafe"
        elif self.has_sufficient_constraints(input_constraint):
            input_constraint.safety_status = "safe"
        else:
            input_constraint.safety_status = "partial"
        
        self.input_constraints.append(input_constraint)
    
    def parse_constraint(self, match: re.Match, constraint_type: ConstraintType, 
                        line: str) -> Optional[Constraint]:
        """Parse constraint details from regex match"""
        constraint = Constraint(
            type=constraint_type,
            expression=line.strip()
        )
        
        if constraint_type == ConstraintType.RANGE:
            if len(match.groups()) >= 3:
                try:
                    constraint.min_value = float(match.group(2))
                    constraint.max_value = float(match.group(3))
                except:
                    pass
        
        elif constraint_type == ConstraintType.CLAMPED:
            if len(match.groups()) >= 3:
                try:
                    constraint.min_value = float(match.group(2))
                    constraint.max_value = float(match.group(3))
                except:
                    pass
        
        return constraint
    
    def trace_constraint_propagation(self, source_code: str):
        """Trace how constraints propagate through the code"""
        # Simplified propagation analysis
        # In production, would use proper dataflow analysis
        
        for constraint in self.input_constraints:
            # Find assignments from constrained variables
            pattern = rf'{constraint.variable_name}\s*=\s*(\w+)'
            matches = re.finditer(pattern, source_code)
            
            for match in matches:
                assigned_var = match.group(1)
                constraint.propagated_to.add(assigned_var)
    
    def verify_operation_constraints(self):
        """Verify each dangerous operation has proper constraints"""
        # Check each operation from determinism report
        for op in self.determinism_report.get('operations', []):
            if op['determinism'] == 'non_deterministic':
                # Check if operation has required constraints
                required = self.REQUIRED_CONSTRAINTS.get(op['type'])
                
                if required:
                    has_constraint = self.check_operation_constraint(op, required)
                    
                    if not has_constraint:
                        violation = SafetyViolation(
                            operation_type=op['type'],
                            location=op['location'],
                            variable="unknown",  # Would extract from AST
                            missing_constraint=required['check'],
                            severity=required['severity'],
                            fix_suggestion=self.generate_fix_suggestion(op['type'], required)
                        )
                        self.violations.append(violation)
    
    def check_operation_constraint(self, operation: Dict, required: Dict) -> bool:
        """Check if operation has required constraint"""
        # Find constraints affecting this operation
        for constraint in self.input_constraints:
            if constraint.function == operation['function']:
                for c in constraint.constraints:
                    if c.type == required['constraint']:
                        return True
        return False
    
    def has_sufficient_constraints(self, input_constraint: InputConstraint) -> bool:
        """Determine if input has sufficient constraints for safety"""
        # Check based on input type
        if input_constraint.input_source in ['rand', 'random']:
            # Random values need range constraints
            return any(c.type in [ConstraintType.RANGE, ConstraintType.CLAMPED] 
                      for c in input_constraint.constraints)
        
        elif input_constraint.input_source in ['scanf', 'getchar']:
            # User input needs validation
            return any(c.type in [ConstraintType.VALIDATED, ConstraintType.RANGE] 
                      for c in input_constraint.constraints)
        
        elif 'ptr' in input_constraint.variable_name or 'pointer' in input_constraint.variable_name:
            # Pointers need NULL checks
            return any(c.type == ConstraintType.NULL_CHECK 
                      for c in input_constraint.constraints)
        
        # Default: need some constraint
        return len(input_constraint.constraints) > 0
    
    def generate_fix_suggestion(self, op_type: str, required: Dict) -> str:
        """Generate fix suggestion for missing constraint"""
        suggestions = {
            'division': "Add check: if (divisor != 0) { result = dividend / divisor; }",
            'array_access': "Add bounds check: if (index >= 0 && index < array_size) { value = array[index]; }",
            'pointer_deref': "Add NULL check: if (ptr != NULL) { value = ptr->field; }",
            'sqrt': "Add range check: if (value >= 0) { result = sqrt(value); }",
            'acos': "Clamp input: value = clamp(value, -1.0, 1.0); result = acos(value);",
            'memory_alloc': "Validate size: if (size > 0 && size < MAX_ALLOC) { ptr = malloc(size); }"
        }
        return suggestions.get(op_type, f"Add constraint: {required['check']}")
    
    def calculate_safety_metrics(self):
        """Calculate overall safety metrics"""
        total_inputs = len(self.input_constraints)
        safe_inputs = sum(1 for c in self.input_constraints if c.safety_status == "safe")
        partial_inputs = sum(1 for c in self.input_constraints if c.safety_status == "partial")
        
        if total_inputs > 0:
            self.constraint_coverage = (safe_inputs + 0.5 * partial_inputs) / total_inputs
        else:
            self.constraint_coverage = 1.0
    
    def extract_function_body(self, func_name: str, source_code: str) -> Optional[str]:
        """Extract function body from source code"""
        # Simplified extraction - in production would use proper parser
        pattern = rf'{func_name}\s*\([^)]*\)\s*\{{'
        match = re.search(pattern, source_code)
        
        if match:
            start = match.end()
            brace_count = 1
            i = start
            
            while i < len(source_code) and brace_count > 0:
                if source_code[i] == '{':
                    brace_count += 1
                elif source_code[i] == '}':
                    brace_count -= 1
                i += 1
            
            return source_code[start:i-1]
        
        return None
    
    def find_input_line(self, input_source: str, lines: List[str]) -> int:
        """Find line number where input occurs"""
        for i, line in enumerate(lines):
            if input_source in line:
                return i
        return -1
    
    def generate_verification_report(self) -> Dict:
        """Generate comprehensive constraint verification report"""
        report = {
            'source_file': self.source_file,
            'constraint_coverage': self.constraint_coverage,
            'safety_score': self.calculate_safety_score(),
            'input_constraints': [
                {
                    'variable': ic.variable_name,
                    'source': ic.input_source,
                    'location': ic.location,
                    'function': ic.function,
                    'constraints': [
                        {
                            'type': c.type.value,
                            'expression': c.expression,
                            'min': c.min_value,
                            'max': c.max_value
                        }
                        for c in ic.constraints
                    ],
                    'safety_status': ic.safety_status,
                    'propagated_to': list(ic.propagated_to)
                }
                for ic in self.input_constraints
            ],
            'violations': [
                {
                    'type': v.operation_type,
                    'location': v.location,
                    'missing': v.missing_constraint,
                    'severity': v.severity,
                    'fix': v.fix_suggestion
                }
                for v in self.violations
            ],
            'summary': self.generate_summary()
        }
        
        return report
    
    def calculate_safety_score(self) -> float:
        """Calculate overall safety score (0-100)"""
        base_score = self.constraint_coverage * 100
        
        # Deduct points for violations
        critical_violations = sum(1 for v in self.violations if v.severity == 'critical')
        high_violations = sum(1 for v in self.violations if v.severity == 'high')
        
        penalty = critical_violations * 20 + high_violations * 10
        
        return max(0, base_score - penalty)
    
    def generate_summary(self) -> str:
        """Generate human-readable summary"""
        total_inputs = len(self.input_constraints)
        safe = sum(1 for c in self.input_constraints if c.safety_status == "safe")
        partial = sum(1 for c in self.input_constraints if c.safety_status == "partial") 
        unsafe = sum(1 for c in self.input_constraints if c.safety_status == "unsafe")
        
        summary = []
        summary.append(f"Total non-deterministic inputs: {total_inputs}")
        summary.append(f"  Fully constrained: {safe}")
        summary.append(f"  Partially constrained: {partial}")
        summary.append(f"  Unconstrained: {unsafe}")
        summary.append(f"\nConstraint coverage: {self.constraint_coverage*100:.1f}%")
        summary.append(f"Safety score: {self.calculate_safety_score():.1f}/100")
        
        if self.violations:
            summary.append(f"\nViolations found: {len(self.violations)}")
            critical = sum(1 for v in self.violations if v.severity == 'critical')
            if critical > 0:
                summary.append(f"  CRITICAL: {critical}")
        else:
            summary.append("\n✓ All non-deterministic operations properly constrained!")
        
        return '\n'.join(summary)

def generate_constraint_proof(report: Dict) -> str:
    """Generate GDB proof that verifies constraints at runtime"""
    proof = [
        "# Runtime Constraint Verification Proof",
        "# Verifies all non-deterministic inputs are properly bounded",
        "",
        "define verify_constraints",
        "  set $violations = 0",
        ""
    ]
    
    # Add verification for each constrained input
    for ic in report['input_constraints']:
        proof.append(f"  # Verify constraints for {ic['source']} at {ic['location']}")
        
        for constraint in ic['constraints']:
            if constraint['type'] == 'range':
                proof.append(f"  # Range check: {constraint['min']} <= value <= {constraint['max']}")
                proof.append(f"  if $value < {constraint['min']} || $value > {constraint['max']}")
                proof.append("    set $violations = $violations + 1")
                proof.append(f'    printf "VIOLATION: Value %f outside range [{constraint["min"]}, {constraint["max"]}]\\n", $value')
                proof.append("  end")
            
            elif constraint['type'] == 'null_check':
                proof.append("  # NULL check")
                proof.append("  if $ptr == 0")
                proof.append("    set $violations = $violations + 1")
                proof.append('    printf "VIOLATION: NULL pointer not checked\\n"')
                proof.append("  end")
        
        proof.append("")
    
    proof.extend([
        "  if $violations == 0",
        '    printf "✓ All constraints verified at runtime\\n"',
        "  else",
        '    printf "✗ %d constraint violations detected\\n", $violations',
        "  end",
        "end",
        "",
        "verify_constraints"
    ])
    
    return '\n'.join(proof)

def main():
    import sys
    
    if len(sys.argv) < 3:
        print("Usage: constraint_verifier.py <source_file.c> <determinism_report.json>")
        sys.exit(1)
    
    source_file = sys.argv[1]
    
    # Load determinism report
    with open(sys.argv[2], 'r') as f:
        det_report = json.load(f)
    
    verifier = ConstraintVerifier(source_file, det_report)
    report = verifier.verify_constraints()
    
    # Print summary
    print("\n" + report['summary'])
    
    # Save detailed report
    with open('constraint_report.json', 'w') as f:
        json.dump(report, f, indent=2)
    print(f"\nDetailed report saved to: constraint_report.json")
    
    # Generate runtime proof
    proof = generate_constraint_proof(report)
    with open('constraint_proof.gdb', 'w') as f:
        f.write(proof)
    print(f"Runtime constraint proof saved to: constraint_proof.gdb")
    
    # Exit with error if critical violations
    critical = sum(1 for v in report['violations'] if v['severity'] == 'critical')
    if critical > 0:
        print(f"\n❌ {critical} CRITICAL violations found - fix before proceeding!")
        sys.exit(1)

if __name__ == '__main__':
    main()