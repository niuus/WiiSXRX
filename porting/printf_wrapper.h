#ifndef PRINTF_WRAPPER_H
#define PRINTF_WRAPPER_H

#include "debug_vconsole.h"

#ifdef printf
#undef printf
#endif

#define printf(fmt, ...) DEBUG_vc_printf__(fmt, ##__VA_ARGS__)

#endif
