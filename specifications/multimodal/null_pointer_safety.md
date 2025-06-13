# Multimodal Specification: Null Pointer Safety

## Natural Language
"Every pointer parameter must be checked for NULL before any dereference operation."

## Formal Logic
```
∀ function f, pointer p:
  called(f, p) → (checked(p) ∨ ¬dereference(p))
```

## Pseudocode
```
FUNCTION safe_operation(pointer):
  IF pointer IS NULL THEN
    RETURN ERROR_NULL_POINTER
  END IF
  
  // Safe to use pointer now
  value = *pointer
  RETURN SUCCESS
END FUNCTION
```

## Valid Example
```c
int process_data(struct data *ptr) {
    if (!ptr) return ERROR_NULL_POINTER;  // ✓ Checked first
    return ptr->value * 2;                // ✓ Safe to dereference
}
```

## Invalid Example
```c
int bad_process(struct data *ptr) {
    int val = ptr->value;     // ✗ No null check!
    if (!ptr) return -1;      // ✗ Too late!
    return val;
}
```

## GDB Proof Script
```gdb
# Catch any dereference of NULL
catch signal SIGSEGV
commands
  print "NULL pointer dereferenced!"
  quit 1
end
```

## Unit Test
```c
void test_null_safety() {
    assert(process_data(NULL) == ERROR_NULL_POINTER);
    assert(process_data(&valid_data) == SUCCESS);
}
```

## Property Test
```
property null_pointers_never_dereferenced:
  forall ptr in [NULL, valid_pointer]:
    result = safe_function(ptr)
    assert no_crash_occurred()
```

## Convergence Check
All eight expressions require:
- NULL check BEFORE dereference
- Return error code on NULL
- No pointer operations without validation