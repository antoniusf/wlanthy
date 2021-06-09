#ifndef WLANTHY_H
#define WLANTHY_H

#include <anthy/anthy.h>
#include <anthy/input.h>
#include <iconv.h>
#include <stdbool.h>
#include <wayland-client-core.h>
#include <xkbcommon/xkbcommon.h>

#define SEGMENT_BUFSIZE 50

enum wlanthy_shift_key {
    WLANTHY_NO_SHIFT,
    WLANTHY_CROSS_SHIFT,
    WLANTHY_SAME_SHIFT
};

enum wlanthy_input_mode {
    WLANTHY_INPUT_MODE_EDIT,
    WLANTHY_INPUT_MODE_CONVERT
};

struct wlanthy_state {
	struct wl_display *display;
	struct zwp_input_method_manager_v2 *input_method_manager;
	struct zwp_virtual_keyboard_manager_v1 *virtual_keyboard_manager;

	bool running;

	struct wl_list seats;

	bool enabled_by_default;
	xkb_keysym_t toggle_key;
};

struct wlanthy_seat {
	struct wl_list link;
	struct wl_seat *wl_seat;
	struct wlanthy_state *state;

    int timerfd;

	iconv_t conv_desc;
	struct anthy_input_config *input_config;
	struct anthy_input_context *input_context;
    anthy_context_t conversion_context;
    int conversion_segment_indices[SEGMENT_BUFSIZE];
    int conversion_num_segments;
    int conversion_current_segment;
	char *preedit_buffer; // zero-terminated, bc apparently that's what everyone does here
	xkb_keycode_t current_key;
    enum wlanthy_shift_key current_shift_key;
    enum wlanthy_input_mode input_mode;
	struct zwp_input_method_v2 *input_method;
	struct zwp_virtual_keyboard_v1 *virtual_keyboard;

	struct xkb_context *xkb_context;
	struct xkb_keymap *xkb_keymap;
	struct xkb_state *xkb_state;
	char *xkb_keymap_string;

	bool active;
	bool enabled;
	uint32_t serial;
	bool pending_activate, pending_deactivate;
	struct zwp_input_method_keyboard_grab_v2 *keyboard_grab;
	xkb_keycode_t pressed[64];
};


char *
iconv_code_conv(iconv_t cd, const char *instr);
#endif
