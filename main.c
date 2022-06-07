#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <wayland-client.h>
#include "wlanthy.h"
#include "log.h"
#include "input-method-unstable-v2-client-protocol.h"
#include "virtual-keyboard-unstable-v1-client-protocol.h"

#define PREEDIT_BUFSIZE 4000
#define THUMB_KEYMAP_ROW_LENGTH 11
#define NO_KEY (XKB_KEYCODE_MAX + 1)
#define THUMB_TIMEOUT_1 100
#define THUMB_TIMEOUT_2 75

const char thumb_keys_noshift_row_d[THUMB_KEYMAP_ROW_LENGTH][4] = {
    { 0xe3, 0x80, 0x82, 0},
    { 0xe3, 0x81, 0x8b, 0},
    { 0xe3, 0x81, 0x9f, 0},
    { 0xe3, 0x81, 0x93, 0},
    { 0xe3, 0x81, 0x95, 0},
    { 0xe3, 0x82, 0x89, 0},
    { 0xe3, 0x81, 0xa1, 0},
    { 0xe3, 0x81, 0x8f, 0},
    { 0xe3, 0x81, 0xa4, 0},
    { 0x2c, 0, 0, 0},
    { 0xe3, 0x80, 0x81, 0}
};

const char thumb_keys_noshift_row_c[THUMB_KEYMAP_ROW_LENGTH][4] = {
    { 0xe3, 0x81, 0x86, 0},
    { 0xe3, 0x81, 0x97, 0},
    { 0xe3, 0x81, 0xa6, 0},
    { 0xe3, 0x81, 0x91, 0},
    { 0xe3, 0x81, 0x9b, 0},
    { 0xe3, 0x81, 0xaf, 0},
    { 0xe3, 0x81, 0xa8, 0},
    { 0xe3, 0x81, 0x8d, 0},
    { 0xe3, 0x81, 0x84, 0},
    { 0xe3, 0x82, 0x93, 0}
};

const char thumb_keys_noshift_row_b[THUMB_KEYMAP_ROW_LENGTH][4] = {

    { 0x2e, 0, 0, 0},
    { 0xe3, 0x81, 0xb2, 0},
    { 0xe3, 0x81, 0x99, 0},
    { 0xe3, 0x81, 0xb5, 0},
    { 0xe3, 0x81, 0xb8, 0},
    { 0xe3, 0x82, 0x81, 0},
    { 0xe3, 0x81, 0x9d, 0},
    { 0xe3, 0x81, 0xad, 0},
    { 0xe3, 0x81, 0xbb, 0},
    { 0xe3, 0x83, 0xbb, 0}
};

const char thumb_keys_sameshift_row_d[THUMB_KEYMAP_ROW_LENGTH][4] = {
    { 0xe3, 0x81, 0x81, 0},
    { 0xe3, 0x81, 0x88, 0},
    { 0xe3, 0x82, 0x8a, 0},
    { 0xe3, 0x82, 0x83, 0},
    { 0xe3, 0x82, 0x8c, 0},
    { 0xe3, 0x82, 0x88, 0},
    { 0xe3, 0x81, 0xab, 0},
    { 0xe3, 0x82, 0x8b, 0},
    { 0xe3, 0x81, 0xbe, 0},
    { 0xe3, 0x81, 0x87, 0}
};

const char thumb_keys_sameshift_row_c[THUMB_KEYMAP_ROW_LENGTH][4] = {
    { 0xe3, 0x82, 0x92, 0},
    { 0xe3, 0x81, 0x82, 0},
    { 0xe3, 0x81, 0xaa, 0},
    { 0xe3, 0x82, 0x85, 0},
    { 0xe3, 0x82, 0x82, 0},
    { 0xe3, 0x81, 0xbf, 0},
    { 0xe3, 0x81, 0x8a, 0},
    { 0xe3, 0x81, 0xae, 0},
    { 0xe3, 0x82, 0x87, 0},
    { 0xe3, 0x81, 0xa3, 0}
};

const char thumb_keys_sameshift_row_b[THUMB_KEYMAP_ROW_LENGTH][4] = {
    { 0xe3, 0x81, 0x85, 0},
    { 0xe3, 0x83, 0xbc, 0},
    { 0xe3, 0x82, 0x8d, 0},
    { 0xe3, 0x82, 0x84, 0},
    { 0xe3, 0x81, 0x83, 0},
    { 0xe3, 0x81, 0xac, 0},
    { 0xe3, 0x82, 0x86, 0},
    { 0xe3, 0x82, 0x80, 0},
    { 0xe3, 0x82, 0x8f, 0},
    { 0xe3, 0x81, 0x89, 0}
};

