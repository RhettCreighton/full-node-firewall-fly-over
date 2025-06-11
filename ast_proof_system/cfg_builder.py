#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Control Flow Graph builder from C AST
Builds complete CFG to find ALL execution paths
"""

import networkx as nx
from typing import Dict, List, Set, Tuple, Optional
from dataclasses import dataclass
import json

@dataclass
class CFGNode:
    """Node in the control flow graph"""
    id: int
    type: str  # 'statement', 'branch', 'merge', 'call', 'return'
    code: str
    line: int
    dangerous_ops: List[str] = None
    
    def __post_init__(self):
        if self.dangerous_ops is None:
            self.dangerous_ops = []

class CFGBuilder:
    def __init__(self):
        self.graph = nx.DiGraph()
        self.node_counter = 0
        self.function_cfgs: Dict[str, nx.DiGraph] = {}
        
    def create_node(self, type: str, code: str, line: int) -> CFGNode:
        """Create a new CFG node"""
        node = CFGNode(
            id=self.node_counter,
            type=type,
            code=code,
            line=line
        )
        self.node_counter += 1
        self.graph.add_node(node.id, data=node)
        return node
    
    def add_edge(self, from_id: int, to_id: int, condition: str = None):
        """Add edge between nodes"""
        self.graph.add_edge(from_id, to_id, condition=condition)
    
    def build_from_ast(self, ast_data: Dict) -> nx.DiGraph:
        """Build CFG from AST data"""
        if ast_data['type'] == 'FunctionDecl':
            return self.build_function_cfg(ast_data)
        elif ast_data['type'] == 'CompoundStmt':
            return self.build_compound_cfg(ast_data['children'])
        else:
            return self.build_statement_cfg(ast_data)
    
    def build_function_cfg(self, func_ast: Dict) -> nx.DiGraph:
        """Build CFG for a function"""
        # Entry node
        entry = self.create_node('entry', f"function {func_ast['name']}", 0)
        
        # Build body
        if 'body' in func_ast:
            body_start = self.build_from_ast(func_ast['body'])
            if body_start:
                self.add_edge(entry.id, body_start)
        
        # Exit node
        exit = self.create_node('exit', f"return from {func_ast['name']}", -1)
        
        # Connect all return statements to exit
        for node_id in self.graph.nodes():
            node = self.graph.nodes[node_id]['data']
            if node.type == 'return':
                self.add_edge(node_id, exit.id)
        
        return entry.id
    
    def build_compound_cfg(self, statements: List[Dict]) -> Optional[int]:
        """Build CFG for compound statement"""
        if not statements:
            return None
            
        prev_node = None
        first_node = None
        
        for stmt in statements:
            stmt_start = self.build_from_ast(stmt)
            
            if first_node is None:
                first_node = stmt_start
            
            if prev_node is not None and stmt_start is not None:
                # Connect previous statement to this one
                self.connect_sequential(prev_node, stmt_start)
            
            prev_node = self.get_last_node(stmt_start)
        
        return first_node
    
    def build_if_cfg(self, if_ast: Dict) -> int:
        """Build CFG for if statement"""
        # Branch node
        branch = self.create_node(
            'branch',
            f"if ({if_ast['condition']})",
            if_ast.get('line', 0)
        )
        
        # Check if condition has dangerous operations
        if '/' in if_ast['condition']:
            branch.dangerous_ops.append('division')
        
        # Then branch
        then_start = self.build_from_ast(if_ast['then'])
        self.add_edge(branch.id, then_start, condition='true')
        
        # Else branch (if exists)
        if 'else' in if_ast:
            else_start = self.build_from_ast(if_ast['else'])
            self.add_edge(branch.id, else_start, condition='false')
        
        # Merge node
        merge = self.create_node('merge', 'endif', -1)
        
        # Connect branches to merge
        then_end = self.get_last_node(then_start)
        if then_end:
            self.add_edge(then_end, merge.id)
        
        if 'else' in if_ast:
            else_end = self.get_last_node(else_start)
            if else_end:
                self.add_edge(else_end, merge.id)
        else:
            # Direct edge for false case
            self.add_edge(branch.id, merge.id, condition='false')
        
        return branch.id
    
    def build_statement_cfg(self, stmt_ast: Dict) -> int:
        """Build CFG for a single statement"""
        node = self.create_node(
            'statement',
            stmt_ast.get('code', str(stmt_ast)),
            stmt_ast.get('line', 0)
        )
        
        # Identify dangerous operations
        if stmt_ast['type'] == 'BinaryOperator' and stmt_ast.get('op') == '/':
            node.dangerous_ops.append('division')
        elif stmt_ast['type'] == 'CallExpr':
            func_name = stmt_ast.get('function', '')
            if 'Vector3Normalize' in func_name:
                node.dangerous_ops.append('normalize')
            elif func_name in ['strcpy', 'strcat', 'sprintf']:
                node.dangerous_ops.append('unsafe_string')
        elif stmt_ast['type'] == 'ArraySubscriptExpr':
            node.dangerous_ops.append('array_access')
        elif stmt_ast['type'] == 'MemberExpr' and stmt_ast.get('arrow', False):
            node.dangerous_ops.append('pointer_deref')
        
        return node.id
    
    def connect_sequential(self, from_node: int, to_node: int):
        """Connect nodes sequentially"""
        # Find all nodes without outgoing edges from from_node's subgraph
        reachable = nx.descendants(self.graph, from_node)
        reachable.add(from_node)
        
        for node in reachable:
            if self.graph.out_degree(node) == 0:
                self.add_edge(node, to_node)
    
    def get_last_node(self, start_node: int) -> Optional[int]:
        """Get the last node(s) reachable from start"""
        if start_node is None:
            return None
            
        reachable = nx.descendants(self.graph, start_node)
        reachable.add(start_node)
        
        # Find nodes with no outgoing edges
        terminals = [n for n in reachable if self.graph.out_degree(n) == 0]
        
        return terminals[0] if terminals else None
    
    def find_all_paths(self, start: int, end: int) -> List[List[int]]:
        """Find all paths from start to end"""
        return list(nx.all_simple_paths(self.graph, start, end))
    
    def analyze_path_safety(self, path: List[int]) -> Dict[str, List[str]]:
        """Analyze safety of a specific path"""
        dangerous_ops = []
        guards = []
        
        for i, node_id in enumerate(path):
            node = self.graph.nodes[node_id]['data']
            
            # Collect dangerous operations
            dangerous_ops.extend(node.dangerous_ops)
            
            # Collect guards (if statements that protect later operations)
            if node.type == 'branch':
                # Check if this guards any dangerous operations
                protected_ops = []
                for j in range(i + 1, len(path)):
                    future_node = self.graph.nodes[path[j]]['data']
                    protected_ops.extend(future_node.dangerous_ops)
                
                if protected_ops:
                    guards.append({
                        'condition': node.code,
                        'protects': protected_ops
                    })
        
        return {
            'dangerous_operations': dangerous_ops,
            'guards': guards
        }
    
    def generate_path_proof(self, function_name: str) -> Dict:
        """Generate proof that all paths in function are safe"""
        # Find entry and exit nodes
        entry = None
        exit = None
        
        for node_id in self.graph.nodes():
            node = self.graph.nodes[node_id]['data']
            if node.type == 'entry' and function_name in node.code:
                entry = node_id
            elif node.type == 'exit' and function_name in node.code:
                exit = node_id
        
        if not entry or not exit:
            return {'error': f'Could not find entry/exit for {function_name}'}
        
        # Find all paths
        all_paths = self.find_all_paths(entry, exit)
        
        # Analyze each path
        path_analyses = []
        unsafe_paths = []
        
        for path in all_paths:
            analysis = self.analyze_path_safety(path)
            path_analyses.append(analysis)
            
            # Check if path has unguarded dangerous operations
            unguarded_ops = []
            for op in analysis['dangerous_operations']:
                is_guarded = False
                for guard in analysis['guards']:
                    if op in guard['protects']:
                        is_guarded = True
                        break
                if not is_guarded:
                    unguarded_ops.append(op)
            
            if unguarded_ops:
                unsafe_paths.append({
                    'path': path,
                    'unguarded_operations': unguarded_ops
                })
        
        return {
            'function': function_name,
            'total_paths': len(all_paths),
            'unsafe_paths': len(unsafe_paths),
            'path_analyses': path_analyses,
            'unsafe_details': unsafe_paths,
            'is_safe': len(unsafe_paths) == 0
        }
    
    def visualize_cfg(self, output_file: str = 'cfg.dot'):
        """Export CFG to Graphviz format"""
        with open(output_file, 'w') as f:
            f.write("digraph CFG {\n")
            f.write("  rankdir=TB;\n")
            
            # Nodes
            for node_id in self.graph.nodes():
                node = self.graph.nodes[node_id]['data']
                label = f"{node.code}\\n"
                if node.dangerous_ops:
                    label += f"⚠️ {', '.join(node.dangerous_ops)}"
                
                color = 'red' if node.dangerous_ops else 'black'
                f.write(f'  n{node_id} [label="{label}", color={color}];\n')
            
            # Edges
            for from_id, to_id, data in self.graph.edges(data=True):
                label = data.get('condition', '')
                f.write(f'  n{from_id} -> n{to_id}')
                if label:
                    f.write(f' [label="{label}"]')
                f.write(';\n')
            
            f.write("}\n")

def main():
    """Example usage"""
    # Example AST (simplified)
    ast = {
        'type': 'FunctionDecl',
        'name': 'enemies_spawn_formation',
        'body': {
            'type': 'CompoundStmt',
            'children': [
                {
                    'type': 'IfStmt',
                    'condition': 'count <= 0',
                    'line': 187,
                    'then': {
                        'type': 'ReturnStmt',
                        'line': 187
                    }
                },
                {
                    'type': 'ForStmt',
                    'line': 190,
                    'body': {
                        'type': 'CompoundStmt',
                        'children': [
                            {
                                'type': 'BinaryOperator',
                                'op': '/',
                                'line': 191,
                                'code': 'angle = (2.0f * PI * i) / count'
                            }
                        ]
                    }
                }
            ]
        }
    }
    
    builder = CFGBuilder()
    builder.build_from_ast(ast)
    
    # Generate proof
    proof = builder.generate_path_proof('enemies_spawn_formation')
    
    print("=== CFG Path Analysis ===")
    print(json.dumps(proof, indent=2))
    
    # Visualize
    builder.visualize_cfg('enemies_cfg.dot')
    print("\nCFG exported to enemies_cfg.dot")
    print("Visualize with: dot -Tpng enemies_cfg.dot -o enemies_cfg.png")

if __name__ == "__main__":
    main()