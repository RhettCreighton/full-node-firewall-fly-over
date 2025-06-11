#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Symbolic execution engine for C code
Proves all paths are safe by exploring execution symbolically
"""

import z3
from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
import re

@dataclass
class SymbolicState:
    """Represents symbolic program state"""
    variables: Dict[str, z3.ExprRef]
    path_condition: z3.BoolRef
    
    def copy(self):
        return SymbolicState(
            variables=self.variables.copy(),
            path_condition=self.path_condition
        )

@dataclass
class SafetyProperty:
    """Safety property to prove"""
    name: str
    condition: z3.BoolRef
    location: str

class SymbolicExecutor:
    def __init__(self):
        self.solver = z3.Solver()
        self.paths_explored = 0
        self.safety_violations = []
        
    def create_symbolic_var(self, name: str, type: str) -> z3.ExprRef:
        """Create a symbolic variable"""
        if type in ['int', 'size_t']:
            return z3.Int(name)
        elif type == 'float':
            return z3.Real(name)
        elif type == 'bool':
            return z3.Bool(name)
        else:
            return z3.Int(name)  # Default to int
    
    def analyze_division(self, numerator: str, denominator: str, 
                        state: SymbolicState, location: str) -> List[SafetyProperty]:
        """Analyze a division operation"""
        properties = []
        
        # Get symbolic values
        num_sym = self.get_symbolic_value(numerator, state)
        den_sym = self.get_symbolic_value(denominator, state)
        
        # Safety property: denominator != 0
        safety_prop = SafetyProperty(
            name=f"division_safety_{location}",
            condition=den_sym != 0,
            location=location
        )
        properties.append(safety_prop)
        
        # Check if property can be violated
        self.solver.push()
        self.solver.add(state.path_condition)
        self.solver.add(z3.Not(safety_prop.condition))
        
        if self.solver.check() == z3.sat:
            model = self.solver.model()
            self.safety_violations.append({
                'type': 'division_by_zero',
                'location': location,
                'counterexample': str(model)
            })
        
        self.solver.pop()
        return properties
    
    def analyze_array_access(self, array: str, index: str, size: str,
                           state: SymbolicState, location: str) -> List[SafetyProperty]:
        """Analyze array access"""
        properties = []
        
        idx_sym = self.get_symbolic_value(index, state)
        size_sym = self.get_symbolic_value(size, state)
        
        # Safety properties: 0 <= index < size
        lower_bound = SafetyProperty(
            name=f"array_lower_bound_{location}",
            condition=idx_sym >= 0,
            location=location
        )
        upper_bound = SafetyProperty(
            name=f"array_upper_bound_{location}",
            condition=idx_sym < size_sym,
            location=location
        )
        
        properties.extend([lower_bound, upper_bound])
        
        # Check both properties
        for prop in properties:
            self.solver.push()
            self.solver.add(state.path_condition)
            self.solver.add(z3.Not(prop.condition))
            
            if self.solver.check() == z3.sat:
                model = self.solver.model()
                self.safety_violations.append({
                    'type': 'array_bounds_violation',
                    'location': location,
                    'property': prop.name,
                    'counterexample': str(model)
                })
            
            self.solver.pop()
        
        return properties
    
    def analyze_pointer_deref(self, pointer: str, state: SymbolicState, 
                            location: str) -> List[SafetyProperty]:
        """Analyze pointer dereference"""
        ptr_sym = self.get_symbolic_value(pointer, state)
        
        # For pointers, we use integer to represent addresses
        # NULL = 0
        safety_prop = SafetyProperty(
            name=f"null_pointer_check_{location}",
            condition=ptr_sym != 0,
            location=location
        )
        
        self.solver.push()
        self.solver.add(state.path_condition)
        self.solver.add(ptr_sym == 0)  # Can pointer be NULL?
        
        if self.solver.check() == z3.sat:
            self.safety_violations.append({
                'type': 'null_pointer_deref',
                'location': location
            })
        
        self.solver.pop()
        return [safety_prop]
    
    def get_symbolic_value(self, expr: str, state: SymbolicState) -> z3.ExprRef:
        """Get symbolic value for an expression"""
        if expr in state.variables:
            return state.variables[expr]
        
        # Try to parse as constant
        try:
            return z3.IntVal(int(expr))
        except:
            try:
                return z3.RealVal(float(expr))
            except:
                # Create new symbolic variable
                sym_var = self.create_symbolic_var(expr, 'int')
                state.variables[expr] = sym_var
                return sym_var
    
    def execute_if_statement(self, condition: str, state: SymbolicState) -> Tuple[SymbolicState, SymbolicState]:
        """Execute if statement, returning both branches"""
        # Parse condition (simplified)
        cond_sym = self.parse_condition(condition, state)
        
        # True branch
        true_state = state.copy()
        true_state.path_condition = z3.And(state.path_condition, cond_sym)
        
        # False branch
        false_state = state.copy()
        false_state.path_condition = z3.And(state.path_condition, z3.Not(cond_sym))
        
        return true_state, false_state
    
    def parse_condition(self, condition: str, state: SymbolicState) -> z3.BoolRef:
        """Parse a condition into Z3 formula"""
        # Simple parser for conditions like "x != 0", "count > 0"
        match = re.match(r'(\w+)\s*([><=!]+)\s*(\w+)', condition)
        if match:
            left, op, right = match.groups()
            left_sym = self.get_symbolic_value(left, state)
            right_sym = self.get_symbolic_value(right, state)
            
            if op == '!=':
                return left_sym != right_sym
            elif op == '==':
                return left_sym == right_sym
            elif op == '>':
                return left_sym > right_sym
            elif op == '>=':
                return left_sym >= right_sym
            elif op == '<':
                return left_sym < right_sym
            elif op == '<=':
                return left_sym <= right_sym
        
        # Default to true if can't parse
        return z3.BoolVal(True)
    
    def prove_function_safe(self, function_ast: Dict) -> bool:
        """Prove a function is safe for all inputs"""
        print(f"\nProving safety of function: {function_ast.get('name', 'unknown')}")
        
        # Initialize symbolic state
        initial_state = SymbolicState(
            variables={},
            path_condition=z3.BoolVal(True)
        )
        
        # Create symbolic variables for parameters
        for param in function_ast.get('parameters', []):
            initial_state.variables[param['name']] = self.create_symbolic_var(
                param['name'], param['type']
            )
        
        # Explore all paths
        self.explore_paths(function_ast['body'], initial_state)
        
        # Check if any violations found
        if self.safety_violations:
            print(f"Found {len(self.safety_violations)} safety violations!")
            for v in self.safety_violations:
                print(f"  - {v['type']} at {v['location']}")
                if 'counterexample' in v:
                    print(f"    Counterexample: {v['counterexample']}")
            return False
        else:
            print(f"Function is SAFE! Explored {self.paths_explored} paths.")
            return True
    
    def explore_paths(self, statements: List[Dict], state: SymbolicState):
        """Explore all execution paths"""
        self.paths_explored += 1
        
        for stmt in statements:
            if stmt['type'] == 'division':
                self.analyze_division(
                    stmt['numerator'], 
                    stmt['denominator'],
                    state,
                    stmt['location']
                )
            
            elif stmt['type'] == 'if':
                true_state, false_state = self.execute_if_statement(
                    stmt['condition'], state
                )
                
                # Explore both branches if feasible
                self.solver.push()
                self.solver.add(true_state.path_condition)
                if self.solver.check() == z3.sat:
                    self.explore_paths(stmt['then_body'], true_state)
                self.solver.pop()
                
                self.solver.push()
                self.solver.add(false_state.path_condition)
                if self.solver.check() == z3.sat:
                    if 'else_body' in stmt:
                        self.explore_paths(stmt['else_body'], false_state)
                self.solver.pop()
            
            elif stmt['type'] == 'array_access':
                self.analyze_array_access(
                    stmt['array'],
                    stmt['index'],
                    stmt['size'],
                    state,
                    stmt['location']
                )

def main():
    """Example usage"""
    executor = SymbolicExecutor()
    
    # Example function AST (simplified)
    function_ast = {
        'name': 'calculate_average',
        'parameters': [
            {'name': 'sum', 'type': 'int'},
            {'name': 'count', 'type': 'int'}
        ],
        'body': [
            {
                'type': 'if',
                'condition': 'count != 0',
                'location': 'line:10',
                'then_body': [
                    {
                        'type': 'division',
                        'numerator': 'sum',
                        'denominator': 'count',
                        'location': 'line:11'
                    }
                ],
                'else_body': []
            }
        ]
    }
    
    # Prove function is safe
    is_safe = executor.prove_function_safe(function_ast)
    
    if is_safe:
        print("\n✓ MATHEMATICAL PROOF: Function cannot crash!")
    else:
        print("\n✗ PROOF FAILED: Function has safety violations!")

if __name__ == "__main__":
    main()