const char thumb_keys_crossshift_row_d[THUMB_KEYMAP_ROW_LENGTH][4] = {
	{ 0, 0, 0, 0},  // no cross-shift action
    { 0xe3, 0x81, 0x8c, 0},
    { 0xe3, 0x81, 0xa0, 0},
    { 0xe3, 0x81, 0x94, 0},
    { 0xe3, 0x81, 0x96, 0},
    { 0xe3, 0x81, 0xb1, 0},
    { 0xe3, 0x81, 0xa2, 0},
    { 0xe3, 0x81, 0x90, 0},
    { 0xe3, 0x81, 0xa5, 0},
    { 0xe3, 0x81, 0xb4, 0}
};

const char thumb_keys_crossshift_row_c[THUMB_KEYMAP_ROW_LENGTH][4] = {
    { 0xe3, 0x82, 0x94, 0}, // note: this is u with dakuten, which was not possible to represent with previous anthy (not in euc-jp. might make bugs?
    { 0xe3, 0x81, 0x98, 0},
    { 0xe3, 0x81, 0xa7, 0},
    { 0xe3, 0x81, 0x92, 0},
    { 0xe3, 0x81, 0x9c, 0},
    { 0xe3, 0x81, 0xb0, 0},
    { 0xe3, 0x81, 0xa9, 0},
    { 0xe3, 0x81, 0x8e, 0},
    { 0xe3, 0x81, 0xbd, 0}
};

const char thumb_keys_crossshift_row_b[THUMB_KEYMAP_ROW_LENGTH][4] = {
	{ 0, 0, 0},  // no cross-shift action
    { 0xe3, 0x81, 0xb3, 0},
    { 0xe3, 0x81, 0x9a, 0},
    { 0xe3, 0x81, 0xb6, 0},
    { 0xe3, 0x81, 0xb9, 0},
    { 0xe3, 0x81, 0xb7, 0},
    { 0xe3, 0x81, 0x9e, 0},
    { 0xe3, 0x81, 0xba, 0},
    { 0xe3, 0x81, 0xbc, 0}
};

// timeout is given in milliseconds
void start_timer(struct wlanthy_seat *seat, long timeout) {
    struct itimerspec timer_info = {
        .it_interval = { 0, 0 },
        .it_value = { 0, timeout * 1000 * 1000 } // 0 seconds, timeout * (1e6 ns == 1 ms)
    };

    timerfd_settime(seat->timerfd, 0, &timer_info, NULL);

    log_line(LV_DEBUG, "timer has been set");
}

void stop_timer(struct wlanthy_seat *seat) {
    struct itimerspec timer_info = {
        .it_interval = { 0, 0 },
        .it_value = { 0, 0 } // set everything to 0, stop the timer
    };

    timerfd_settime(seat->timerfd, 0, &timer_info, NULL);
    log_line(LV_DEBUG, "timer has been reset");
}

// commit: if true, send the preedit buffer as commit text. then, clear it.
void send_preedit_buffer(struct wlanthy_seat *seat, bool commit) {

    //char *utf8_str = iconv_code_conv(seat->conv_desc, seat->im_state.preedit_buffer);
    char *utf8_str = seat->im_state.preedit_buffer;
	log_line(LV_DEBUG, "%s", utf8_str);

	if (commit) {
		zwp_input_method_v2_commit_string(seat->input_method, utf8_str);

		seat->im_state.preedit_buffer[0] = 0;
        seat->im_state.input_mode = WLANTHY_INPUT_MODE_EDIT;
        seat->im_state.preedit_cursor_start = 0;
        seat->im_state.preedit_cursor_end = 0;
	}
	else {
        log_line(LV_DEBUG, "cursor start: %i, cursor end: %i", seat->im_state.preedit_cursor_start, seat->im_state.preedit_cursor_end);
		zwp_input_method_v2_set_preedit_string(seat->input_method,
	utf8_str, seat->im_state.preedit_cursor_start, seat->im_state.preedit_cursor_end); // todo: the 0, 0 is a cursor position, make that better
	}
	zwp_input_method_v2_commit(seat->input_method, seat->serial);

	//free(utf8_str);
}

void update_preedit_buffer_conversion(struct wlanthy_im_state *im_state) {
    if (im_state->input_mode != WLANTHY_INPUT_MODE_CONVERT) {
        return;
    }

    struct anthy_segment_stat segment_stat;

    size_t buffer_write_pos = 0;

    for (int i = 0; i < im_state->conversion_num_segments; i++) {
        anthy_get_segment_stat(im_state->conversion_context, i, &segment_stat);
        int segment_index = im_state->conversion_segment_indices[i];
        int chars_written = anthy_get_segment(im_state->conversion_context, i, segment_index, im_state->preedit_buffer + buffer_write_pos, PREEDIT_BUFSIZE - buffer_write_pos);
        if (chars_written >= 0) {
            buffer_write_pos += chars_written;
        }
        else {
            log_line(LV_DEBUG, "anthy returned a conversion error");
        }

        if (i == im_state->conversion_current_segment) {
            im_state->preedit_cursor_start = buffer_write_pos - chars_written;
            im_state->preedit_cursor_end = buffer_write_pos;
        }
    }
}

