/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include "sky_combat/core/secure_code_points.h"

/* Joystick state */
typedef struct {
    int fd;
    int axes[8];
    int buttons[12];
    bool initialized;
} joystick_state_t;

/* Initialize joystick */
bool init_joystick(joystick_state_t* js, const char* device) {
    SECURE_CODE_POINT(JOYSTICK_INIT_START, "entering init");
    
    if (!js) {
        SECURE_CODE_POINT(JOYSTICK_INIT_NULL_STATE, "js == NULL");
        return false;
    }
    
    if (!device) {
        SECURE_CODE_POINT(JOYSTICK_INIT_NULL_DEVICE, "device == NULL");
        return false;
    }
    
    js->fd = open(device, O_RDONLY | O_NONBLOCK);
    if (js->fd < 0) {
        SECURE_CODE_POINT(JOYSTICK_OPEN_FAILED, "open() failed");
        return false;
    }
    
    SECURE_CODE_POINT(JOYSTICK_OPENED, "fd valid");
    
    /* Initialize state */
    memset(js->axes, 0, sizeof(js->axes));
    memset(js->buttons, 0, sizeof(js->buttons));
    js->initialized = true;
    
    SECURE_CODE_POINT(JOYSTICK_INIT_SUCCESS, "initialization complete");
    return true;
}

/* Process joystick event */
void process_joystick_event(joystick_state_t* js, struct js_event* event) {
    SECURE_CODE_POINT(PROCESS_EVENT_START, "entering process");
    
    if (!js) {
        SECURE_CODE_POINT(PROCESS_NULL_STATE, "js == NULL");
        return;
    }
    
    if (!event) {
        SECURE_CODE_POINT(PROCESS_NULL_EVENT, "event == NULL");
        return;
    }
    
    if (!js->initialized) {
        SECURE_CODE_POINT(PROCESS_NOT_INITIALIZED, "!initialized");
        return;
    }
    
    /* Handle event type */
    switch (event->type & ~JS_EVENT_INIT) {
    case JS_EVENT_AXIS:
        SECURE_CODE_POINT(EVENT_TYPE_AXIS, "axis event");
        
        if (event->number >= 8) {
            SECURE_CODE_POINT(AXIS_OUT_OF_BOUNDS, "axis >= 8");
            return;
        }
        
        /* Special handling for right stick Y */
        if (event->number == 5) {
            SECURE_CODE_POINT(RIGHT_STICK_Y_AXIS_5, "axis == 5");
            js->axes[5] = event->value;
        }
        else if (event->number == 3) {
            /* This should be unreachable in correct code */
            SECURE_CODE_POINT(RIGHT_STICK_Y_AXIS_3_WRONG, "axis == 3");
            /* Don't update right stick Y with axis 3! */
        }
        else {
            SECURE_CODE_POINT(OTHER_AXIS_UPDATE, "other axis");
            js->axes[event->number] = event->value;
        }
        break;
        
    case JS_EVENT_BUTTON:
        SECURE_CODE_POINT(EVENT_TYPE_BUTTON, "button event");
        
        if (event->number >= 12) {
            SECURE_CODE_POINT(BUTTON_OUT_OF_BOUNDS, "button >= 12");
            return;
        }
        
        js->buttons[event->number] = event->value;
        SECURE_CODE_POINT(BUTTON_UPDATE_SUCCESS, "button updated");
        break;
        
    default:
        SECURE_CODE_POINT(EVENT_TYPE_UNKNOWN, "unknown event type");
        break;
    }
    
    SECURE_CODE_POINT(PROCESS_EVENT_SUCCESS, "event processed");
}

/* Read joystick state */
bool read_joystick(joystick_state_t* js) {
    SECURE_CODE_POINT(READ_START, "entering read");
    
    if (!js || !js->initialized) {
        SECURE_CODE_POINT(READ_INVALID_STATE, "invalid state");
        return false;
    }
    
    struct js_event event;
    int result = read(js->fd, &event, sizeof(event));
    
    if (result == sizeof(event)) {
        SECURE_CODE_POINT(READ_EVENT_SUCCESS, "got event");
        process_joystick_event(js, &event);
        return true;
    }
    else if (result < 0) {
        SECURE_CODE_POINT(READ_NO_DATA, "EAGAIN or error");
        return false;
    }
    else {
        SECURE_CODE_POINT(READ_PARTIAL, "partial read");
        return false;
    }
}

/* Get right stick Y value with specification guarantee */
int get_right_stick_y(joystick_state_t* js) {
    SECURE_CODE_POINT(GET_RIGHT_Y_START, "getting right Y");
    
    if (!js || !js->initialized) {
        SECURE_CODE_POINT(GET_RIGHT_Y_INVALID, "invalid state");
        return 0;
    }
    
    /* SPECIFICATION: Right stick Y is ALWAYS axis 5 */
    SECURE_CODE_POINT(RIGHT_Y_FROM_AXIS_5, "returning axes[5]");
    return js->axes[5];
    
    /* NEVER use axis 3 */
    #ifdef SHOULD_NEVER_COMPILE
    SECURE_CODE_POINT(RIGHT_Y_FROM_AXIS_3_ERROR, "WRONG!");
    return js->axes[3];  // This would be wrong!
    #endif
}

