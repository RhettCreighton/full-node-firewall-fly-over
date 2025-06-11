# Example CMake integration for GDB Proof System

# Include the proof system
include(RunProofs.cmake)

# Example: Add proofs to a target
# add_gdb_proofs(sky_combat_ultimate)

# Example: Add proofs to multiple targets
# foreach(target IN LISTS game_targets)
#     add_gdb_proofs(${target})
# endforeach()

# Generate summary of all proofs
generate_proof_summary()

# Custom target to run all proofs
add_custom_target(run_all_proofs
    COMMAND ${CMAKE_CTEST_COMMAND} -L safety --output-on-failure
    COMMENT "Running all GDB safety proofs..."
    VERBATIM
)

# Installation of proof reports
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/gdb_proofs/
    DESTINATION share/sky_combat/safety_proofs
    OPTIONAL
    FILES_MATCHING PATTERN "*.json"
)