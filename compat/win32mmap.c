#include "../git-compat-util.h"

/* Note that this doesn't return the actual pagesize, but
 * the allocation granularity. If future Windows specific git code
 * needs the real getpagesize function, we need to find another solution.
 */
int mingw_getpagesize(void)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwAllocationGranularity;
}

void *git_mmap
(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	HANDLE hmap;
	void *temp;
	size_t len;
	struct stat st;
	uint64_t o = offset;
	uint32_t l = o & 0xFFFFFFFF;
	uint32_t h = (o >> 32) & 0xFFFFFFFF;

	if (!fstat(fd, &st))
		len = xsize_t(st.st_size);
	else
		die("mmap: could not determine filesize");

	if ((length + offset) > len)
		length = len - offset;

	if (!(flags & MAP_PRIVATE))
		die("Invalid usage of mmap when built with USE_WIN32_MMAP");

	hmap = CreateFileMapping((HANDLE)_get_osfhandle(fd), 0, PAGE_WRITECOPY,
		0, 0, 0);

	if (!hmap)
		return MAP_FAILED;

	temp = MapViewOfFileEx(hmap, FILE_MAP_COPY, h, l, length, start);

	if (!CloseHandle(hmap))
		warning("unable to close file mapping handle\n");

	return temp ? temp : MAP_FAILED;
}

int git_munmap(void *start, size_t length)
{
	return !UnmapViewOfFile(start);
}
