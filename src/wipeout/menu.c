#include "../system.h"
#include "../input.h"
#include "../utils.h"

#include "game.h"
#include "menu.h"
#include "ui.h"
#include "sfx.h"

bool blink(void) {
	// blink 30 times per second
	return fmodf(system_cycle_time(), 1.0F/15.0F) < 1.0F/30.0F;
}

void menu_reset(menu_t *menu) {
	menu->index = -1;
}

menu_page_t *menu_push(menu_t *menu, char *title, void(*draw_func)(menu_t *, int, PlaydateAPI*)) {
	// error_if(menu->index >= MENU_PAGES_MAX-1, "MENU_PAGES_MAX exceeded");
	menu_page_t *page = &menu->pages[++menu->index];
	page->layout_flags = MENU_VERTICAL | MENU_ALIGN_CENTER;
	page->block_width = 320;
	page->title = title;
	page->subtitle = NULL;
	page->draw_func = draw_func;
	page->entries_len = 0;
	page->index = 0;
	page->title_anchor = UI_POS_MIDDLE | UI_POS_CENTER;
	page->items_anchor = UI_POS_MIDDLE | UI_POS_CENTER;
	return page;
}

menu_page_t *menu_confirm(menu_t *menu, char *title, char *subtitle, char *yes, char *no, void(*confirm_func)(menu_t *, int)) {
	// error_if(menu->index >= MENU_PAGES_MAX-1, "MENU_PAGES_MAX exceeded");
	menu_page_t *page = &menu->pages[++menu->index];
	page->layout_flags = MENU_HORIZONTAL;
	page->title = title;
	page->subtitle = subtitle;
	page->draw_func = NULL;
	page->entries_len = 0;
	menu_page_add_button(page, 1, yes, confirm_func);
	menu_page_add_button(page, 0, no, confirm_func);
	page->index = 1;
	page->title_anchor = UI_POS_MIDDLE | UI_POS_CENTER;
	page->items_anchor = UI_POS_MIDDLE | UI_POS_CENTER;
	return page;
}

void menu_pop(menu_t *menu) {
	if (menu->index == 0) {
		return;
	}
	menu->index--;
}

void menu_page_add_button(menu_page_t *page, int data, char *text, void(*select_func)(menu_t *, int)) {
	// error_if(page->entries_len >= MENU_ENTRIES_MAX-1, "MENU_ENTRIES_MAX exceeded");
	menu_entry_t *entry = &page->entries[page->entries_len++];
	entry->data = data;
	entry->text = text;
	entry->select_func = select_func;
	entry->type = MENU_ENTRY_BUTTON;
}

void menu_page_add_toggle(menu_page_t *page, int data, char *text, const char **options, int len, void(*select_func)(menu_t *, int)) {
	// error_if(page->entries_len >= MENU_ENTRIES_MAX-1, "MENU_ENTRIES_MAX exceeded");
	menu_entry_t *entry = &page->entries[page->entries_len++];
	entry->data = data;
	entry->text = text;
	entry->select_func = select_func;
	entry->type = MENU_ENTRY_TOGGLE;
	entry->options = options;
	entry->options_len = len;
}


