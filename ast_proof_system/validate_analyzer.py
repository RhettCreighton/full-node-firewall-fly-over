#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Validation suite for AST analyzer
Ensures we catch all known dangerous operations
"""

import subprocess
import sys
import json
from dataclasses import dataclass
from typing import List, Dict, Set

@dataclass
class ExpectedBug:
    """Expected dangerous operation in test code"""
    function: str
    type: str
    line_contains: str
    should_be_found: bool = True

class AnalyzerValidator:
    """Validates AST analyzer against known bugs"""
    
    # Expected bugs in test_dangerous_code.c
    EXPECTED_BUGS = [
        # Definite bugs that should be caught
        ExpectedBug("calculate_damage", "division", "armor"),
        ExpectedBug("update_object", "null_deref", "obj->position"),
        ExpectedBug("process_items", "array_bounds", "items[i]"),
        ExpectedBug("set_name", "buffer_overflow", "strcpy"),
        ExpectedBug("normalize_vector", "division", "length"),
        ExpectedBug("calculate_distance", "sqrt_negative", "sqrtf"),
        ExpectedBug("calculate_angle", "acos_domain", "acosf"),
        ExpectedBug("calculate_score", "integer_overflow", "kills * multiplier"),
        ExpectedBug("create_object", "null_deref", "obj->id"),
        ExpectedBug("cleanup_objects", "double_free", "free(objects[i])"),
        ExpectedBug("process_and_free", "null_deref", "obj->health"),
        ExpectedBug("process_dynamic_array", "alloca_overflow", "alloca"),
        ExpectedBug("set_position", "float_to_int", "(int)x"),
        ExpectedBug("get_wrapped_index", "division", "index % wrap"),
        ExpectedBug("process_buffer", "null_deref", "*ptr"),
        ExpectedBug("invert_matrix", "division", "/ det"),
        ExpectedBug("play_sound", "null_deref", "sound->id"),
        ExpectedBug("render_texture", "array_bounds", "tex->pixels[pixel_index]"),
        ExpectedBug("complex_calculation", "division", "c / a"),
        
        # Safe operations that should NOT be flagged
        ExpectedBug("safe_divide", "division", "a / b", should_be_found=False),
        ExpectedBug("safe_deref", "null_deref", "obj->health", should_be_found=False),
        ExpectedBug("safe_normalize", "division", "length", should_be_found=False),
    ]
    
    def __init__(self):
        self.results = {
            'passed': 0,
            'failed': 0,
            'false_positives': 0,
            'false_negatives': 0,
            'details': []
        }
    
    def run_validation(self, analyzer_path: str, test_file: str):
        """Run complete validation suite"""
        print("=== AST Analyzer Validation Suite ===\n")
        
        # Run enhanced analyzer
        print(f"Running analyzer on {test_file}...")
        found_bugs = self.run_analyzer(analyzer_path, test_file)
        
        # Validate results
        print("\nValidating detection results...")
        self.validate_results(found_bugs)
        
        # Generate report
        self.generate_report()
        
        return self.results['failed'] == 0
    
    def run_analyzer(self, analyzer_path: str, test_file: str) -> List[Dict]:
        """Run the analyzer and parse results"""
        cmd = ['python3', analyzer_path, test_file]
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"Analyzer failed: {result.stderr}")
            return []
        
        # Parse output to extract found bugs
        # This is simplified - real implementation would parse structured output
        found_bugs = []
        lines = result.stdout.split('\n')
        
        current_function = None
        for line in lines:
            if 'Function:' in line:
                current_function = line.split('Function:')[1].strip()
            elif any(bug_type in line for bug_type in ['division', 'null_deref', 'array_bounds']):
                found_bugs.append({
                    'function': current_function,
                    'type': self.extract_bug_type(line),
                    'line': line
                })
        
        return found_bugs
    
    def extract_bug_type(self, line: str) -> str:
        """Extract bug type from analyzer output"""
        bug_types = [
            'division', 'null_deref', 'array_bounds', 'buffer_overflow',
            'integer_overflow', 'sqrt_negative', 'acos_domain', 'float_to_int',
            'double_free', 'alloca_overflow'
        ]
        
        for bug_type in bug_types:
            if bug_type in line:
                return bug_type
        return 'unknown'
    
    def validate_results(self, found_bugs: List[Dict]):
        """Validate found bugs against expected"""
        found_set = {(bug['function'], bug['type']) for bug in found_bugs}
        
        for expected in self.EXPECTED_BUGS:
            key = (expected.function, expected.type)
            found = key in found_set
            
            if expected.should_be_found and found:
                self.results['passed'] += 1
                self.results['details'].append(f"✓ Found expected bug: {expected.function} - {expected.type}")
            elif expected.should_be_found and not found:
                self.results['failed'] += 1
                self.results['false_negatives'] += 1
                self.results['details'].append(f"✗ MISSED bug: {expected.function} - {expected.type}")
            elif not expected.should_be_found and not found:
                self.results['passed'] += 1
                self.results['details'].append(f"✓ Correctly ignored safe code: {expected.function}")
            else:  # not should_be_found but found
                self.results['failed'] += 1
                self.results['false_positives'] += 1
                self.results['details'].append(f"✗ FALSE POSITIVE: {expected.function} - {expected.type}")
    
    def generate_report(self):
        """Generate validation report"""
        print("\n=== Validation Results ===")
        print(f"Total tests: {len(self.EXPECTED_BUGS)}")
        print(f"Passed: {self.results['passed']}")
        print(f"Failed: {self.results['failed']}")
        print(f"False positives: {self.results['false_positives']}")
        print(f"False negatives: {self.results['false_negatives']}")
        
        if self.results['failed'] > 0:
            print("\nFailed tests:")
            for detail in self.results['details']:
                if detail.startswith('✗'):
                    print(f"  {detail}")
        
        # Calculate accuracy
        total = len(self.EXPECTED_BUGS)
        accuracy = (self.results['passed'] / total * 100) if total > 0 else 0
        print(f"\nAccuracy: {accuracy:.1f}%")
        
        # Write detailed report
        with open('validation_report.json', 'w') as f:
            json.dump(self.results, f, indent=2)
        
        print("\nDetailed report written to validation_report.json")

def main():
    validator = AnalyzerValidator()
    
    # Use enhanced analyzer by default
    analyzer_path = 'enhanced_ast_analyzer.py'
    test_file = 'test_dangerous_code.c'
    
    if len(sys.argv) > 1:
        analyzer_path = sys.argv[1]
    if len(sys.argv) > 2:
        test_file = sys.argv[2]
    
    success = validator.run_validation(analyzer_path, test_file)
    
    # Return exit code based on validation
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()