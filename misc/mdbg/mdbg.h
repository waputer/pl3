#pragma once

/*
 * required headers
 */
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * external declarations
 */
extern int mdbg_cnt;
extern struct avl_root_t mdbg_tree;

/*
 * function declarations
 */
void mdbg_check(void);
void mdbg_assert(void);

void *mdbg_malloc(size_t size, const char *file, uint64_t line);
void *mdbg_malloc_release(size_t size);
void *mdbg_malloc_test(size_t size);
void *mdbg_malloc_debug(size_t size, const char *file, uint64_t line);

void *mdbg_realloc(void *ptr, size_t size);
void *mdbg_realloc_release(void *ptr, size_t size);
void *mdbg_realloc_test(void *ptr, size_t size);
void *mdbg_realloc_debug(void *ptr, size_t size);

void mdbg_free(void *ptr);
void mdbg_free_release(void *ptr);
void mdbg_free_test(void *ptr);
void mdbg_free_debug(void *ptr);

char *mdbg_mprintf(const char *restrict fmt, ...) __attribute__ ((format (printf, 1, 2)));
char *mdbg_vmprintf(const char *restrict fmt, va_list args);
__attribute__((noreturn)) void mdbg_fatal_release(const char *restrict fmt, ...);
__attribute__((noreturn)) void mdbg_fatal_debug(const char *file, uint64_t line, const char *restrict fmt, ...);


/**
 * Debugged duplicate memory.
 *   @ptr: The original pointer.
 *   @size: The size.
 *   @file: The file.
 *   @line: The line.
 *   &returns: The duplicated memory.
 */
static inline void *mdbg_memdup(const void *ptr, size_t size, const char *file, uint64_t line)
{
	void *alloc;

	alloc = mdbg_malloc(size, file, line);
	memcpy(alloc, ptr, size);

	return alloc;
}

/**
 * Debugged duplicate a string.
 *   @str: The string.
 *   @file: The file.
 *   @line: The line.
 *   &returns: The copy.
 */
static inline char *mdbg_strdup(const char *str, const char *file, uint64_t line)
{
	return mdbg_memdup(str, strlen(str) + 1, file, line);
}


/*
 * fallback definitions
 */
static inline void *c_malloc(size_t nbytes) { return malloc(nbytes); }
static inline void *c_realloc(void *ptr, size_t nbytes) { return realloc(ptr, nbytes); }
static inline void c_free(void *ptr) { free(ptr); }


/*
 * redefine symbols
 */
#ifndef MDBG_NO_RENAME
#	undef malloc
#	undef realloc
#	undef free
#	undef memdup
#	undef strdup
#	define malloc(size) mdbg_malloc(size, __FILE__, __LINE__)
#	define realloc mdbg_realloc
#	define free mdbg_free
#	define memdup(ptr, size) mdbg_memdup(ptr, size, __FILE__, __LINE__)
#	define strdup(str) mdbg_strdup(str, __FILE__, __LINE__)
#	
#	ifndef RELEASE
#		define mdbg_fatal(...) mdbg_fatal_debug(__FILE__, __LINE__, __VA_ARGS__)
#	else
#		define mdbg_fatal(...) mdbg_fatal_release(__VA_ARGS__)
#	endif
#
#	define mprintf mdbg_mprintf
#	define vmprintf mdbg_vmprintf
#	define fatal mdbg_fatal
#endif
