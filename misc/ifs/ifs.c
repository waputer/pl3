#include "ifs.h"

#include "../mdbg/mdbg.h"

#include <unistd.h>


/**
 * Check if a file exists.
 *   @path: The path.
 */
bool ifs_exists(const char *path)
{
	return access(path, F_OK) == 0;
}



/**
 * Retrieve the directory name for a path.
 *   @path: The path.
 *   &returns: The directory name. Must be freed.
 */
char *ifs_dirname(const char *path)
{
	char *ret, *ptr = strrchr(path, '/');

	if(ptr != NULL) {
		ret = malloc(ptr - path + 1);
		memcpy(ret, path, ptr - path);
		ret[ptr - path] = '\0';
	}
	else
		ret = strdup(".");

	return ret;
}
