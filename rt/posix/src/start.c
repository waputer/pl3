#include <errno.h>

int geterr(void) {
	return errno;
};

void seterr(int val) {
	errno = val;
}
