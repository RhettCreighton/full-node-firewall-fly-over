add_library(sky_combat_aircraft STATIC
    aircraft.c
)

target_include_directories(sky_combat_aircraft PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${RAYLIB_INCLUDE_DIRS}
)

target_link_libraries(sky_combat_aircraft
    ${RAYLIB_LIBRARIES}
    m
)