void start_conversion(struct wlanthy_im_state *im_state) {
    if (im_state->input_mode == WLANTHY_INPUT_MODE_CONVERT) {
        log_line(LV_DEBUG, "already in convert mode, refusing to start conversion");
        return;
    }

    if (strlen(im_state->preedit_buffer) == 0) {
        log_line(LV_DEBUG, "preedit buffer empty, refusing to start conversion");
        return;
    }

    im_state->conversion_context = anthy_create_context();
    anthy_context_set_encoding(im_state->conversion_context, ANTHY_UTF8_ENCODING);
    anthy_set_string(im_state->conversion_context, im_state->preedit_buffer);
    struct anthy_conv_stat conv_stat;
    anthy_get_stat(im_state->conversion_context, &conv_stat);
    im_state->conversion_num_segments = conv_stat.nr_segment;
    im_state->conversion_current_segment = 0;

    for (int i = 0; i < im_state->conversion_num_segments; i++) {
        im_state->conversion_segment_indices[i] = 0;
    }

    im_state->input_mode = WLANTHY_INPUT_MODE_CONVERT;

    update_preedit_buffer_conversion(im_state);

    log_line(LV_DEBUG, "starting conversion");
}

void stop_conversion_no_commit(struct wlanthy_im_state *im_state) {
    if (im_state->input_mode != WLANTHY_INPUT_MODE_CONVERT) {
        return;
    }

    log_line(LV_DEBUG, "stopping conversion");

    for (int i = 0; i < im_state->conversion_num_segments; i++) {
        im_state->conversion_segment_indices[i] = NTH_HIRAGANA_CANDIDATE;
    }
    update_preedit_buffer_conversion(im_state);

    anthy_release_context(im_state->conversion_context);
    im_state->conversion_context = NULL;
    im_state->input_mode = WLANTHY_INPUT_MODE_EDIT;
    im_state->preedit_cursor_start = 0;
    im_state->preedit_cursor_end = 0;
}

void conversion_change_candidate(struct wlanthy_im_state *im_state, int next) {
    if (im_state->input_mode != WLANTHY_INPUT_MODE_CONVERT) {
        log_line(LV_DEBUG, "not in conversion mode: can't select next candidate");
        return;
    }

    int delta = next ? 1 : -1;

    struct anthy_segment_stat segment_stat;

    anthy_get_segment_stat(
        im_state->conversion_context,
        im_state->conversion_current_segment,
        &segment_stat);

    int *current_segment_index =
        &im_state->conversion_segment_indices[im_state->conversion_current_segment];

    *current_segment_index =
        (*current_segment_index + delta + segment_stat.nr_candidate) % segment_stat.nr_candidate;

    log_line(LV_DEBUG, "selecting candidate number %i for segment %i", *current_segment_index, im_state->conversion_current_segment);

    update_preedit_buffer_conversion(im_state);
}

void write_key(struct wlanthy_seat *seat) {

    log_line(LV_DEBUG, "writing key!");

    if (seat->im_state.current_key != XKB_KEYCODE_MAX + 1) {
        enum wlanthy_shift_key shift_key = seat->im_state.current_shift_key;

        log_line(LV_DEBUG, "%d", seat->im_state.current_key);
        log_line(LV_DEBUG, "%d", NO_KEY);
        const char *keycode_name = xkb_keymap_key_get_name(seat->xkb_keymap, seat->im_state.current_key);
        size_t column = strtol(&keycode_name[2], NULL, 10);
        column -= 1;

        if (column >= THUMB_KEYMAP_ROW_LENGTH) {
            return;
        }

        char row = keycode_name[1];
        const char *character;

        if (row == 'B') {
            if (shift_key == WLANTHY_SAME_SHIFT) {
                character = thumb_keys_sameshift_row_b[column];
            } else if (shift_key == WLANTHY_CROSS_SHIFT) {
                character = thumb_keys_crossshift_row_b[column];
            } else {
                character = thumb_keys_noshift_row_b[column];
            }
        } else if (row == 'C') {
            if (shift_key == WLANTHY_SAME_SHIFT) {
                character = thumb_keys_sameshift_row_c[column];
            } else if (shift_key == WLANTHY_CROSS_SHIFT) {
                character = thumb_keys_crossshift_row_c[column];
            } else {
                character = thumb_keys_noshift_row_c[column];
            }
        } else if (row == 'D') {
            if (shift_key == WLANTHY_SAME_SHIFT) {
                character = thumb_keys_sameshift_row_d[column];
            } else if (shift_key == WLANTHY_CROSS_SHIFT) {
                character = thumb_keys_crossshift_row_d[column];
            } else {
                character = thumb_keys_noshift_row_d[column];
            }
        } else {
            return;
        }

        strcat(seat->im_state.preedit_buffer, character);
    }

    else if (seat->im_state.current_shift_key != WLANTHY_NO_SHIFT) {
        log_line(LV_DEBUG, "shift key action!");

        if (seat->im_state.current_shift_key == WLANTHY_SAME_SHIFT) {
            // i'm treating this as henkan
            if (seat->im_state.input_mode == WLANTHY_INPUT_MODE_EDIT) {
                start_conversion(&seat->im_state);
            }
            else {
                if (xkb_state_mod_name_is_active(
                        seat->xkb_state,
                        XKB_MOD_NAME_SHIFT,
                        XKB_STATE_MODS_EFFECTIVE) > 0) {

                    conversion_change_candidate(&seat->im_state, 0);
                }
                else {
                    conversion_change_candidate(&seat->im_state, 1);
                }
            }
        }

    }

    // TODO: handle space (henkan/muhenkan)

    seat->im_state.current_key = XKB_KEYCODE_MAX + 1;
    seat->im_state.current_shift_key = WLANTHY_NO_SHIFT;
    //stop_timer();
    send_preedit_buffer(seat, false);
}

