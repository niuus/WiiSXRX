#ifndef DEBUG_ASSERT_H
#define DEBUG_ASSERT_H

#ifdef SHOW_DEBUGVC

#include "debug_vconsole.h"

#define assert(x) { \
	if ((x)) { \
		DEBUG_vc_printf__("assert: %s:%d\n", __FILE__, __LINE__); \
	} \
}

#else

#define assert(x)

#endif

#endif
