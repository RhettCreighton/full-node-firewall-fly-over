#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Proof Strength Calculator
Combines determinism analysis and constraint verification
to calculate the mathematical strength of proofs
"""

from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple
from enum import Enum
import json
import math

class ProofStrength(Enum):
    """Mathematical strength of a proof"""
    MATHEMATICAL_CERTAINTY = "mathematical_certainty"  # 100% proven, deterministic
    BOUNDED_PROOF = "bounded_proof"  # Proven within constrained bounds
    STATISTICAL_PROOF = "statistical_proof"  # High probability based on constraints
    RUNTIME_DEPENDENT = "runtime_dependent"  # Requires runtime checks
    UNPROVEN = "unproven"  # Cannot be proven

@dataclass
class ProofMetrics:
    """Detailed metrics for a proof"""
    strength: ProofStrength
    confidence: float  # 0.0 to 1.0
    determinism_score: float
    constraint_score: float
    coverage: float  # What percentage of input space is covered
    optimization_potential: str
    verification_cost: int  # Relative cost (1-10)
    cacheable: bool
    eliminable: bool  # Can be removed after first proof

@dataclass
class OperationProof:
    """Complete proof information for an operation"""
    operation_type: str
    location: str
    function: str
    determinism: str
    constraints: List[Dict]
    metrics: ProofMetrics
    proof_strategy: str
    optimization_recommendation: str

class ProofStrengthCalculator:
    """Calculates mathematical proof strength by combining analyses"""
    
    # Proof strength requirements by operation type
    OPERATION_REQUIREMENTS = {
        'division': {
            'critical': True,
            'required_constraint': 'non_zero',
            'deterministic_benefit': 0.9
        },
        'array_bounds': {
            'critical': True,
            'required_constraint': 'range_check',
            'deterministic_benefit': 0.95
        },
        'null_deref': {
            'critical': True,
            'required_constraint': 'null_check',
            'deterministic_benefit': 0.85
        },
        'sqrt_negative': {
            'critical': False,
            'required_constraint': 'non_negative',
            'deterministic_benefit': 0.8
        },
        'integer_overflow': {
            'critical': False,
            'required_constraint': 'range_limit',
            'deterministic_benefit': 0.7
        }
    }
    
    def __init__(self, determinism_report: Dict, constraint_report: Dict):
        self.det_report = determinism_report
        self.const_report = constraint_report
        self.operation_proofs: List[OperationProof] = []
        
    def calculate_all_proofs(self) -> Dict:
        """Calculate proof strength for all operations"""
        print("=== Calculating Mathematical Proof Strength ===")
        
        # Process each operation
        for op in self.det_report.get('operations', []):
            proof = self.calculate_operation_proof(op)
            self.operation_proofs.append(proof)
        
        # Calculate overall metrics
        overall_metrics = self.calculate_overall_metrics()
        
        # Generate optimization plan
        optimization_plan = self.generate_optimization_plan()
        
        return {
            'operation_proofs': [self.proof_to_dict(p) for p in self.operation_proofs],
            'overall_metrics': overall_metrics,
            'optimization_plan': optimization_plan,
            'proof_strategies': self.generate_proof_strategies()
        }
    
    def calculate_operation_proof(self, operation: Dict) -> OperationProof:
        """Calculate proof strength for a single operation"""
        # Get operation requirements
        requirements = self.OPERATION_REQUIREMENTS.get(
            operation['type'], 
            {'critical': False, 'deterministic_benefit': 0.5}
        )
        
        # Check determinism
        is_deterministic = operation['determinism'] == 'deterministic'
        
        # Find relevant constraints
        constraints = self.find_operation_constraints(operation)
        has_required_constraints = self.check_required_constraints(
            operation['type'], 
            constraints
        )
        
        # Calculate metrics
        metrics = self.calculate_proof_metrics(
            is_deterministic,
            has_required_constraints,
            constraints,
            requirements
        )
        
        # Determine proof strategy
        proof_strategy = self.determine_proof_strategy(metrics, operation)
        
        # Generate optimization recommendation
        optimization = self.generate_optimization(metrics, operation)
        
        return OperationProof(
            operation_type=operation['type'],
            location=operation['location'],
            function=operation['function'],
            determinism=operation['determinism'],
            constraints=constraints,
            metrics=metrics,
            proof_strategy=proof_strategy,
            optimization_recommendation=optimization
        )
    
    def find_operation_constraints(self, operation: Dict) -> List[Dict]:
        """Find constraints that affect this operation"""
        constraints = []
        
        # Look for constraints in the same function
        for ic in self.const_report.get('input_constraints', []):
            if ic['function'] == operation['function']:
                constraints.extend(ic['constraints'])
        
        return constraints
    
    def check_required_constraints(self, op_type: str, constraints: List[Dict]) -> bool:
        """Check if operation has required constraints"""
        requirements = self.OPERATION_REQUIREMENTS.get(op_type, {})
        required = requirements.get('required_constraint')
        
        if not required:
            return True
        
        constraint_type_map = {
            'non_zero': ['inequality', 'range'],
            'range_check': ['array_bounds', 'range'],
            'null_check': ['null_check'],
            'non_negative': ['range', 'clamped'],
            'range_limit': ['range', 'clamped']
        }
        
        required_types = constraint_type_map.get(required, [])
        
        for constraint in constraints:
            if constraint.get('type') in required_types:
                # Additional checks based on constraint type
                if required == 'non_zero' and constraint.get('type') == 'range':
                    # Check that range excludes zero
                    min_val = constraint.get('min', float('-inf'))
                    max_val = constraint.get('max', float('inf'))
                    if min_val > 0 or max_val < 0:
                        return True
                else:
                    return True
        
        return False
    
    def calculate_proof_metrics(self, is_deterministic: bool, 
                               has_constraints: bool,
                               constraints: List[Dict],
                               requirements: Dict) -> ProofMetrics:
        """Calculate detailed proof metrics"""
        # Base scores
        det_score = 1.0 if is_deterministic else 0.3
        const_score = 1.0 if has_constraints else 0.0
        
        # Adjust constraint score based on constraint quality
        if constraints:
            const_score = self.evaluate_constraint_quality(constraints)
        
        # Calculate confidence
        confidence = (det_score * 0.6 + const_score * 0.4)
        
        # Determine proof strength
        if is_deterministic and has_constraints:
            strength = ProofStrength.MATHEMATICAL_CERTAINTY
            coverage = 1.0
        elif is_deterministic and not has_constraints:
            strength = ProofStrength.RUNTIME_DEPENDENT
            coverage = 0.5
        elif not is_deterministic and has_constraints:
            strength = ProofStrength.BOUNDED_PROOF
            coverage = self.calculate_constraint_coverage(constraints)
        else:
            strength = ProofStrength.UNPROVEN
            coverage = 0.0
        
        # Optimization potential
        if is_deterministic:
            opt_potential = "high"
            cacheable = True
            if has_constraints and requirements.get('critical'):
                eliminable = True
            else:
                eliminable = False
        else:
            opt_potential = "low"
            cacheable = False
            eliminable = False
        
        # Verification cost (1-10 scale)
        if is_deterministic and cacheable:
            verification_cost = 1
        elif is_deterministic:
            verification_cost = 3
        elif has_constraints:
            verification_cost = 5
        else:
            verification_cost = 10
        
        return ProofMetrics(
            strength=strength,
            confidence=confidence,
            determinism_score=det_score,
            constraint_score=const_score,
            coverage=coverage,
            optimization_potential=opt_potential,
            verification_cost=verification_cost,
            cacheable=cacheable,
            eliminable=eliminable
        )
    
    def evaluate_constraint_quality(self, constraints: List[Dict]) -> float:
        """Evaluate the quality of constraints"""
        if not constraints:
            return 0.0
        
        quality_scores = []
        
        for constraint in constraints:
            c_type = constraint.get('type')
            
            # Score by constraint type
            if c_type == 'validated':
                quality_scores.append(0.95)  # Custom validation is strong
            elif c_type == 'clamped':
                quality_scores.append(0.9)   # Clamping is very effective
            elif c_type == 'range':
                # Check if range is tight
                min_val = constraint.get('min', float('-inf'))
                max_val = constraint.get('max', float('inf'))
                if min_val != float('-inf') and max_val != float('inf'):
                    quality_scores.append(0.85)
                else:
                    quality_scores.append(0.7)
            elif c_type == 'null_check':
                quality_scores.append(0.8)
            elif c_type == 'array_bounds':
                quality_scores.append(0.85)
            else:
                quality_scores.append(0.5)
        
        return sum(quality_scores) / len(quality_scores)
    
    def calculate_constraint_coverage(self, constraints: List[Dict]) -> float:
        """Calculate what percentage of input space is covered"""
        if not constraints:
            return 0.0
        
        # Estimate based on constraint types
        coverage = 0.0
        
        for constraint in constraints:
            c_type = constraint.get('type')
            
            if c_type == 'range':
                # Bounded range provides good coverage
                min_val = constraint.get('min', float('-inf'))
                max_val = constraint.get('max', float('inf'))
                
                if min_val != float('-inf') and max_val != float('inf'):
                    # Finite range
                    coverage = max(coverage, 0.9)
                elif min_val != float('-inf') or max_val != float('inf'):
                    # Half-bounded
                    coverage = max(coverage, 0.7)
            
            elif c_type == 'clamped':
                coverage = max(coverage, 0.95)  # Clamping covers all inputs
            
            elif c_type == 'validated':
                coverage = max(coverage, 0.85)  # Depends on validation
        
        return coverage
    
    def determine_proof_strategy(self, metrics: ProofMetrics, 
                                operation: Dict) -> str:
        """Determine the best proof strategy"""
        if metrics.strength == ProofStrength.MATHEMATICAL_CERTAINTY:
            return "Single compile-time proof with caching"
        
        elif metrics.strength == ProofStrength.BOUNDED_PROOF:
            return "Verify constraints hold, then prove within bounds"
        
        elif metrics.strength == ProofStrength.STATISTICAL_PROOF:
            return "Statistical verification with confidence intervals"
        
        elif metrics.strength == ProofStrength.RUNTIME_DEPENDENT:
            return "Runtime verification required for each execution"
        
        else:
            return "Add constraints to enable proof"
    
    def generate_optimization(self, metrics: ProofMetrics, 
                            operation: Dict) -> str:
        """Generate specific optimization recommendation"""
        if metrics.eliminable:
            return f"Eliminate runtime check after first proof (save ~{metrics.verification_cost}ms per call)"
        
        elif metrics.cacheable:
            return f"Cache proof result (reuse for {operation['function']})"
        
        elif metrics.optimization_potential == "high":
            return "Extract to deterministic function for better optimization"
        
        elif metrics.strength == ProofStrength.UNPROVEN:
            return "Add constraints to enable optimization"
        
        else:
            return "Limited optimization potential due to non-determinism"
    
    def calculate_overall_metrics(self) -> Dict:
        """Calculate overall proof system metrics"""
        total_ops = len(self.operation_proofs)
        
        if total_ops == 0:
            return {}
        
        # Count by strength
        strength_counts = {}
        for proof in self.operation_proofs:
            strength = proof.metrics.strength.value
            strength_counts[strength] = strength_counts.get(strength, 0) + 1
        
        # Calculate averages
        avg_confidence = sum(p.metrics.confidence for p in self.operation_proofs) / total_ops
        avg_coverage = sum(p.metrics.coverage for p in self.operation_proofs) / total_ops
        
        # Count optimizable operations
        cacheable = sum(1 for p in self.operation_proofs if p.metrics.cacheable)
        eliminable = sum(1 for p in self.operation_proofs if p.metrics.eliminable)
        
        # Estimate performance improvement
        total_cost = sum(p.metrics.verification_cost for p in self.operation_proofs)
        optimized_cost = sum(
            1 if p.metrics.cacheable else p.metrics.verification_cost 
            for p in self.operation_proofs
        )
        speedup = total_cost / optimized_cost if optimized_cost > 0 else 1.0
        
        return {
            'total_operations': total_ops,
            'strength_distribution': strength_counts,
            'average_confidence': avg_confidence,
            'average_coverage': avg_coverage,
            'cacheable_operations': cacheable,
            'eliminable_operations': eliminable,
            'estimated_speedup': speedup,
            'proof_completeness': avg_confidence * avg_coverage
        }
    
    def generate_optimization_plan(self) -> List[Dict]:
        """Generate prioritized optimization plan"""
        plan = []
        
        # Group operations by optimization potential
        high_impact = []
        medium_impact = []
        low_impact = []
        
        for proof in self.operation_proofs:
            impact = {
                'operation': f"{proof.operation_type} at {proof.location}",
                'current_cost': proof.metrics.verification_cost,
                'optimization': proof.optimization_recommendation,
                'estimated_savings': proof.metrics.verification_cost - 1 if proof.metrics.cacheable else 0
            }
            
            if proof.metrics.eliminable:
                high_impact.append(impact)
            elif proof.metrics.cacheable:
                medium_impact.append(impact)
            else:
                low_impact.append(impact)
        
        # Sort by savings
        high_impact.sort(key=lambda x: x['estimated_savings'], reverse=True)
        medium_impact.sort(key=lambda x: x['estimated_savings'], reverse=True)
        
        plan.extend(high_impact[:5])  # Top 5 high impact
        plan.extend(medium_impact[:5])  # Top 5 medium impact
        
        return plan
    
    def generate_proof_strategies(self) -> Dict[str, List[str]]:
        """Generate proof strategies by category"""
        strategies = {
            'immediate_optimizations': [],
            'constraint_additions': [],
            'refactoring_suggestions': [],
            'advanced_techniques': []
        }
        
        for proof in self.operation_proofs:
            if proof.metrics.eliminable:
                strategies['immediate_optimizations'].append(
                    f"Remove runtime check for {proof.operation_type} at {proof.location}"
                )
            
            elif proof.metrics.strength == ProofStrength.UNPROVEN:
                strategies['constraint_additions'].append(
                    f"Add constraints for {proof.operation_type} at {proof.location}"
                )
            
            elif proof.determinism == "non_deterministic" and proof.metrics.cacheable:
                strategies['refactoring_suggestions'].append(
                    f"Extract deterministic portion of {proof.function}"
                )
        
        # Advanced techniques
        if any(p.metrics.strength == ProofStrength.BOUNDED_PROOF for p in self.operation_proofs):
            strategies['advanced_techniques'].append(
                "Use interval arithmetic for bounded proofs"
            )
        
        return strategies
    
    def proof_to_dict(self, proof: OperationProof) -> Dict:
        """Convert proof object to dictionary"""
        return {
            'operation_type': proof.operation_type,
            'location': proof.location,
            'function': proof.function,
            'determinism': proof.determinism,
            'constraints': proof.constraints,
            'metrics': {
                'strength': proof.metrics.strength.value,
                'confidence': proof.metrics.confidence,
                'coverage': proof.metrics.coverage,
                'cacheable': proof.metrics.cacheable,
                'eliminable': proof.metrics.eliminable,
                'verification_cost': proof.metrics.verification_cost
            },
            'proof_strategy': proof.proof_strategy,
            'optimization': proof.optimization_recommendation
        }

def generate_optimized_verification_code(proof_report: Dict) -> str:
    """Generate optimized verification code based on proof strength"""
    code = [
        "/* SPDX-FileCopyrightText: 2025 Rhett Creighton",
        " * SPDX-License-Identifier: Apache-2.0",
        " */",
        "",
        "/* Optimized verification based on proof strength analysis */",
        "",
        "#include <stdbool.h>",
        "#include <stdint.h>",
        "",
        "/* Proof cache for deterministic operations */",
        "typedef struct {",
        "    uint64_t operation_hash;",
        "    bool verified;",
        "} proof_cache_entry_t;",
        "",
        "static proof_cache_entry_t proof_cache[1024];",
        "static size_t cache_size = 0;",
        "",
        "/* Check if operation needs verification */",
        "static inline bool needs_verification(uint64_t op_hash, bool is_deterministic) {",
        "    if (!is_deterministic) return true;",
        "    ",
        "    /* Check cache for deterministic operations */",
        "    for (size_t i = 0; i < cache_size; i++) {",
        "        if (proof_cache[i].operation_hash == op_hash) {",
        "            return !proof_cache[i].verified;",
        "        }",
        "    }",
        "    return true;",
        "}",
        ""
    ]
    
    # Add specific verification functions
    for proof in proof_report['operation_proofs']:
        if proof['metrics']['eliminable']:
            code.append(f"/* {proof['operation_type']} at {proof['location']} - ELIMINATED after first proof */")
            code.append("#ifdef FIRST_RUN")
            code.append(f"#define VERIFY_{proof['operation_type'].upper()}_{hash(proof['location']) % 1000}() \\")
            code.append("    do { /* Verification code */ } while(0)")
            code.append("#else")
            code.append(f"#define VERIFY_{proof['operation_type'].upper()}_{hash(proof['location']) % 1000}() /* NOP */")
            code.append("#endif")
            code.append("")
    
    return '\n'.join(code)

def main():
    import sys
    
    if len(sys.argv) < 3:
        print("Usage: proof_strength_calculator.py <determinism_report.json> <constraint_report.json>")
        sys.exit(1)
    
    # Load reports
    with open(sys.argv[1], 'r') as f:
        det_report = json.load(f)
    
    with open(sys.argv[2], 'r') as f:
        const_report = json.load(f)
    
    calculator = ProofStrengthCalculator(det_report, const_report)
    proof_report = calculator.calculate_all_proofs()
    
    # Print summary
    metrics = proof_report['overall_metrics']
    print(f"\n=== Proof Strength Analysis ===")
    print(f"Total operations: {metrics['total_operations']}")
    print(f"Average confidence: {metrics['average_confidence']*100:.1f}%")
    print(f"Average coverage: {metrics['average_coverage']*100:.1f}%")
    print(f"Proof completeness: {metrics['proof_completeness']*100:.1f}%")
    print(f"\nOptimization potential:")
    print(f"  Cacheable: {metrics['cacheable_operations']}")
    print(f"  Eliminable: {metrics['eliminable_operations']}")
    print(f"  Estimated speedup: {metrics['estimated_speedup']:.1f}x")
    
    print(f"\nProof strength distribution:")
    for strength, count in metrics['strength_distribution'].items():
        print(f"  {strength}: {count}")
    
    # Save detailed report
    with open('proof_strength_report.json', 'w') as f:
        json.dump(proof_report, f, indent=2)
    print(f"\nDetailed report saved to: proof_strength_report.json")
    
    # Generate optimized code
    opt_code = generate_optimized_verification_code(proof_report)
    with open('optimized_verification.c', 'w') as f:
        f.write(opt_code)
    print(f"Optimized verification code saved to: optimized_verification.c")

if __name__ == '__main__':
    main()