/*
 * Returns false if the key needs to be passed through
 */
static bool handle_key_anthy(struct wlanthy_seat *seat,
							 xkb_keycode_t xkb_key,
							 uint32_t key_state) {
	int state = anthy_input_get_state(seat->input_context);
	int map = anthy_input_get_selected_map(seat->input_context);

	xkb_keysym_t sym = xkb_state_key_get_one_sym(seat->xkb_state, xkb_key);

	bool do_commit = false;

	if (sym == seat->state->toggle_key) {
		seat->enabled = !seat->enabled;
		if (!seat->enabled) {
			// reset state
			anthy_input_free_context(seat->input_context);
			seat->input_context = anthy_input_create_context(seat->input_config);
			zwp_input_method_v2_set_preedit_string(seat->input_method, "", 0, 0);
			zwp_input_method_v2_commit(seat->input_method, seat->serial);
		}
		return true;
	} else if (!seat->enabled) {
		return false;
	} else {
		const char *keycode_name = xkb_keymap_key_get_name(seat->xkb_keymap, xkb_key);
		log_line(LV_DEBUG, "%s", keycode_name);

		if (strlen(keycode_name) == 4
                && (keycode_name[0] == 'A')
				&& (keycode_name[1] >= 'B')
				&& (keycode_name[1] <= 'D')
				&& (keycode_name[2] >= '0')
				&& (keycode_name[2] <= '9')
				&& (keycode_name[3] >= '0')
				&& (keycode_name[3] <= '9')
			) {

			if (key_state == WL_KEYBOARD_KEY_STATE_PRESSED) {
				log_line(LV_DEBUG, "key pressed: %s", keycode_name);
				if (seat->im_state.current_key != XKB_KEYCODE_MAX + 1) {
					log_line(LV_DEBUG, "... replacing current key");
                    // since this is replacing the currently pressed key,
                    // we write that one immediately
                    write_key(seat);
                    seat->im_state.current_key = xkb_key; 
                    start_timer(seat, THUMB_TIMEOUT_2); // this is t2 in ibus-anthy (i think), i.e. the shorter of the two
				}
                else if (seat->im_state.current_shift_key != WLANTHY_NO_SHIFT) {
                    // a space key is already pressed, so we can write
                    // immediately. note that this is mutually exclusive
                    // with the first case, since whenever a key and a space
                    // key are pressed simultaneously, they are written
                    // immediately and then reset.
                    seat->im_state.current_key = xkb_key; 
                    write_key(seat);
                }
                else {
                    seat->im_state.current_key = xkb_key; 
                    start_timer(seat, THUMB_TIMEOUT_2);
                }
			} else if (key_state == WL_KEYBOARD_KEY_STATE_RELEASED) {
				if (xkb_key == seat->im_state.current_key) {
					log_line(LV_DEBUG, "current key released: %s", keycode_name);

                    write_key(seat);
				}
			}
		}

		else if (strcmp(keycode_name, "SPCE") == 0) {
			if (key_state == WL_KEYBOARD_KEY_STATE_RELEASED) {
                if (seat->im_state.current_shift_key == WLANTHY_SAME_SHIFT) {
                    write_key(seat);
                }
			}
			else {
                // TODO: handle shift key replace
                // (press other shift key, this becomes the new shift key
                // and commits the action of the old one)
                seat->im_state.current_shift_key = WLANTHY_SAME_SHIFT;
                if (seat->im_state.current_key != NO_KEY) {
                    write_key(seat);
                }
                else {
                    seat->im_state.current_shift_key = WLANTHY_SAME_SHIFT;
                    start_timer(seat, THUMB_TIMEOUT_1); // this is t1 in ibus-anthy, i.e. the longer of the two
                }
			}
		}

		else if ((strcmp(keycode_name, "LALT") == 0) || (strcmp(keycode_name, "RALT") == 0)) {
			if (key_state == WL_KEYBOARD_KEY_STATE_RELEASED) {
                if (seat->im_state.current_shift_key == WLANTHY_CROSS_SHIFT) {
                    seat->im_state.current_shift_key = WLANTHY_NO_SHIFT;
                }
			}
			else {
                seat->im_state.current_shift_key = WLANTHY_CROSS_SHIFT;
                if (seat->im_state.current_key != NO_KEY) {
                    write_key(seat);
                }
                else {
                    start_timer(seat, THUMB_TIMEOUT_1); // this is t1 in ibus-anthy (i think)
                }
			}
		}

		else if (strcmp(keycode_name, "RTRN") == 0) {
			if (strlen(seat->im_state.preedit_buffer) == 0) {
				return false;
			}
            send_preedit_buffer(seat, true);
		}

        else if (strcmp(keycode_name, "BKSP") == 0) {
            if (key_state == WL_KEYBOARD_KEY_STATE_PRESSED) {

                if (seat->im_state.input_mode == WLANTHY_INPUT_MODE_EDIT) {
                    size_t preedit_buffer_len = strlen(seat->im_state.preedit_buffer);

                    log_line(LV_DEBUG, "deleting; preedit buffer length in bytes: %d", preedit_buffer_len);

                    if (preedit_buffer_len >= 1) {
                        size_t pos = preedit_buffer_len - 1;

                        while ((pos--) > 0) {
                            if (((unsigned char) seat->im_state.preedit_buffer[pos] >> 6) != 2) {
                                seat->im_state.preedit_buffer[pos] = 0;
                                break;
                            }
                        }

                        log_line(LV_DEBUG, "deleted; preedit buffer length in bytes: %d", strlen(seat->im_state.preedit_buffer));
                    }
                    else {
                        return false;
                    }
                } 
                else if (seat->im_state.input_mode == WLANTHY_INPUT_MODE_CONVERT) {
                    stop_conversion_no_commit(&seat->im_state);
                }

                send_preedit_buffer(seat, false);
			}
            else {
                return false;
            }
		}

        else {
            return false;
        }

/*		else {

			size_t index;
			switch (sym) {
			case XKB_KEY_a ... XKB_KEY_z:
				sym &= ~(1 << 5);
			case XKB_KEY_A ... XKB_KEY_Z:
				index = sym - 0x41;
				//strcat(seat->im_state.preedit_buffer, thumb_keys_leftshift[index]);
				break;
			//case XKB_KEY_exclam ... XKB_KEY_asciitilde:;
			//	uint32_t ch = xkb_state_key_get_utf32(seat->xkb_state, xkb_key);
			//	anthy_input_key(seat->input_context, ch);
			//	break;
			case XKB_KEY_space:
				anthy_input_space(seat->input_context);
				break;
			case XKB_KEY_BackSpace:
				if (state != ANTHY_INPUT_ST_NONE) {
					// TODO: send this repeatedly until key release
					anthy_input_erase_prev(seat->input_context);
				} else
					return false;
				break;
			case XKB_KEY_Tab:
				if (state != ANTHY_INPUT_ST_NONE) {
					if (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_ALT,
													 XKB_STATE_MODS_EFFECTIVE) > 0)
						anthy_input_resize(seat->input_context, 1);
					else
					anthy_input_move(seat->input_context, 1);
				} else
					return false;
				break;
			case XKB_KEY_ISO_Left_Tab:
				if (state != ANTHY_INPUT_ST_NONE) {
					if (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_ALT,
													 XKB_STATE_MODS_EFFECTIVE) > 0)
						anthy_input_resize(seat->input_context, -1);
					else
					anthy_input_move(seat->input_context, -1);
				} else
					return false;
				break;
			case XKB_KEY_Return:
				if (state != ANTHY_INPUT_ST_NONE) {
					anthy_input_commit(seat->input_context);
				} else
					return false;
				break;
			case XKB_KEY_Alt_L:
				return true;
			case XKB_KEY_F5:
				anthy_input_map_select(seat->input_context, ANTHY_INPUT_MAP_HIRAGANA);
				break;
			case XKB_KEY_F6:
				anthy_input_map_select(seat->input_context, ANTHY_INPUT_MAP_KATAKANA);
				break;
			case XKB_KEY_F7:
				anthy_input_map_select(seat->input_context, ANTHY_INPUT_MAP_ALPHABET);
				break;
			case XKB_KEY_F8:
				anthy_input_map_select(seat->input_context, ANTHY_INPUT_MAP_WALPHABET);
				break;
			default:
				return false;
			}
		} */
	}
	/*
	 * At this point the key has been handled by anthy
	 */
	char name[64];
	xkb_keysym_get_name(sym, name, 64);
	log_line(LV_DEBUG, "state: %d, map: %d", state, map);
	log_line(LV_DEBUG, "pressed %s", name);
/*	anthy_context_t ac;
	if ((ac = anthy_input_get_anthy_context(seat->input_context)))
		anthy_print_context(ac);*/

	struct anthy_input_preedit *pe = anthy_input_get_preedit(seat->input_context);

	//if (pe->commit) {
	//	//char *commit_str = iconv_code_conv(seat->conv_desc, pe->commit);
    //    char *commit_str = pe->commit;
	//	log_line(LV_DEBUG, "%s", commit_str);
	//	zwp_input_method_v2_commit_string(seat->input_method, commit_str);
	//	//free(commit_str);
	//zwp_input_method_v2_commit(seat->input_method, seat->serial);
	//return true;
	//}

	//char preedit_str[PREEDIT_BUFSIZE]; preedit_str[0] = '\0';
	//int totlen = 0;
	//int begin = 0;
	//int end = 0;
	//log_head(LV_DEBUG);
	//log_body(LV_DEBUG, "|");
	//for (struct anthy_input_segment* cur = pe->segment; cur != NULL &&
	//	 cur->str != NULL; cur = cur->next) {
	//	//char *utf8_str = iconv_code_conv(seat->conv_desc, cur->str);
    //    char *utf8_str = cur->str;
	//	size_t len = strlen(utf8_str);
	//	if (cur == pe->cur_segment) {
	//		begin = totlen;
	//		end = totlen+len;
	//		log_body(LV_DEBUG, "*");
	//	}
	//	totlen += len;
	//	log_body(LV_DEBUG, "%s|", utf8_str);
	//	if (PREEDIT_BUFSIZE-totlen-1 > 0)
	//		strcat(preedit_str, utf8_str);
	//	//free(utf8_str);
	//}
	//log_tail(LV_DEBUG);
	//anthy_input_free_preedit(pe);
	return true;
}