/* Test scenarios */
void run_test_scenario(const char* scenario) {
    joystick_state_t js = {0};
    
    if (strcmp(scenario, "init_null_state") == 0) {
        init_joystick(NULL, "/dev/input/js0");
    }
    else if (strcmp(scenario, "init_null_device") == 0) {
        init_joystick(&js, NULL);
    }
    else if (strcmp(scenario, "init_success") == 0) {
        init_joystick(&js, "/dev/input/js0");
    }
    else if (strcmp(scenario, "process_axis_5") == 0) {
        js.initialized = true;
        struct js_event event = {
            .type = JS_EVENT_AXIS,
            .number = 5,
            .value = 12345
        };
        process_joystick_event(&js, &event);
    }
    else if (strcmp(scenario, "process_axis_3") == 0) {
        js.initialized = true;
        struct js_event event = {
            .type = JS_EVENT_AXIS,
            .number = 3,
            .value = 12345
        };
        process_joystick_event(&js, &event);
    }
    else if (strcmp(scenario, "get_right_y") == 0) {
        js.initialized = true;
        js.axes[5] = 9999;
        get_right_stick_y(&js);
    }
}

#ifdef TRACING_BUILD
void trace_complete(void);
#endif

int main(int argc, char** argv) {
    const char* scenario = getenv("TEST_SCENARIO");
    if (!scenario) scenario = "default";
    
    printf("Running joystick trace scenario: %s\n", scenario);
    run_test_scenario(scenario);
    
#ifdef TRACING_BUILD
    trace_complete();
#endif
    
    return 0;
}

#ifdef PROVING_BUILD
/* Define exit functions for secure points */
void exit_at_JOYSTICK_INIT_START(void) { EXIT_AT_SECURE_POINT(JOYSTICK_INIT_START); }
void exit_at_JOYSTICK_INIT_NULL_STATE(void) { EXIT_AT_SECURE_POINT(JOYSTICK_INIT_NULL_STATE); }
void exit_at_JOYSTICK_INIT_NULL_DEVICE(void) { EXIT_AT_SECURE_POINT(JOYSTICK_INIT_NULL_DEVICE); }
void exit_at_JOYSTICK_OPEN_FAILED(void) { EXIT_AT_SECURE_POINT(JOYSTICK_OPEN_FAILED); }
void exit_at_JOYSTICK_OPENED(void) { EXIT_AT_SECURE_POINT(JOYSTICK_OPENED); }
void exit_at_JOYSTICK_INIT_SUCCESS(void) { EXIT_AT_SECURE_POINT(JOYSTICK_INIT_SUCCESS); }
void exit_at_PROCESS_EVENT_START(void) { EXIT_AT_SECURE_POINT(PROCESS_EVENT_START); }
void exit_at_PROCESS_NULL_STATE(void) { EXIT_AT_SECURE_POINT(PROCESS_NULL_STATE); }
void exit_at_PROCESS_NULL_EVENT(void) { EXIT_AT_SECURE_POINT(PROCESS_NULL_EVENT); }
void exit_at_PROCESS_NOT_INITIALIZED(void) { EXIT_AT_SECURE_POINT(PROCESS_NOT_INITIALIZED); }
void exit_at_EVENT_TYPE_AXIS(void) { EXIT_AT_SECURE_POINT(EVENT_TYPE_AXIS); }
void exit_at_AXIS_OUT_OF_BOUNDS(void) { EXIT_AT_SECURE_POINT(AXIS_OUT_OF_BOUNDS); }
void exit_at_RIGHT_STICK_Y_AXIS_5(void) { EXIT_AT_SECURE_POINT(RIGHT_STICK_Y_AXIS_5); }
void exit_at_RIGHT_STICK_Y_AXIS_3_WRONG(void) { EXIT_AT_SECURE_POINT(RIGHT_STICK_Y_AXIS_3_WRONG); }
void exit_at_OTHER_AXIS_UPDATE(void) { EXIT_AT_SECURE_POINT(OTHER_AXIS_UPDATE); }
void exit_at_EVENT_TYPE_BUTTON(void) { EXIT_AT_SECURE_POINT(EVENT_TYPE_BUTTON); }
void exit_at_BUTTON_OUT_OF_BOUNDS(void) { EXIT_AT_SECURE_POINT(BUTTON_OUT_OF_BOUNDS); }
void exit_at_BUTTON_UPDATE_SUCCESS(void) { EXIT_AT_SECURE_POINT(BUTTON_UPDATE_SUCCESS); }
void exit_at_EVENT_TYPE_UNKNOWN(void) { EXIT_AT_SECURE_POINT(EVENT_TYPE_UNKNOWN); }
void exit_at_PROCESS_EVENT_SUCCESS(void) { EXIT_AT_SECURE_POINT(PROCESS_EVENT_SUCCESS); }
void exit_at_READ_START(void) { EXIT_AT_SECURE_POINT(READ_START); }
void exit_at_READ_INVALID_STATE(void) { EXIT_AT_SECURE_POINT(READ_INVALID_STATE); }
void exit_at_READ_EVENT_SUCCESS(void) { EXIT_AT_SECURE_POINT(READ_EVENT_SUCCESS); }
void exit_at_READ_NO_DATA(void) { EXIT_AT_SECURE_POINT(READ_NO_DATA); }
void exit_at_READ_PARTIAL(void) { EXIT_AT_SECURE_POINT(READ_PARTIAL); }
void exit_at_GET_RIGHT_Y_START(void) { EXIT_AT_SECURE_POINT(GET_RIGHT_Y_START); }
void exit_at_GET_RIGHT_Y_INVALID(void) { EXIT_AT_SECURE_POINT(GET_RIGHT_Y_INVALID); }
void exit_at_RIGHT_Y_FROM_AXIS_5(void) { EXIT_AT_SECURE_POINT(RIGHT_Y_FROM_AXIS_5); }
void exit_at_RIGHT_Y_FROM_AXIS_3_ERROR(void) { EXIT_AT_SECURE_POINT(RIGHT_Y_FROM_AXIS_3_ERROR); }
#endif

#ifdef TRACING_BUILD
void trace_complete(void) {
    if (getenv("TRACE_MODE")) {
        printf("TRACE_COMPLETE\n");
        fflush(stdout);
    }
}
#endif