void menu_update(menu_t *menu, PlaydateAPI *pd) {
	render_set_view_2d();
	
	// error_if(menu->index < 0, "Attempt to update menu without a page");
	menu_page_t *page = &menu->pages[menu->index];

	// Handle menu entry selecting
	int last_index = page->index;
	int selected_data = 0;
	if (page->entries_len > 0) {
		if (flags_is(page->layout_flags, MENU_HORIZONTAL)) {
			if (input_pressed(A_MENU_LEFT, pd)) {
				page->index--;
			}
			else if (input_pressed(A_MENU_RIGHT, pd)) {
				page->index++;
			}
		}
		else {
			if (input_pressed(A_MENU_UP, pd)) {
				page->index--;
			}
			if (input_pressed(A_MENU_DOWN, pd)) {
				page->index++;
			}
		}

		if (page->index >= page->entries_len) {
			page->index = 0;
		}
		if (page->index < 0) {
			page->index = page->entries_len - 1;
		}

		if (last_index != page->index) {
			sfx_play(SFX_MENU_MOVE);
		}
		selected_data = page->entries[page->index].data;
	}

	if (page->draw_func) {
		page->draw_func(menu, selected_data, pd);
	}

	render_set_view_2d();

	// Draw Horizontal (confirm)
	if (flags_is(page->layout_flags, MENU_HORIZONTAL)) {
		vec2i_t pos = vec2i(0, -20);
		ui_draw_text_centered(page->title, ui_scaled_pos(page->title_anchor, pos, pd), UI_SIZE_8, true, pd);
		if (page->subtitle) {
			pos.y += 12;
			ui_draw_text_centered(page->subtitle, ui_scaled_pos(page->title_anchor, pos, pd), UI_SIZE_8, true, pd);
		}
		pos.y += 16;

		page = &menu->pages[menu->index];
		pos.x = -50;
		for (int i = 0; i < page->entries_len; i++) {
			menu_entry_t *entry = &page->entries[i];
			bool text_color;
			if (i == page->index && blink()) {
				text_color = false;
			}
			else {
				text_color = true;
			}
			ui_draw_text_centered(entry->text, ui_scaled_pos(page->items_anchor, pos, pd), UI_SIZE_16, text_color, pd);
			pos.x = 60;
		}
	}

	// Draw Vertical
	else {
		vec2i_t title_pos, items_pos;
		if (flags_not(page->layout_flags, MENU_FIXED)) {
			int height = 20 + page->entries_len * 12;
			title_pos = vec2i(0, -height/2);
			items_pos = vec2i(0, -height/2 + 20);
		}
		else {
			title_pos = page->title_pos;
			items_pos = page->items_pos;
		}
		// if (flags_is(page->layout_flags, MENU_ALIGN_CENTER)) {
		// 	ui_draw_text_centered(page->title, ui_scaled_pos(page->title_anchor, title_pos, pd), UI_SIZE_8, true, pd);
		// }
		// else {
		// 	ui_draw_text(page->title, ui_scaled_pos(page->title_anchor, title_pos, pd), UI_SIZE_8, true, pd);	
		// }

		page = &menu->pages[menu->index];
		for (int i = 0; i < page->entries_len; i++) {
			menu_entry_t *entry = &page->entries[i];
			bool text_color;
			if (i == page->index && blink()) {
				text_color = false;
			}
			else {
				text_color = true;
			}
			
			ui_draw_text(entry->text, vec2i(5, (i + 1) * 20), UI_SIZE_8, text_color, pd);

			// if (flags_is(page->layout_flags, MENU_ALIGN_CENTER)) {
			// 	ui_draw_text_centered(entry->text, ui_scaled_pos(page->items_anchor, items_pos, pd), UI_SIZE_8, text_color, pd);
			// }
			// else {
			// 	ui_draw_text(entry->text, ui_scaled_pos(page->items_anchor, items_pos, pd), UI_SIZE_8, text_color, pd);
			// }

			if (entry->type == MENU_ENTRY_TOGGLE) {
				vec2i_t toggle_pos = items_pos;
				toggle_pos.x += page->block_width - ui_text_width(entry->options[entry->data], UI_SIZE_8);
				ui_draw_text(entry->options[entry->data], ui_scaled_pos(page->items_anchor, toggle_pos, pd), UI_SIZE_8, text_color, pd);
			}
			items_pos.y += 12;
		}
	}

	// Handle back buttons
	if (input_pressed(A_MENU_BACK, pd) || input_pressed(A_MENU_QUIT, pd)) {
		if (menu->index != 0) {
			menu_pop(menu);
			sfx_play(SFX_MENU_SELECT);
		}
		return;
	}

	if (page->entries_len == 0) {
		return;
	}


	// Handle toggle entries
	menu_entry_t *entry = &page->entries[page->index];

	if (entry->type == MENU_ENTRY_TOGGLE) {
		if (input_pressed(A_MENU_LEFT, pd)) {
			sfx_play(SFX_MENU_SELECT);
			entry->data--;
			if (entry->data < 0) {
				entry->data = entry->options_len-1;
			}
			if (entry->select_func) {
				entry->select_func(menu, entry->data);
			}
		}
		else if (input_pressed(A_MENU_RIGHT, pd) || input_pressed(A_MENU_SELECT, pd)) {
			sfx_play(SFX_MENU_SELECT);
			entry->data = (entry->data + 1) % entry->options_len;
			if (entry->select_func) {
				entry->select_func(menu, entry->data);
			}
		}
	}

	// Handle buttons
	else {
		if (input_pressed(A_MENU_SELECT, pd) || input_pressed(A_MENU_START, pd)) {
			if (entry->select_func) {
				sfx_play(SFX_MENU_SELECT);
				if (entry->type == MENU_ENTRY_TOGGLE) {
					entry->data = (entry->data + 1) % entry->options_len;
				}
				entry->select_func(menu, entry->data);
			}
		}
	}
}