uint32_t last = 0; // HACK?

static void handle_key(void *data,
		struct zwp_input_method_keyboard_grab_v2 *keyboard_grab,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	struct wlanthy_seat *seat = data;
	xkb_keycode_t xkb_key = key + 8;
	last = key; // HACK?

	if (seat->xkb_state == NULL) {
		return;
	}

	bool handled;
	if (xkb_state_mod_names_are_active(seat->xkb_state,
XKB_STATE_MODS_EFFECTIVE, XKB_STATE_MATCH_ANY, XKB_MOD_NAME_CTRL,
XKB_MOD_NAME_LOGO, XKB_MOD_NAME_CAPS,
// TODO: investigate EFFECTIVE vs others
// TODO: investigate XKB_LED_NAME_CAPS, XKB_LED_NAME_NUM, XKB_LED_NAME_SCROLL
NULL) > 0) {
	/*
	 * Passthrough key if any modifier is active
	 */
		handled = false;
	}
	else {
		handled = handle_key_anthy(seat, xkb_key, state);
	}

	// we are sending too many release here... bring back wlhangul stuff
	if (!handled) {
		zwp_virtual_keyboard_v1_key(seat->virtual_keyboard, time, key, state);
	}
}

static void handle_modifiers(void *data,
		struct zwp_input_method_keyboard_grab_v2 *keyboard_grab,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
	struct wlanthy_seat *seat = data;

	if (seat->xkb_state == NULL) {
		return;
	}

	xkb_state_update_mask(seat->xkb_state, mods_depressed,
		mods_latched, mods_locked, 0, 0, group);
	zwp_virtual_keyboard_v1_modifiers(seat->virtual_keyboard,
		mods_depressed, mods_latched, mods_locked, group);
}

