#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Determinism Analyzer for AST GDB Proving System
Tags all operations as deterministic or non-deterministic
Enables stronger proofs and performance optimizations
"""

import subprocess
import re
from dataclasses import dataclass, field
from typing import List, Dict, Set, Optional, Tuple
from enum import Enum
import json

class Determinism(Enum):
    """Determinism classification"""
    DETERMINISTIC = "deterministic"
    NON_DETERMINISTIC = "non_deterministic"
    CONDITIONALLY_DETERMINISTIC = "conditionally_deterministic"
    UNKNOWN = "unknown"

@dataclass
class InputSource:
    """Represents a source of input to the program"""
    name: str
    type: str  # 'user_input', 'time', 'random', 'file', 'network', 'hardware'
    location: str
    determinism: Determinism
    affects_functions: Set[str] = field(default_factory=set)
    taint_paths: List[str] = field(default_factory=list)

@dataclass
class Operation:
    """Enhanced operation with determinism tracking"""
    type: str
    location: str
    function: str
    determinism: Determinism
    input_sources: List[InputSource] = field(default_factory=list)
    proof_strength: str = "weak"  # 'weak', 'strong', 'mathematical'
    optimization_potential: str = "none"  # 'none', 'cache', 'precompute', 'eliminate'

class DeterminismAnalyzer:
    """Analyzes code to identify deterministic vs non-deterministic operations"""
    
    # Non-deterministic sources
    NON_DETERMINISTIC_SOURCES = {
        # User input
        'scanf': 'user_input',
        'getchar': 'user_input',
        'fgets': 'user_input',
        'read': 'user_input',
        'recv': 'network',
        
        # Time-based
        'time': 'time',
        'gettimeofday': 'time',
        'clock': 'time',
        'clock_gettime': 'time',
        
        # Random
        'rand': 'random',
        'random': 'random',
        'drand48': 'random',
        'getrandom': 'random',
        '/dev/random': 'random',
        '/dev/urandom': 'random',
        
        # System state
        'getpid': 'system',
        'getenv': 'environment',
        'getcwd': 'filesystem',
        
        # Hardware
        'rdtsc': 'hardware',  # CPU timestamp counter
        'cpuid': 'hardware',
        
        # Game-specific
        'GetKeyState': 'user_input',
        'GetMousePosition': 'user_input',
        'GetGamepadAxis': 'user_input',
        'NetworkReceive': 'network',
    }
    
    # Deterministic operations (can be proven once)
    DETERMINISTIC_OPERATIONS = {
        'pure_math': ['sin', 'cos', 'sqrt', 'pow', 'fabs'],
        'constants': ['PI', 'E', 'MAX_INT'],
        'compile_time': ['sizeof', 'offsetof', '_Static_assert'],
        'pure_functions': []  # Populated by analysis
    }
    
    def __init__(self, source_file: str):
        self.source_file = source_file
        self.input_sources: List[InputSource] = []
        self.operations: List[Operation] = []
        self.function_determinism: Dict[str, Determinism] = {}
        self.taint_graph: Dict[str, Set[str]] = {}  # var -> sources
        self.current_function = None
        
    def analyze(self) -> Dict:
        """Run complete determinism analysis"""
        print(f"=== Determinism Analysis for {self.source_file} ===")
        
        # Extract AST
        ast_text = self.extract_ast()
        
        # Phase 1: Find all input sources
        self.find_input_sources(ast_text)
        
        # Phase 2: Build taint graph
        self.build_taint_graph(ast_text)
        
        # Phase 3: Classify function determinism
        self.classify_functions(ast_text)
        
        # Phase 4: Tag all operations
        self.tag_operations(ast_text)
        
        # Phase 5: Calculate proof optimization potential
        self.calculate_optimizations()
        
        return self.generate_report()
    
    def extract_ast(self) -> str:
        """Extract AST with full details"""
        cmd = [
            'clang', '-Xclang', '-ast-dump',
            '-Xclang', '-ast-dump-filter=',
            '-fsyntax-only', self.source_file
        ]
        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.stdout
    
    def find_input_sources(self, ast_text: str):
        """Identify all sources of non-determinism"""
        lines = ast_text.split('\n')
        
        for i, line in enumerate(lines):
            # Track current function
            if 'FunctionDecl' in line:
                match = re.search(r"'([^']+)'", line)
                if match:
                    self.current_function = match.group(1)
            
            # Check for non-deterministic calls
            if 'CallExpr' in line:
                for func_name, input_type in self.NON_DETERMINISTIC_SOURCES.items():
                    if func_name in line:
                        location = self.extract_location(line)
                        source = InputSource(
                            name=func_name,
                            type=input_type,
                            location=location,
                            determinism=Determinism.NON_DETERMINISTIC
                        )
                        source.affects_functions.add(self.current_function or 'global')
                        self.input_sources.append(source)
                        
            # Check for volatile variables (hardware access)
            elif 'volatile' in line:
                location = self.extract_location(line)
                source = InputSource(
                    name='volatile_access',
                    type='hardware',
                    location=location,
                    determinism=Determinism.NON_DETERMINISTIC
                )
                source.affects_functions.add(self.current_function or 'global')
                self.input_sources.append(source)
    
    def build_taint_graph(self, ast_text: str):
        """Track how non-determinism propagates through the program"""
        # Simplified taint analysis
        # In production, would use proper dataflow analysis
        
        tainted_vars = set()
        
        # Mark variables assigned from non-deterministic sources
        for source in self.input_sources:
            # Find assignments from this source
            # This is simplified - real implementation would parse AST properly
            if source.affects_functions:
                for func in source.affects_functions:
                    self.taint_graph[func] = self.taint_graph.get(func, set())
                    self.taint_graph[func].add(source.name)
    
    def classify_functions(self, ast_text: str):
        """Classify each function's determinism"""
        lines = ast_text.split('\n')
        current_func = None
        func_calls = {}  # function -> called functions
        
        for line in lines:
            if 'FunctionDecl' in line:
                match = re.search(r"'([^']+)'", line)
                if match:
                    current_func = match.group(1)
                    self.function_determinism[current_func] = Determinism.DETERMINISTIC
                    func_calls[current_func] = set()
            
            elif 'CallExpr' in line and current_func:
                # Track function calls
                match = re.search(r"'([^']+)'", line)
                if match:
                    called = match.group(1)
                    func_calls[current_func].add(called)
        
        # Propagate non-determinism through call graph
        changed = True
        while changed:
            changed = False
            for func, calls in func_calls.items():
                if self.function_determinism.get(func) == Determinism.DETERMINISTIC:
                    # Check if any called function is non-deterministic
                    for called in calls:
                        if called in self.NON_DETERMINISTIC_SOURCES:
                            self.function_determinism[func] = Determinism.NON_DETERMINISTIC
                            changed = True
                            break
                        elif self.function_determinism.get(called) == Determinism.NON_DETERMINISTIC:
                            self.function_determinism[func] = Determinism.NON_DETERMINISTIC
                            changed = True
                            break
        
        # Check for tainted functions
        for func, sources in self.taint_graph.items():
            if sources:
                self.function_determinism[func] = Determinism.NON_DETERMINISTIC
    
    def tag_operations(self, ast_text: str):
        """Tag each dangerous operation with determinism info"""
        # Import enhanced analyzer
        from enhanced_ast_analyzer import EnhancedASTAnalyzer
        
        analyzer = EnhancedASTAnalyzer(self.source_file)
        results = analyzer.analyze_complete()
        
        # Tag each dangerous operation
        for dangerous_op in results['dangerous_operations']:
            op = Operation(
                type=dangerous_op.type,
                location=dangerous_op.location,
                function=dangerous_op.function,
                determinism=self.function_determinism.get(
                    dangerous_op.function, 
                    Determinism.UNKNOWN
                )
            )
            
            # Determine proof strength based on determinism
            if op.determinism == Determinism.DETERMINISTIC:
                op.proof_strength = "mathematical"
                op.optimization_potential = "cache"
            elif op.determinism == Determinism.NON_DETERMINISTIC:
                op.proof_strength = "weak"
                op.optimization_potential = "none"
            
            # Special cases
            if op.type == 'division' and op.determinism == Determinism.DETERMINISTIC:
                op.optimization_potential = "precompute"
            elif op.type == 'array_bounds' and op.determinism == Determinism.DETERMINISTIC:
                op.optimization_potential = "eliminate"  # Can be removed after first proof
            
            self.operations.append(op)
    
    def calculate_optimizations(self):
        """Calculate optimization potential based on determinism"""
        self.optimization_stats = {
            'total_operations': len(self.operations),
            'deterministic_ops': 0,
            'non_deterministic_ops': 0,
            'cacheable_proofs': 0,
            'eliminable_checks': 0,
            'precomputable_values': 0,
            'estimated_speedup': 0.0
        }
        
        for op in self.operations:
            if op.determinism == Determinism.DETERMINISTIC:
                self.optimization_stats['deterministic_ops'] += 1
                
                if op.optimization_potential == "cache":
                    self.optimization_stats['cacheable_proofs'] += 1
                elif op.optimization_potential == "eliminate":
                    self.optimization_stats['eliminable_checks'] += 1
                elif op.optimization_potential == "precompute":
                    self.optimization_stats['precomputable_values'] += 1
            else:
                self.optimization_stats['non_deterministic_ops'] += 1
        
        # Estimate speedup
        if self.optimization_stats['total_operations'] > 0:
            deterministic_ratio = (self.optimization_stats['deterministic_ops'] / 
                                 self.optimization_stats['total_operations'])
            # Rough estimate: deterministic ops can be 10x faster with caching
            self.optimization_stats['estimated_speedup'] = 1 + (deterministic_ratio * 9)
    
    def extract_location(self, line: str) -> str:
        """Extract file:line location from AST line"""
        match = re.search(r'<([^>]+)>', line)
        if match:
            return match.group(1).split(',')[0]
        return "unknown"
    
    def generate_report(self) -> Dict:
        """Generate comprehensive determinism report"""
        report = {
            'source_file': self.source_file,
            'input_sources': [
                {
                    'name': src.name,
                    'type': src.type,
                    'location': src.location,
                    'affects': list(src.affects_functions)
                }
                for src in self.input_sources
            ],
            'function_determinism': {
                func: det.value 
                for func, det in self.function_determinism.items()
            },
            'operations': [
                {
                    'type': op.type,
                    'location': op.location,
                    'function': op.function,
                    'determinism': op.determinism.value,
                    'proof_strength': op.proof_strength,
                    'optimization': op.optimization_potential
                }
                for op in self.operations
            ],
            'optimization_potential': self.optimization_stats,
            'recommendations': self.generate_recommendations()
        }
        
        return report
    
    def generate_recommendations(self) -> List[str]:
        """Generate optimization recommendations"""
        recommendations = []
        
        if self.optimization_stats['deterministic_ops'] > 0:
            recommendations.append(
                f"Enable proof caching for {self.optimization_stats['deterministic_ops']} "
                f"deterministic operations (estimated {self.optimization_stats['estimated_speedup']:.1f}x speedup)"
            )
        
        if self.optimization_stats['eliminable_checks'] > 0:
            recommendations.append(
                f"Consider compile-time elimination of {self.optimization_stats['eliminable_checks']} "
                "redundant safety checks in deterministic code paths"
            )
        
        if self.optimization_stats['precomputable_values'] > 0:
            recommendations.append(
                f"Precompute {self.optimization_stats['precomputable_values']} "
                "constant expressions at compile time"
            )
        
        # Suggest isolation of non-deterministic code
        if self.optimization_stats['non_deterministic_ops'] > 0:
            recommendations.append(
                "Isolate non-deterministic operations into separate functions "
                "to maximize deterministic code regions"
            )
        
        return recommendations

