#ifndef WIIIO_H
#define WIIIO_H

/*
WiiIO is a stream io interface compatible to the
standard C stream io interface and uses internal
the fileBrowser implementation.
It's compatible to fopen(), fread(), fwrite(), etc.

The advantage is the easy porting of existing code
which uses fopen(), fread() etc.

So that the names of the calls to fopen(), fread()
etc. do not have to be changed, stdio_file_wrapper.h
can be included.
*/

#include <stddef.h>
#include "../Gamecube/fileBrowser/fileBrowser.h"

typedef enum wiiio_type
{
	WIIIO_TYPE_UNKNOWN = 0,
	WIIIO_TYPE_ISO_FILE,
	WIIIO_TYPE_SAVE_FILE,
	WIIIO_TYPE_BIOS_FILE,
} wiiio_type_t;

typedef struct wii_file WIIFILE;

int wiiio_register_filebrowser(fileBrowser_file* file, const char *pathname);

/**
 * @return Return a WIIFILE pointer on success. Otherwise NULL for an error.
 */
WIIFILE *wiiio_fopen(const char *pathname, const char *mode);

/**
 * @return Return a WIIFILE pointer on success. Otherwise NULL for an error.
 */
WIIFILE *wiiio_fbopen(fileBrowser_file* file, wiiio_type_t wiiioType);

/**
 * This function reads 'nmemb' items of data, each 'size' bytes long,
 * from the stream pointed to by 'stream', storing them at the location
 * given by 'ptr'.
 *
 * @return On success, it returns the number of items read.
 *         The count of items and NOT the count of bytes.
 *         This number equals the number of bytes transferred only when
 *         'size' is 1.
 *         It does not distinguish between end-of-file and error. In both
 *         cases 0 is returned.
 */
size_t wiiio_fread(void *ptr, size_t size, size_t nmemb, WIIFILE *stream);

/**
 * This function writes 'nmemb' items of data, each 'size' bytes long,
 * to the stream pointed to by 'stream', obtaining them from the location
 * given by 'ptr'.
 *
 * @return On success, it returns the number of items written.
 *         The count of items and NOT the count of bytes.
 *         This number equals the number of bytes transferred only when
 *         'size' is 1.
 *         If the return value doesn't equal with 'nmemb' then
 *         a full write was NOT possible.
 */
size_t wiiio_fwrite(const void *ptr, size_t size, size_t nmemb, WIIFILE *stream);

/**
 * @return Return 0 for success. Return -1 (EOF) for error.
 */
int wiiio_fclose(WIIFILE *stream);

#endif