static void handle_keymap(void *data,
		struct zwp_input_method_keyboard_grab_v2 *keyboard_grab,
		uint32_t format, int32_t fd, uint32_t size) {
	struct wlanthy_seat *seat = data;

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
		close(fd);
		return;
	}

	char *str = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (str == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return;
	}

	/*
	 * This is currently needed to avoid the keymap loop bug in sway
	 */
	static bool first_call = true;
	if (!first_call)
    		return;
	first_call = false;

	zwp_virtual_keyboard_v1_keymap(seat->virtual_keyboard, format, fd,
								   size);
	seat->xkb_keymap_string = strdup(str);

	if (seat->xkb_keymap != NULL) {
		xkb_keymap_unref(seat->xkb_keymap);
	}
	seat->xkb_keymap = xkb_keymap_new_from_string(seat->xkb_context, str,
		XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(str, size);
	close(fd);

	if (seat->xkb_keymap == NULL) {
		fprintf(stderr, "Failed to compile keymap\n");
		return;
	}

	if (seat->xkb_state != NULL) {
		xkb_state_unref(seat->xkb_state);
	}
	seat->xkb_state = xkb_state_new(seat->xkb_keymap);
	if (seat->xkb_state == NULL) {
		fprintf(stderr, "Failed to create XKB state\n");
		return;
	}
}

