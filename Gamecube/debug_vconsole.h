#ifndef DEBUG_VCONSOLE_H
#define DEBUG_VCONSOLE_H

/*
by retro100

If the macro SHOW_DEBUGVC is defined then
a virtual console with a size of 86x40 can
be used to print messages.
The output of the virtual console scrolls
if a new line is added.
\n and also \t are supported.
The font is NOT monospace. The effective length
of the rendered output depends on the used characters.
*/

#ifdef SHOW_DEBUGVC
#define DEBUG_vc_printf(fmt, ...) DEBUG_vc_printf__(fmt, ##__VA_ARGS__)
#define DEBUG_vc_print(text) DEBUG_vc_print__(text)
#define DEBUG_vc_render_console() DEBUG_vc_render_console__()
#else
#define DEBUG_vc_printf(fmt, ...)
#define DEBUG_vc_print(fmt, ...)
#define DEBUG_vc_render_console()
#endif


#ifdef __cplusplus
extern "C" {
#endif

void DEBUG_vc_printf__(const char *format, ...)
#if defined(__GNUC__)
	__attribute__ ((format(printf, 1, 2)))
#endif
	;

void DEBUG_vc_print__(const char* text);
void DEBUG_vc_render_console__();

#ifdef __cplusplus
}
#endif

#endif

