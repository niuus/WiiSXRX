//
// Copyright (c) 2008, Wei Mingzhi. All rights reserved.
//
// Use, redistribution and modification of this code is unrestricted as long as this
// notice is preserved.
//

#ifndef CONFIG_H
#define CONFIG_H

#if 0
#ifndef MAXPATHLEN
//match PATH_MAX in <sys/param.h>
#define MAXPATHLEN 1024
#endif
#endif

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "1.9"
#endif

#if 0
#ifndef PREFIX
#define PREFIX "./"
#endif
#endif

#if 0
#ifndef inline
#ifdef _DEBUG
#define inline /* */
#else
#define inline __inline__
#endif
#endif
#endif

#endif