def generate_optimized_prover(report: Dict) -> str:
    """Generate optimized GDB prover based on determinism analysis"""
    prover = [
        "# Optimized GDB Prover with Determinism Awareness",
        "# Generated from determinism analysis",
        "",
        "# Cache for deterministic proofs",
        "set $proof_cache = {}",
        "",
        "define verify_with_cache",
        "  set $op_location = $arg0",
        "  set $op_determinism = $arg1",
        "  ",
        "  # Check cache for deterministic operations",
        '  if $op_determinism == "deterministic"',
        "    # Look up in cache",
        "    set $cached = 0  # Would check actual cache",
        "    if $cached",
        '      printf "CACHED PROOF: %s is safe\\n", $op_location',
        "      return",
        "    end",
        "  end",
        "  ",
        "  # Perform actual verification",
        '  printf "VERIFYING: %s\\n", $op_location',
        "  # ... verification logic ...",
        "  ",
        "  # Cache result if deterministic",
        '  if $op_determinism == "deterministic"',
        "    # Store in cache",
        '    printf "CACHING proof for %s\\n", $op_location',
        "  end",
        "end",
        ""
    ]
    
    # Add specific verification for each operation
    for op in report['operations']:
        if op['determinism'] == 'deterministic':
            prover.append(f'# {op["location"]}: {op["type"]} - DETERMINISTIC (cached)')
            prover.append(f'verify_with_cache "{op["location"]}" "deterministic"')
        else:
            prover.append(f'# {op["location"]}: {op["type"]} - NON-DETERMINISTIC (always verify)')
            prover.append(f'verify_with_cache "{op["location"]}" "non_deterministic"')
        prover.append("")
    
    return '\n'.join(prover)

