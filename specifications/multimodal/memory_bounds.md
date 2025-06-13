# Multimodal Specification: Memory Bounds

## Natural Language
"Every array access must use an index that is guaranteed to be within the array's bounds."

## Formal Logic
```
∀ array A, index i:
  access(A[i]) → (0 ≤ i < length(A))
```

## Pseudocode
```
FUNCTION safe_array_access(array, index):
  safe_index = index MODULO array.length
  IF safe_index < 0 THEN
    safe_index = safe_index + array.length
  END IF
  RETURN array[safe_index]
END FUNCTION
```

## Valid Example
```c
float get_value(float arr[], size_t size, int idx) {
    size_t safe_idx = ((idx % size) + size) % size;  // ✓ Always valid
    return arr[safe_idx];
}
```

## Invalid Example
```c
float bad_get(float arr[], int idx) {
    return arr[idx];  // ✗ No bounds check!
}
```

## GDB Proof Script
```gdb
# Monitor array bounds
break __fortify_fail
commands
  print "Buffer overflow detected!"
  quit 1
end
```

## Unit Test
```c
void test_bounds_safety() {
    float arr[5] = {1, 2, 3, 4, 5};
    assert(get_value(arr, 5, -1) == 5);    // Wraps to last
    assert(get_value(arr, 5, 10) == 1);    // Wraps to first
    assert(get_value(arr, 5, 2) == 3);     // Normal access
}
```

## Property Test
```
property array_access_always_safe:
  forall size in [1..1000]:
    forall index in [-1000000..1000000]:
      array = create_array(size)
      value = safe_access(array, index)
      assert no_crash_occurred()
```

## Convergence Check
All expressions require:
- Modulo wrapping for out-of-bounds
- Handle negative indices correctly
- Never access outside allocation