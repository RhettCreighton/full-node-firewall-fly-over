# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# This module ensures 3D building specifications are met
# The build WILL FAIL if requirements are not satisfied

function(verify_3d_buildings target)
    message(STATUS "Verifying 3D building specifications for ${target}...")
    
    # Add a pre-build command to verify specifications
    add_custom_command(
        TARGET ${target}
        PRE_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/verify_3d_specs.sh
        COMMENT "Verifying 3D building specifications (build will fail if not met)"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Add compile definitions to enforce specs
    target_compile_definitions(${target} PRIVATE
        ENFORCE_3D_BUILDING_SPECS
        REQUIRE_3D_BUILDINGS
    )
    
    message(STATUS "3D building verification configured - build blocked until proven")
endfunction()