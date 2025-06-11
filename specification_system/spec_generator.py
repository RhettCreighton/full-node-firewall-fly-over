#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

"""
Specification-Driven Code Generator
Generates safe wrapper functions that enforce specifications
"""

import re
import sys
from pathlib import Path

# Specification templates for common patterns
SPEC_TEMPLATES = {
    'safe_array_access': '''
// SPECIFICATION: Array access must be bounds-checked
static inline {return_type} {name}_safe(
    {array_type} array,
    size_t index,
    size_t size
) {{
    SPEC_REQUIRES(array != NULL);
    SPEC_REQUIRES(size > 0);
    
    if (index >= size) {{
        spec_array_overflow();
        index = size - 1;  // Safe fallback
    }}
    
    return array[index];
}}
''',
    
    'safe_pointer_op': '''
// SPECIFICATION: Pointer operations must be null-safe
static inline {return_type} {name}_safe(
    {ptr_type}* ptr
) {{
    SPEC_REQUIRES(ptr != NULL);
    
    if (!ptr) {{
        spec_null_deref();
        static {ptr_type} safe_default = {{0}};
        return &safe_default;
    }}
    
    return ptr->{member};
}}
''',
    
    'safe_arithmetic': '''
// SPECIFICATION: Arithmetic must not overflow/underflow
static inline {type} {name}_safe(
    {type} a,
    {type} b
) {{
    {type} result;
    
    if (__builtin_{op}_overflow(a, b, &result)) {{
        spec_arithmetic_overflow();
        return {safe_default};
    }}
    
    return result;
}}
''',
    
    'bounded_value': '''
// SPECIFICATION: Value must stay within bounds
typedef struct {{
    {type} value;
    {type} min;
    {type} max;
}} {name}_bounded_t;

static inline void {name}_set(
    {name}_bounded_t* bounded,
    {type} new_value
) {{
    SPEC_REQUIRES(bounded != NULL);
    SPEC_INVARIANT(bounded->min <= bounded->max);
    
    if (new_value < bounded->min) {{
        bounded->value = bounded->min;
    }} else if (new_value > bounded->max) {{
        bounded->value = bounded->max;
    }} else {{
        bounded->value = new_value;
    }}
    
    SPEC_ENSURES(bounded->value >= bounded->min);
    SPEC_ENSURES(bounded->value <= bounded->max);
}}
'''
}

def parse_unsafe_function(code):
    """Parse unsafe function and determine what specifications it needs"""
    specs_needed = []
    
    # Check for array access without bounds
    if re.search(r'\[[^\]]+\]', code) and 'size' not in code:
        specs_needed.append('bounds_checking')
    
    # Check for pointer dereference without null check
    if re.search(r'->', code) and not re.search(r'if.*!=.*NULL', code):
        specs_needed.append('null_checking')
    
    # Check for arithmetic that could overflow
    if re.search(r'[+\-*/]', code) and 'overflow' not in code:
        specs_needed.append('overflow_checking')
    
    # Check for division without zero check
    if re.search(r'/', code) and not re.search(r'if.*!=.*0', code):
        specs_needed.append('div_zero_checking')
    
    return specs_needed

def generate_safe_wrapper(func_name, func_code, specs):
    """Generate a safe wrapper that enforces specifications"""
    wrapper = f"// Safe wrapper for {func_name}\n"
    wrapper += f"// Enforces: {', '.join(specs)}\n"
    
    # Add appropriate safety macros based on needed specs
    if 'bounds_checking' in specs:
        wrapper += "// TODO: Add SPEC_MONITOR_RANGE\n"
    
    if 'null_checking' in specs:
        wrapper += "// TODO: Add SPEC_MONITOR_PTR\n"
    
    if 'overflow_checking' in specs:
        wrapper += "// TODO: Add overflow checks\n"
        
    return wrapper

def generate_specification_header(spec_file):
    """Generate header file with all specifications"""
    header = """/* Auto-generated specification enforcement header */
#ifndef GENERATED_SPECIFICATIONS_H
#define GENERATED_SPECIFICATIONS_H

#include "sky_combat/specification_enforcement.h"

// ============================================================================
// GENERATED SAFE WRAPPERS
// ============================================================================

"""
    
    # Read specification file
    with open(spec_file, 'r') as f:
        specs = f.readlines()
    
    # Generate safe wrappers for each specification
    for spec in specs:
        if spec.strip().startswith("SPEC:"):
            parts = spec.strip().split(":")
            if len(parts) >= 3:
                spec_type = parts[1]
                spec_detail = parts[2]
                
                if spec_type == "ARRAY_BOUNDS":
                    header += SPEC_TEMPLATES['safe_array_access'].format(
                        return_type="int",
                        name=spec_detail.lower(),
                        array_type="int*"
                    )
                elif spec_type == "NULL_SAFE":
                    header += SPEC_TEMPLATES['safe_pointer_op'].format(
                        return_type="int",
                        name=spec_detail.lower(),
                        ptr_type="void",
                        member="value"
                    )
    
    header += "\n#endif // GENERATED_SPECIFICATIONS_H\n"
    return header

def main():
    if len(sys.argv) < 2:
        print("Usage: spec_generator.py <specification_file>")
        sys.exit(1)
    
    spec_file = sys.argv[1]
    output_file = "generated_specifications.h"
    
    # Generate specification header
    header_content = generate_specification_header(spec_file)
    
    # Write to file
    with open(output_file, 'w') as f:
        f.write(header_content)
    
    print(f"Generated {output_file} with specification enforcement")

if __name__ == "__main__":
    main()