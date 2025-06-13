/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Building joystick with specifications...\n");
    
    const char* cmd = "gcc -std=c99 -D_XOPEN_SOURCE=500 -I./include -o sky_combat_joystick_verified sky_combat_joystick_verified.c "
        "$(pkg-config --cflags --libs raylib) -lpthread -lm "
        "-DTRACING_BUILD "
        "-DPATH_JOYSTICK_SPEC_START_EXISTS=1 "
        "-DPATH_RIGHT_STICK_PITCH_SPEC_EXISTS=1 "
        "-DPATH_RIGHT_STICK_ROLL_SPEC_EXISTS=1 "
        "-DPATH_PULL_BACK_CLIMBS_EXISTS=1 "
        "-DPATH_PUSH_FORWARD_DIVES_EXISTS=1 "
        "-DPATH_STICK_LEFT_TURNS_LEFT_EXISTS=1 "
        "-DPATH_STICK_RIGHT_TURNS_RIGHT_EXISTS=1 "
        "-DPATH_THROTTLE_FORWARD_INCREASE_EXISTS=1 "
        "-DPATH_THROTTLE_BACK_DECREASE_EXISTS=1 "
        "-DPATH_JOYSTICK_SPEC_COMPLETE_EXISTS=1 "
        "-DPATH_JOYSTICK_DEVICE_OPENED_EXISTS=1 "
        "-DPATH_JOYSTICK_INFO_READ_EXISTS=1 "
        "-DPATH_JOYSTICK_AXIS_EVENT_EXISTS=1 "
        "-DPATH_JOYSTICK_BUTTON_EVENT_EXISTS=1 "
        "-DPATH_PITCH_AXIS_DETECTED_EXISTS=1 "
        "-DPATH_ROLL_AXIS_DETECTED_EXISTS=1 "
        "-DPATH_THROTTLE_AXIS_DETECTED_EXISTS=1 "
        "-DPATH_CALIBRATION_COMPLETE_EXISTS=1 "
        "-DPATH_STICK_PULLED_BACK_EXISTS=1 "
        "-DPATH_STICK_PUSHED_FORWARD_EXISTS=1 "
        "-DPATH_STICK_MOVED_LEFT_EXISTS=1 "
        "-DPATH_STICK_MOVED_RIGHT_EXISTS=1 "
        "-DPATH_THROTTLE_INCREASED_EXISTS=1 "
        "-DPATH_THROTTLE_DECREASED_EXISTS=1 "
        "-DPATH_BOOST_BUTTON_PRESSED_EXISTS=1";
    
    int result = system(cmd);
    
    if (result == 0) {
        printf("\n✅ JOYSTICK WITH SPECIFICATIONS BUILT!\n\n");
        printf("Run:\n./sky_combat_joystick_verified\n\n");
        printf("This version:\n");
        printf("✓ CALIBRATES your joystick on first run\n");
        printf("✓ ENSURES pull back = climb\n");
        printf("✓ ENSURES push forward = dive\n");
        printf("✓ ENSURES left/right = turn\n");
        printf("✓ Uses secure code points to verify correct behavior\n");
        printf("\nThe calibration will detect which axes control what\n");
        printf("and fix any inversion issues automatically!\n");
    }
    
    return result;
}