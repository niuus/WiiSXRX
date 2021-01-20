#include "debug_vconsole.h"

#ifdef SHOW_DEBUGVC

#include <gccore.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "libgui/IPLFontC.h"

#define DEBUG_VCONSOLE_WIDTH  87 // usable are 86
#define DEBUG_VCONSOLE_HEIGHT 40
#define MAX_PRINTF_LEN 128

static char vc_text_lines[DEBUG_VCONSOLE_HEIGHT][DEBUG_VCONSOLE_WIDTH] = {0};
static int vc_current_line_index = 0;
static int vc_current_width_cursor_position = 0;
static char *vc_current_line = vc_text_lines[0];

static void vc_jump_to_next_line() {
	// terminate the current line
	vc_current_line[vc_current_width_cursor_position] = '\0';

	// jump to next line
	vc_current_width_cursor_position = 0;
	++vc_current_line_index;
	if (vc_current_line_index >= DEBUG_VCONSOLE_HEIGHT) {
		vc_current_line_index = 0;
	}
	vc_current_line = vc_text_lines[vc_current_line_index];

	// clear the line content of the new line.
	// Using memset instead of simple
	// vc_current_line[vc_current_width_cursor_position] = '\0';
	// because if there are race conditions of different threads
	// at the current line then it always has a termination
	// at any position.
	memset(vc_current_line, 0, DEBUG_VCONSOLE_WIDTH);
}

void DEBUG_vc_printf__(const char *format, ...)
{
	char msg[MAX_PRINTF_LEN];

	va_list args;
	va_start (args, format);
	vsnprintf(msg, MAX_PRINTF_LEN, format, args);
	va_end (args);

	msg[MAX_PRINTF_LEN - 1] = '\0';

	DEBUG_vc_print(msg);
}

void DEBUG_vc_print__(const char* text)
{
	for (; *text; ++text) {
		char ch = *text;
		if (ch == '\n') {
			vc_jump_to_next_line();
		}
		else if (ch == '\r') {
			// ignore it.
			// --> For a new line \r\n or \n must be used.
			// Only \r makes no new line.
		}
		else if (ch == '\t') {
			if (vc_current_width_cursor_position >= DEBUG_VCONSOLE_WIDTH - 4) {
				vc_jump_to_next_line();
			}
			vc_current_line[vc_current_width_cursor_position] += ' ';
			vc_current_line[vc_current_width_cursor_position + 1] += ' ';
			vc_current_line[vc_current_width_cursor_position + 2] += ' ';
			vc_current_line[vc_current_width_cursor_position + 3] += ' ';

			vc_current_width_cursor_position += 4;
			if (vc_current_width_cursor_position >= DEBUG_VCONSOLE_WIDTH - 1) {
				vc_jump_to_next_line();
			}
		}
		else {
			if (ch < 0x20 || ch >= 0x7f) {
				ch = '?';
			}
			vc_current_line[vc_current_width_cursor_position] = ch;
			++vc_current_width_cursor_position;
			if (vc_current_width_cursor_position >= DEBUG_VCONSOLE_WIDTH - 1) {
				vc_jump_to_next_line();
			}
		}
	}
	// terminate the new added content
	vc_current_line[vc_current_width_cursor_position] = '\0';
}

void DEBUG_vc_render_console__()
{
	int line_index = 0;
	for (int i = vc_current_line_index + 1; i < DEBUG_VCONSOLE_HEIGHT; ++i, ++line_index) {
		IplFont_drawString(20, (10 * line_index + 60), vc_text_lines[i], 0.5, false);
	}
	for (int i = 0; i <= vc_current_line_index; ++i, ++line_index) {
		IplFont_drawString(20, (10 * line_index + 60), vc_text_lines[i], 0.5, false);
	}
}

#else

void DEBUG_vc_printf__(const char *format, ...)
{
}

void DEBUG_vc_print__(const char* text)
{
}

void DEBUG_vc_render_console__()
{
}

#endif

