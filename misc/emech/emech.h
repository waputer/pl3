#pragma once

/*
 * required headers
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * prototypes
 */
int asprintf(char **, const char *restrict, ...);


/**
 * Error cleanup macro.
 *   @ARG: All actions to be taken when returning from an error.
 */
#define onerr(ARG) \
	char *_emech_error; \
	goto _emech_after; \
	_emech_return: \
	do { ARG } while(0); \
	return _emech_error; \
	goto _emech_return; \
	_emech_after: \

/**
 * Fail macro, calling the `onerr` code and returning the error message.
 *   @...: The printf-style format and arguments.
 */
#define fail(...) \
	do { if(asprintf(&_emech_error, __VA_ARGS__) < 0) { fprintf(stderr, "cannot allocate string\n"); abort(); }; goto _emech_return; } while(0)

/**
 * Fail macro, calling the `onerr` code and returning the error message.
 *   @msg: The heap allocated message.
 */
#define failmsg(msg) \
	do { _emech_error = msg; goto _emech_return; } while(0)

/**
 * Check and return macro.
 *   @err: The error value to check, returning and cleaning up on failure.
 */
#define chkret(val) \
	do { char *_emech_tmp = val; if(_emech_tmp != NULL) { _emech_error = _emech_tmp; goto _emech_return; } } while(0)

/**
 * Check and exit macro.
 *   @err: The error value to check, exiting with an error on failure.
 */
#define chkexit(val) \
	do { char *_emech_tmp = val; if(_emech_tmp != NULL) { fprintf(stderr, "%s\n", _emech_tmp); exit(1); } } while(0)

/**
 * Check and abort macro.
 *   @err: The error value to check, aborting the program on failure.
 */
#define chkabort(val) \
	do { char *_emech_tmp = val; if(_emech_tmp != NULL) { fprintf(stderr, "%s\n", _emech_tmp); abort(); } } while(0)

/**
 * Check and convert to bool macro.
 *   @err: The error value to check, aborting the program on failure.
 *   &returns: True if successful, false on failure.
 */
#define chkbool(val) \
	({ char *_emech_tmp = val; if(_emech_tmp != NULL) free(_emech_tmp); _emech_tmp == NULL; })

/**
 * Check and convert to bool macro, printing a warning on failure.
 *   @err: The error value to check, aborting the program on failure.
 *   &returns: True if successful, false on failure.
 */
#define chkwarn(val) \
	({ char *_emech_tmp = val; if(_emech_tmp != NULL) { fprintf(stderr, "%s\n", _emech_tmp); free(_emech_tmp); } _emech_tmp == NULL; })

/**
 * Unreachable macro. If `RELEASE` is defined, the compiler will optimize as
 * if the surrounding code cannot be executed. If `RELEASE` is not defined, it
 * prints an error and aborts. This function should never be called in normal
 * operation.
 */
#ifdef RELEASE
#	define unreachable() __builtin_unreachable();
#else
__attribute__((noreturn)) static inline void unreachable() {
	fprintf(stderr, "Fatal error. Unreachable code.\n");
	abort();
}
#endif