static void handle_repeat_info(void *data,
		struct zwp_input_method_keyboard_grab_v2 *keyboard_grab, int32_t rate,
		int32_t delay) {
	// TODO
}

static const struct zwp_input_method_keyboard_grab_v2_listener
		keyboard_grab_listener = {
	.key = handle_key,
	.modifiers = handle_modifiers,
	.keymap = handle_keymap,
	.repeat_info = handle_repeat_info,
};

static void handle_activate(void *data,
		struct zwp_input_method_v2 *input_method) {
	struct wlanthy_seat *seat = data;
	seat->pending_activate = true;
}

static void handle_deactivate(void *data,
		struct zwp_input_method_v2 *input_method) {
	struct wlanthy_seat *seat = data;
	seat->pending_deactivate = true;
}

static void handle_surrounding_text(void *data,
		struct zwp_input_method_v2 *input_method, const char *text,
		uint32_t cursor, uint32_t anchor) {
}

static void handle_text_change_cause(void *data,
		struct zwp_input_method_v2 *input_method, uint32_t cause) {
}

static void handle_content_type(void *data,
		struct zwp_input_method_v2 *input_method, uint32_t hint,
		uint32_t purpose) {
}

static void handle_done(void *data, struct zwp_input_method_v2 *input_method) {
	struct wlanthy_seat *seat = data;
	seat->serial++;

	if (seat->pending_activate && !seat->active) {
		seat->keyboard_grab = zwp_input_method_v2_grab_keyboard(input_method);
		zwp_input_method_keyboard_grab_v2_add_listener(seat->keyboard_grab,
			&keyboard_grab_listener, seat);
		seat->active = true;
	} else if (seat->pending_deactivate && seat->active) {
		zwp_input_method_keyboard_grab_v2_release(seat->keyboard_grab);

		// reset state
		anthy_input_free_context(seat->input_context);
		seat->input_context = anthy_input_create_context(seat->input_config);
		seat->im_state.preedit_buffer[0] = 0;
		seat->im_state.current_key = NO_KEY;
        seat->im_state.current_shift_key = WLANTHY_NO_SHIFT;

		memset(seat->pressed, 0, sizeof(seat->pressed));

		// wlhangul doesn't need this... why?
		if (seat->xkb_keymap != NULL)
			zwp_virtual_keyboard_v1_key(seat->virtual_keyboard, 0, last,
										WL_KEYBOARD_KEY_STATE_RELEASED);
		seat->keyboard_grab = NULL;
		seat->active = false;
	}

	seat->pending_activate = false;
	seat->pending_deactivate = false;
}

static void handle_unavailable(void *data,
		struct zwp_input_method_v2 *input_method) {
}

static const struct zwp_input_method_v2_listener input_method_listener = {
	.activate = handle_activate,
	.deactivate = handle_deactivate,
	.surrounding_text = handle_surrounding_text,
	.text_change_cause = handle_text_change_cause,
	.content_type = handle_content_type,
	.done = handle_done,
	.unavailable = handle_unavailable,
};

static struct wlanthy_seat *create_seat(struct wlanthy_state *state,
		struct wl_seat *wl_seat) {
	struct wlanthy_seat *seat = calloc(1, sizeof(*seat));
	seat->wl_seat = wl_seat;
	seat->state = state;
	seat->xkb_keymap_string = "";
	wl_list_insert(&state->seats, &seat->link);
	return seat;
}

static void registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	struct wlanthy_state *state = data;
	if (strcmp(interface, wl_seat_interface.name) == 0) {
		struct wl_seat *seat =
			wl_registry_bind(registry, name, &wl_seat_interface, 1);
		create_seat(state, seat);
	} else if (strcmp(interface, zwp_input_method_manager_v2_interface.name) == 0) {
		state->input_method_manager = wl_registry_bind(registry, name,
			&zwp_input_method_manager_v2_interface, 1);
	} else if (strcmp(interface, zwp_virtual_keyboard_manager_v1_interface.name) == 0) {
		state->virtual_keyboard_manager = wl_registry_bind(registry, name,
			&zwp_virtual_keyboard_manager_v1_interface, 1);
	}
}

