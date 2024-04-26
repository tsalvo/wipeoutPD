// #include <SDL2/SDL.h>

#include "platform.h"
#include "utils.h"
#include "mem.h"

static uint64_t perf_freq = 0;
static void (*audio_callback)(float *buffer, uint32_t len) = NULL;
static char *path_assets = "";
static char *path_userdata = "";
static char *temp_path = NULL;

void platform_pump_events(void) {
// 	SDL_Event ev;
// 	while (SDL_PollEvent(&ev)) {
// 		// Detect ALT+Enter press to toggle fullscreen
// 		if (
// 			ev.type == SDL_KEYDOWN && 
// 			ev.key.keysym.scancode == SDL_SCANCODE_RETURN &&
// 			(ev.key.keysym.mod & (KMOD_LALT | KMOD_RALT))
// 		) {
// 			platform_set_fullscreen(!platform_get_fullscreen());
// 		}
// 
// 		// Input Keyboard
// 		else if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
// 			int code = ev.key.keysym.scancode;
// 			float state = ev.type == SDL_KEYDOWN ? 1.0 : 0.0;
// 			if (code >= SDL_SCANCODE_LCTRL && code <= SDL_SCANCODE_RALT) {
// 				int code_internal = code - SDL_SCANCODE_LCTRL + INPUT_KEY_LCTRL;
// 				input_set_button_state(code_internal, state);
// 			}
// 			else if (code > 0 && code < INPUT_KEY_MAX) {
// 				input_set_button_state(code, state);
// 			}
// 		}
// 
// 		else if (ev.type == SDL_TEXTINPUT) {
// 			input_textinput(ev.text.text[0]);
// 		}
// 
// 		// Gamepads connect/disconnect
// 		else if (ev.type == SDL_CONTROLLERDEVICEADDED) {
// 			gamepad = SDL_GameControllerOpen(ev.cdevice.which);
// 		}
// 		else if (ev.type == SDL_CONTROLLERDEVICEREMOVED) {
// 			if (gamepad && ev.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad))) {
// 				SDL_GameControllerClose(gamepad);
// 				gamepad = platform_find_gamepad();
// 			}
// 		}
// 
// 		// Input Gamepad Buttons
// 		else if (
// 			ev.type == SDL_CONTROLLERBUTTONDOWN || 
// 			ev.type == SDL_CONTROLLERBUTTONUP
// 		) {
// 			if (ev.cbutton.button < SDL_CONTROLLER_BUTTON_MAX) {
// 				button_t button = platform_sdl_gamepad_map[ev.cbutton.button];
// 				if (button != INPUT_INVALID) {
// 					float state = ev.type == SDL_CONTROLLERBUTTONDOWN ? 1.0 : 0.0;
// 					input_set_button_state(button, state);
// 				}
// 			}
// 		}
// 
// 		// Input Gamepad Axis
// 		else if (ev.type == SDL_CONTROLLERAXISMOTION) {
// 			float state = (float)ev.caxis.value / 32767.0;
// 
// 			if (ev.caxis.axis < SDL_CONTROLLER_AXIS_MAX) {
// 				int code = platform_sdl_axis_map[ev.caxis.axis];
// 				if (
// 					code == INPUT_GAMEPAD_L_TRIGGER || 
// 					code == INPUT_GAMEPAD_R_TRIGGER
// 				) {
// 					input_set_button_state(code, state);
// 				}
// 				else if (state > 0) {
// 					input_set_button_state(code, 0.0);
// 					input_set_button_state(code+1, state);
// 				}
// 				else {
// 					input_set_button_state(code, -state);
// 					input_set_button_state(code+1, 0.0);
// 				}
// 			}
// 		}
// 
// 		// Mouse buttons
// 		else if (
// 			ev.type == SDL_MOUSEBUTTONDOWN ||
// 			ev.type == SDL_MOUSEBUTTONUP
// 		) {
// 			button_t button = INPUT_BUTTON_NONE;
// 			switch (ev.button.button) {
// 				case SDL_BUTTON_LEFT: button = INPUT_MOUSE_LEFT; break;
// 				case SDL_BUTTON_MIDDLE: button = INPUT_MOUSE_MIDDLE; break;
// 				case SDL_BUTTON_RIGHT: button = INPUT_MOUSE_RIGHT; break;
// 				default: break;
// 			}
// 			if (button != INPUT_BUTTON_NONE) {
// 				float state = ev.type == SDL_MOUSEBUTTONDOWN ? 1.0 : 0.0;
// 				input_set_button_state(button, state);
// 			}
// 		}
// 
// 		// Mouse wheel
// 		else if (ev.type == SDL_MOUSEWHEEL) {
// 			button_t button = ev.wheel.y > 0 
// 				? INPUT_MOUSE_WHEEL_UP
// 				: INPUT_MOUSE_WHEEL_DOWN;
// 			input_set_button_state(button, 1.0);
// 			input_set_button_state(button, 0.0);
// 		}
// 
// 		// Mouse move
// 		else if (ev.type == SDL_MOUSEMOTION) {
// 			input_set_mouse_pos(ev.motion.x, ev.motion.y);
// 		}
// 
// 		// Window Events
// 		if (ev.type == SDL_QUIT) {
// 			wants_to_exit = true;
// 		}
// 		else if (
// 			ev.type == SDL_WINDOWEVENT &&
// 			(
// 				ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
// 				ev.window.event == SDL_WINDOWEVENT_RESIZED
// 			)
// 		) {
// 			system_resize(platform_screen_size());
// 		}
// 	}
}

void platform_audio_callback(void* userdata, uint8_t* stream, int len) {
	// if (audio_callback) {
	// 	audio_callback((float *)stream, len/sizeof(float));
	// }
	// else {
	// 	memset(stream, 0, len);
	// }
}

void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len)) {
	// audio_callback = cb;
	// SDL_PauseAudioDevice(audio_device, 0);
}

uint8_t *platform_load_asset(const char *name, uint32_t *bytes_read, PlaydateAPI *pd) {
	return file_load(name, bytes_read, pd);
}

uint8_t *platform_load_userdata(const char *name, uint32_t *bytes_read, PlaydateAPI *pd) {
	return file_load(name, bytes_read, pd);
}