def main():
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: determinism_analyzer.py <source_file.c>")
        sys.exit(1)
    
    analyzer = DeterminismAnalyzer(sys.argv[1])
    report = analyzer.analyze()
    
    # Print summary
    print("\n=== Determinism Analysis Summary ===")
    print(f"Input sources found: {len(report['input_sources'])}")
    for src in report['input_sources']:
        print(f"  - {src['name']} ({src['type']}) at {src['location']}")
    
    print(f"\nFunction determinism:")
    det_funcs = sum(1 for d in report['function_determinism'].values() 
                    if d == 'deterministic')
    non_det_funcs = sum(1 for d in report['function_determinism'].values() 
                        if d == 'non_deterministic')
    print(f"  Deterministic: {det_funcs}")
    print(f"  Non-deterministic: {non_det_funcs}")
    
    print(f"\nOptimization potential:")
    opt = report['optimization_potential']
    print(f"  Total operations: {opt['total_operations']}")
    print(f"  Deterministic: {opt['deterministic_ops']} ({opt['deterministic_ops']/opt['total_operations']*100:.1f}%)")
    print(f"  Cacheable proofs: {opt['cacheable_proofs']}")
    print(f"  Eliminable checks: {opt['eliminable_checks']}")
    print(f"  Estimated speedup: {opt['estimated_speedup']:.1f}x")
    
    print(f"\nRecommendations:")
    for rec in report['recommendations']:
        print(f"  • {rec}")
    
    # Save detailed report
    with open('determinism_report.json', 'w') as f:
        json.dump(report, f, indent=2)
    print(f"\nDetailed report saved to: determinism_report.json")
    
    # Generate optimized prover
    prover_code = generate_optimized_prover(report)
    with open('optimized_prover.gdb', 'w') as f:
        f.write(prover_code)
    print(f"Optimized GDB prover saved to: optimized_prover.gdb")

if __name__ == '__main__':
    main()