static void registry_handle_global_remove(void *data,
		struct wl_registry *registry, uint32_t name) {
	// TODO
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

static const char usage[] = "usage: wlanthy [options...]\n"
	"\n"
	"    -d                 Show debug messages\n"
	"    -i anthy|pass      Initial input mode (default: anthy)\n"
	"    -k <key>           Key to toggle mode (default: F12)\n";

int main(int argc, char *argv[]) {
	struct wlanthy_state state = {0};
	state.enabled_by_default = true;
	state.toggle_key = XKB_KEY_F12;
	wl_list_init(&state.seats);

	int opt;
	while ((opt = getopt(argc, argv, "hdi:k:")) != -1) {
		switch (opt) {
		case 'd':
			log_set_loglevel(LV_DEBUG);
			break;
		case 'i':
			if (strcmp(optarg, "anthy") == 0) {
				state.enabled_by_default = true;
			} else if (strcmp(optarg, "pass") == 0) {
				state.enabled_by_default = false;
			} else {
				fprintf(stderr, "Invalid value for -i\n");
				return 1;
			}
			break;
		case 'k':;
			state.toggle_key =
				xkb_keysym_from_name(optarg, XKB_KEYSYM_NO_FLAGS);
			if (state.toggle_key == XKB_KEY_NoSymbol) {
				fprintf(stderr, "Invalid key for -k\n");
				return 1;
			}
			break;
		default:
			fprintf(stderr, "%s", usage);
			return 1;
		}
	}

	state.display = wl_display_connect(NULL);
	if (state.display == NULL) {
		fprintf(stderr, "failed to connect to Wayland display\n");
		return 1;
	}

	struct wl_registry *registry = wl_display_get_registry(state.display);
	wl_registry_add_listener(registry, &registry_listener, &state);
	wl_display_roundtrip(state.display);

	if (state.input_method_manager == NULL) {
		fprintf(stderr, "missing wl_seat or zwp_input_method_manager_v2\n");
		return 1;
	}

    struct pollfd *pollfds = malloc(sizeof(struct pollfd));
    pollfds[0].fd = wl_display_get_fd(state.display);
    pollfds[0].events = POLLIN;
    pollfds[0].revents = 0;
    size_t num_pollfds = 1;

	anthy_input_init();

	struct wlanthy_seat *seat;
	wl_list_for_each(seat, &state.seats, link) {
		seat->conv_desc = iconv_open("UTF-8", "EUC-JP"); // should be unique...
		seat->input_config = anthy_input_create_config();
		seat->input_context = anthy_input_create_context(seat->input_config);
		seat->input_method = zwp_input_method_manager_v2_get_input_method(
			state.input_method_manager, seat->wl_seat);
		zwp_input_method_v2_add_listener(seat->input_method,
			&input_method_listener, seat);
		seat->virtual_keyboard =
			zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(
			state.virtual_keyboard_manager, seat->wl_seat);
		seat->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		seat->enabled = state.enabled_by_default;
        seat->timerfd = timerfd_create(CLOCK_REALTIME, 0);

        seat->im_state.input_mode = WLANTHY_INPUT_MODE_EDIT;
		seat->im_state.preedit_buffer = malloc(PREEDIT_BUFSIZE);
		seat->im_state.preedit_buffer[0] = 0;
        seat->im_state.preedit_cursor_start = 0;
        seat->im_state.preedit_cursor_end = 0;
		seat->im_state.current_key = NO_KEY;
        seat->im_state.current_shift_key = WLANTHY_NO_SHIFT;

        num_pollfds += 1;
        pollfds = realloc(pollfds, num_pollfds * sizeof(struct pollfd));
        pollfds[num_pollfds - 1].fd = seat->timerfd;
        pollfds[num_pollfds - 1].events = POLLIN;
        pollfds[num_pollfds - 1].revents = 0;
	}

    log_line(LV_DEBUG, "%d", num_pollfds);

	state.running = true;
	while (true) {

        // this first one probably isn't necessary, but I'm doing
        // it anyway to be clean
        wl_display_dispatch_pending(state.display);
        wl_display_flush(state.display);
        poll(pollfds, num_pollfds, -1);

        // todo: what about error states etc
        if (pollfds[0].revents & POLLIN) {

            if (wl_display_dispatch(state.display) == -1) {
                break;
            }
        }

        // iterate over each seat's fd
        size_t i = 1;
        wl_list_for_each(seat, &state.seats, link) {
            if (pollfds[i].revents & POLLIN) {
                // timer has expired, do something

                // also, clear the timer (we might not have to do this though,
                // since it'll be reset anyway the next time someone calls settime?
                uint64_t times_expired;
                read(seat->timerfd, &times_expired, sizeof(times_expired));

                log_line(LV_DEBUG, "timer has expired");
                write_key(seat);
            }
            i += 1;
        }

        if (!state.running) {
            break;
        }
	}

//	finalize (seat->conv_desc);
	return 0;
}
