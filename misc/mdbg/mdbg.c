#define MDBG_NO_RENAME 1

#include "mdbg.h"
#include "../avl/avl.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>


/*
 * memory counter variable
 */
int mdbg_cnt = 0;

struct avl_root_t mdbg_tree = { 0, NULL, avl_cmp_ptr };

/*
 * local declarations
 */
static void chk(bool val);


/**
 * Check for any memory leaks.
 */
void mdbg_check(void)
{
	if(mdbg_cnt != 0) {
		struct avl_node_t *node;

		fprintf(stderr, "missed %d allocations\n", mdbg_cnt);

		for(node = avl_root_first(&mdbg_tree); node != NULL; node = avl_node_next(node)) {
			const char *file = *(const char **)((void *)node - 16);
			uint64_t line = *(uint64_t *)((void *)node - 8);

			fprintf(stderr, "  %s:%lu\n", file, line);
		}
	}
}

/**
 * Check for any memory leaks.
 */
void mdbg_assert(void)
{
	if(mdbg_cnt != 0) {
		mdbg_check();

#ifndef RELEASE
		abort();
#endif
	}
}


/**
 * Base version of malloc.
 *   @size: The allocation size.
 *   @file: The file.
 *   @line: The line.
 *   &returns: The allocated pointer.
 */
void *mdbg_malloc(size_t size, const char *file, uint64_t line)
{
#ifdef RELEASE
	return mdbg_malloc_release(size);
#elif TEST
	return mdbg_malloc_test(size);
#elif DEBUG
	return mdbg_malloc_debug(size, file, line);
#else
#	error "Must define one of 'RELEASE', 'TEST', or 'DEBUG'."
#endif
}

/**
 * Release version of malloc.
 *   @size: The allocation size.
 *   &returns: The allocated pointer.
 */
void *mdbg_malloc_release(size_t size)
{
	void *ptr;

	ptr = malloc(size ?: 1);
	if(ptr == NULL) {
		fprintf(stderr, "memory allocation failed\n");
		abort();
	}

	return ptr;
}

/**
 * Test version of malloc.
 *   @size: The allocation size.
 *   &returns: The allocated pointer.
 */
void *mdbg_malloc_test(size_t size)
{
	mdbg_cnt++;

	return mdbg_malloc_release(size);
}

/**
 * Debug version of malloc.
 *   @size: The allocation size.
 *   @file: The file.
 *   @line: The line.
 *   &returns: The allocated pointer.
 */
void *mdbg_malloc_debug(size_t size, const char *file, uint64_t line)
{
	void *ptr;

	ptr = mdbg_malloc_test(48 + size + sizeof(struct avl_node_t));

	*(uint64_t *)(ptr + 0) = (uint64_t)size;
	*(uint64_t *)(ptr + 8) = (uint64_t)(ptr + 8) * 4221379234814892313ul;
	*(uint64_t *)(ptr + 16 + size + 0) = (uint64_t)(ptr + 16 + 0) * 4221379234814892313ul;
	*(uint64_t *)(ptr + 16 + size + 8) = (uint64_t)(ptr + 16 + 8) * 4221379234814892313ul;
	*(const void **)(ptr + 16 + size + 16) = file;
	*(uint64_t *)(ptr + 16 + size + 24) = line;
	avl_node_init(ptr + 16 + size + 32, ptr);
	avl_root_add(&mdbg_tree, ptr + 16 + size + 32);

	return ptr + 16;
}


/**
 * Base version of realloc.
 *   @ptr: The pointer.
 *   @size: The size.
 *   &returns: The reallocated pointer.
 */
void *mdbg_realloc(void *ptr, size_t size)
{
#ifdef RELEASE
	return mdbg_realloc_release(ptr, size);
#elif TEST
	return mdbg_realloc_test(ptr, size);
#elif DEBUG
	return mdbg_realloc_debug(ptr, size);
#else
#	error "Must define one of 'RELEASE', 'TEST', or 'DEBUG'."
#endif
}

/**
 * Release version of free.
 *   @ptr: The pointer.
 *   @size: The size.
 *   &returns: The reallocated pointer.
 */
void *mdbg_realloc_release(void *ptr, size_t size)
{
	if(ptr == NULL) {
		fprintf(stderr, "attempted to realloc null pointer\n");
		abort();
	}

	return realloc(ptr, size);
}

/**
 * Test version of realloc.
 *   @ptr: The pointer.
 *   @size: The size.
 *   &returns: The reallocated pointer.
 */
void *mdbg_realloc_test(void *ptr, size_t size)
{
	return mdbg_realloc_release(ptr, size);
}

/**
 * Debug version of realloc.
 *   @ptr: The pointer.
 *   @size: The size.
 *   &returns: The reallocated pointer.
 */
