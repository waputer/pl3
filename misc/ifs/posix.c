#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "posix.h"
#include "ifs.h"
#include "../mdbg/mdbg.h"
#include "../emech/emech.h"


/**
 * Create a directory.
 *   @path: The path.
 *   @mode: The mode.
 *   &returns: Error.
 */
char *ifs_mkdir(const char *path, uint8_t mode)
{
	if(mkdir(path, mode) < 0)
		return mprintf("Failed to create directory '%s'. %s.", path, strerror(errno));

	if(chmod(path, mode) < 0)
		return mprintf("Failed to set permissions on '%s'. %s.", path, strerror(errno));

	return NULL;
}


/**
 * Read an entire file from a path.
 *   @path: The path.
 *   @data: Out. The file.
 *   @out: In, out. The number of bytes read. Initially set to the maximum
 *     length
 *   &returns: Error.
 */
char *ifs_getfile(const char *path, void **data, uint64_t *nbytes)
{
        int fd;
        ssize_t rd;
        void *ptr;
	struct stat stat;
        uint64_t idx, maxlen = *nbytes;

	onerr( )

        fd = open(path, O_RDONLY);
        if(fd < 0)
		return mprintf("Cannot open '%s'. %s.", path, strerror(errno));

	fstat(fd, &stat);
	*nbytes = stat.st_size;
        if((maxlen > 0) && (*nbytes > maxlen))
                *nbytes = maxlen;

        ptr = malloc(*nbytes);

        idx = 0;
        do {
                rd = read(fd, ptr + idx, *nbytes - idx);
                if(rd < 0) {
                        if(errno == EINTR)
                                continue;
                        else
				fail("Cannot read from '%s'. %s.", path, strerror(errno));
                }
                else if(rd == 0)
			fail("Cannot read from '%s'.", path);

                idx += rd;
        } while(idx < *nbytes);

	close(fd);
	*data = ptr;

	return NULL;
}
