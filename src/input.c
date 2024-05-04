#include <string.h>

#include "input.h"
#include "utils.h"

// static float actions_state[INPUT_ACTION_MAX];
// static bool actions_pressed[INPUT_ACTION_MAX];
// static bool actions_released[INPUT_ACTION_MAX];

// static uint8_t expected_button[INPUT_ACTION_MAX];
// static uint8_t bindings[INPUT_LAYER_MAX][INPUT_BUTTON_MAX];

static input_capture_callback_t capture_callback;
static void *capture_user;

// static int32_t mouse_x;
// static int32_t mouse_y;

void input_init(void) {
	// input_unbind_all(INPUT_LAYER_SYSTEM);
	// input_unbind_all(INPUT_LAYER_USER);
}

void input_cleanup(void) {

}

void input_clear(void) {
	// clear(actions_pressed);
	// clear(actions_released);
}

void input_set_layer_button_state(input_layer_t layer, button_t button, float state) {
// 	error_if(layer < 0 || layer >= INPUT_LAYER_MAX, "Invalid input layer %d", layer);
// 
// 	uint8_t action = bindings[layer][button];
// 	if (action == INPUT_ACTION_NONE) {
// 		return;
// 	}
// 
// 	uint8_t expected = expected_button[action];
// 	if (!expected || expected == button) {
// 		state = (state > INPUT_DEADZONE) ? state : 0;
// 
// 		if (state && !actions_state[action]) {
// 			actions_pressed[action] = true;
// 			expected_button[action] = button;
// 		}
// 		else if (!state && actions_state[action]) {
// 			actions_released[action] = true;
// 			expected_button[action] = INPUT_BUTTON_NONE;
// 		}
// 		actions_state[action] = state;
// 	}
}

void input_capture(input_capture_callback_t cb, void *user) {
	// capture_callback = cb;
	// capture_user = user;
	// input_clear();
}

void input_textinput(int32_t ascii_char) {
	if (capture_callback) {
		capture_callback(capture_user, INPUT_INVALID, ascii_char);
	}
}

void input_bind(input_layer_t layer, button_t button, uint8_t action) {
// 	error_if(button < 0 || button >= INPUT_BUTTON_MAX, "Invalid input button %d", button);
// 	error_if(action < 0 || action >= INPUT_ACTION_MAX, "Invalid input action %d", action);
// 	error_if(layer < 0 || layer >= INPUT_LAYER_MAX, "Invalid input layer %d", layer);
// 
// 	actions_state[action] = 0;
// 	bindings[layer][button] = action;
}

float input_state(uint8_t action, PlaydateAPI* pd) {
	PDButtons current;
	PDButtons pushed;
	PDButtons released;
	pd->system->getButtonState(&current, &pushed, &released); // buttons currently pushed, pushed last frame, released last frame
	
	float result = 0.0;
	switch (action) {
		case A_UP:
		result = (current & kButtonUp) ? 1.0 : 0.0;
		break;
		case A_DOWN:
		result = (current & kButtonDown) ? 1.0 : 0.0;
		break;
		case A_LEFT:
		result = (current & kButtonLeft) ? 1.0 : 0.0;
		break;
		case A_RIGHT:
		result = (current & kButtonRight) ? 1.0 : 0.0;
		break;
		case A_BRAKE_LEFT:
		break;
		case A_BRAKE_RIGHT:
		break;
		case A_THRUST:
		result = (current & kButtonA) ? 1.0 : 0.0;
		break;
		case A_FIRE:
		break;
		case A_CHANGE_VIEW:
		break;
		default:
		break;
	}
	return result;
}

bool input_pressed(uint8_t action, PlaydateAPI *pd) {
	
	PDButtons current;
	PDButtons pushed;
	PDButtons released;
	pd->system->getButtonState(&current, &pushed, &released); // buttons currently pushed, pushed last frame, released last frame
	
	bool result = false;
	switch (action) {
		case A_MENU_UP:
			result = (pushed & kButtonUp);
			break;
		case A_MENU_DOWN:
			result = (pushed & kButtonDown);
			break;
		case A_MENU_LEFT:
			result = (pushed & kButtonLeft);
			break;
		case A_MENU_RIGHT:
			result = (pushed & kButtonRight);
			break;
		case A_CHANGE_VIEW:
		case A_MENU_BACK:
			result = (pushed & kButtonB);
			break;
		case A_MENU_SELECT:
			result = (pushed & kButtonA);
			break;
		default:
			break;
	}
	
	return result;
}