void *mdbg_realloc_debug(void *ptr, size_t size)
{
	size_t chksize;

	ptr -= 16;

	chksize = *(uint64_t *)ptr;
	chk(*(uint64_t *)(ptr + 8) == (uint64_t)(ptr + 8) * 4221379234814892313ul);
	chk(*(uint64_t *)(ptr + 16 + chksize + 0) == (uint64_t)(ptr + 16 + 0) * 4221379234814892313ul);
	chk(*(uint64_t *)(ptr + 16 + chksize + 8) == (uint64_t)(ptr + 16 + 8) * 4221379234814892313ul);
	chk(avl_root_remove(&mdbg_tree, ptr) == (ptr + 16 + chksize + 32));

	ptr = mdbg_realloc_test(ptr, 48 + size + sizeof(struct avl_node_t));

	*(uint64_t *)(ptr + 0) = (uint64_t)size;
	*(uint64_t *)(ptr + 8) = (uint64_t)(ptr + 8) * 4221379234814892313ul;
	*(uint64_t *)(ptr + 16 + size + 0) = (uint64_t)(ptr + 16 + 0) * 4221379234814892313ul;
	*(uint64_t *)(ptr + 16 + size + 8) = (uint64_t)(ptr + 16 + 8) * 4221379234814892313ul;
	avl_node_init(ptr + 16 + size + 32, ptr);
	avl_root_add(&mdbg_tree, ptr + 16 + size + 32);

	return ptr + 16;
}


/**
 * Base version of free.
 *   @ptr: The pointer.
 */
void mdbg_free(void *ptr)
{
#ifdef RELEASE
	return mdbg_free_release(ptr);
#elif TEST
	return mdbg_free_test(ptr);
#elif DEBUG
	return mdbg_free_debug(ptr);
#else
#	error "Must define one of 'RELEASE', 'TEST', or 'DEBUG'."
#endif
}

/**
 * Release version of free.
 *   @ptr: The pointer.
 */
void mdbg_free_release(void *ptr)
{
	if(ptr == NULL) {
		fprintf(stderr, "attempted to free null pointer\n");
		abort();
	}

	free(ptr);
}

/**
 * Test version of free.
 *   @ptr: The pointer.
 */
void mdbg_free_test(void *ptr)
{
	mdbg_free_release(ptr);
	mdbg_cnt--;
}

/**
 * Debug version of free.
 *   @ptr: The pointer.
 */
void mdbg_free_debug(void *ptr)
{
	size_t size;

	ptr -= 16;

	size = *(uint64_t *)ptr;
	chk(*(uint64_t *)(ptr + 8) == (uint64_t)(ptr + 8) * 4221379234814892313ul);
	chk(*(uint64_t *)(ptr + 16 + size + 0) == (uint64_t)(ptr + 16 + 0) * 4221379234814892313ul);
	chk(*(uint64_t *)(ptr + 16 + size + 8) == (uint64_t)(ptr + 16 + 8) * 4221379234814892313ul);
	chk(avl_root_remove(&mdbg_tree, ptr) == (ptr + 16 + size + 32));

	mdbg_free_test(ptr);
}


/**
 * Run a printf call directly to an allocated string.
 *   @fmt; The printf format.
 *   @...: The printf arguments.
 *   &returns: The allocated string.
 */
char *mdbg_mprintf(const char *restrict fmt, ...)
{
	char *str;
	va_list args;

	va_start(args, fmt);
	str = mdbg_vmprintf(fmt, args);
	va_end(args);

	return str;
}

/**
 * Run a vprintf call directly writing to an allocated string.
 *   @fmt; The printf format.
 *   @args: The printf arguments.
 *   &returns: The allocated string.
 */
char *mdbg_vmprintf(const char *restrict fmt, va_list args)
{
	char *str;
	size_t len;
	va_list copy;

	va_copy(copy, args);
	len = vsnprintf(NULL, 0, fmt, copy);
	va_end(copy);

	str = mdbg_malloc(len + 1, __FILE__, __LINE__);
	vsprintf(str, fmt, args);

	return str;
}

/**
 * Terminate a program with a message.
 *   @fmt: The printf format.
 *   @...: The printf arguments.
 *   &noreturn
 */
__attribute__((noreturn)) void mdbg_fatal_release(const char *restrict fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);

	abort();
}

/**
 * Terminate a program with a message and line information.
 *   @file: The file.
 *   @line: The line.
 *   @fmt: The printf format.
 *   @...: The printf arguments.
 *   &noreturn
 */
__attribute__((noreturn)) void mdbg_fatal_debug(const char *file, uint64_t line, const char *restrict fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "%s:%lu: ", file, line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);

	abort();
}

/**
 * Helper that checks for heap corruption.
 */
static void chk(bool val)
{
	if(!val) {
		fprintf(stderr, "heap corruption\n");
		abort();
	}
}
