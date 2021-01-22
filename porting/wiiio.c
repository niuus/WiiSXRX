#include "wiiio.h"
#include <stdlib.h>
#include "../libpcsxcore/system.h"

struct wii_file
{
	fileBrowser_file* file;
	wiiio_type_t wiiioType;
};

int wiiio_register_filebrowser(fileBrowser_file* file, const char *pathname)
{
	return 0;
}

WIIFILE *wiiio_fopen(const char *pathname, const char *mode)
{
	return NULL;
}

WIIFILE *wiiio_fbopen(fileBrowser_file* file, wiiio_type_t wiiioType)
{
	if (!file) {
		SysPrintf("wiiio_fopen(): error: Null pointer for fileBrowser_file.\n");
		return NULL;
	}
	WIIFILE* fp = malloc(sizeof(struct wii_file));
	fp->file = file;
	fp->file->offset = 0; // reset position to the beginning
	fp->wiiioType = wiiioType;
	SysPrintf("wiiio_fopen(): Successful open for %s\n", file->name);
	return fp;
}

size_t wiiio_fread(void *ptr, size_t size, size_t nmemb, WIIFILE *stream)
{
	if (!stream) {
		SysPrintf("wiiio_fread(): error: Null pointer for WIIFILE.\n");
		return 0;
	}
	if (!stream->file) {
		SysPrintf("wiiio_fread(): error: Null pointer for internal fileBrowser_file.\n");
		return 0;
	}
	if (!ptr) {
		SysPrintf("wiiio_fread(): error: Null pointer for ptr.\n");
		return 0;
	}
	int cnt = -1;
	switch (stream->wiiioType) {
		case WIIIO_TYPE_UNKNOWN:
			SysPrintf("wiiio_fread(): error: Unknown wiiio type.\n");
			return 0;
		case WIIIO_TYPE_ISO_FILE:
			cnt = isoFile_readFile(stream->file, ptr, nmemb * size);
			break;
		case WIIIO_TYPE_SAVE_FILE:
			cnt = saveFile_readFile(stream->file, ptr, nmemb * size);
			break;
		case WIIIO_TYPE_BIOS_FILE:
			cnt = biosFile_readFile(stream->file, ptr, nmemb * size);
			break;
	}
	if (cnt < 0) {
		SysPrintf("wiiio_fread(): error: failed with %d\n", cnt);
		return 0;
	}
	if (cnt % size) {
		SysPrintf("wiiio_fread(): warning: Can't read a full item\n");
	}
	SysPrintf("wiiio_fread(): read %u items, item size: %u bytes\n", (size_t)cnt / size, size);

	return (size_t)cnt / size;
}

size_t wiiio_fwrite(const void *ptr, size_t size, size_t nmemb, WIIFILE *stream)
{
	if (!stream) {
		SysPrintf("wiiio_fwrite(): error: Null pointer for WIIFILE.\n");
		return 0;
	}
	if (!stream->file) {
		SysPrintf("wiiio_fwrite(): error: Null pointer for internal fileBrowser_file.\n");
		return 0;
	}
	if (!ptr) {
		SysPrintf("wiiio_fread(): error: Null pointer for ptr.\n");
		return 0;
	}
	int cnt = -1;
	switch (stream->wiiioType) {
		case WIIIO_TYPE_UNKNOWN:
			SysPrintf("wiiio_write(): error: Unknown wiiio type.\n");
			return 0;
		case WIIIO_TYPE_ISO_FILE:
			SysPrintf("wiiio_write(): No write support for ISO file.\n");
			return 0;
		case WIIIO_TYPE_SAVE_FILE:
			// remove const. cast from const void* to void* because fileBrowser use no const
			cnt = saveFile_writeFile(stream->file, (void*)ptr, nmemb * size);
			break;
		case WIIIO_TYPE_BIOS_FILE:
			SysPrintf("wiiio_write(): No write support for bios file.\n");
			break;
	}
	if (cnt < 0) {
		SysPrintf("wiiio_fwrite(): error: failed with %d\n", cnt);
		return 0;
	}
	if (cnt % size) {
		SysPrintf("wiiio_fwrite(): warning: Can't write a full item\n");
	}
	SysPrintf("wiiio_write(): read %u items, item size: %u bytes\n", (size_t)cnt / size, size);

	return (size_t)cnt / size;
}

int wiiio_fclose(WIIFILE *stream)
{
	if (!stream) {
		SysPrintf("wiiio_fclose(): error: Null pointer for WIIFILE.\n");
		return -1;
	}
	free(stream);
	SysPrintf("wiiio_fclose(): Close was successful\n");
	return 0;
}

