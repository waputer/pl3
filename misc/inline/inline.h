#pragma once

/*
 * required headers
 */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


/**
 * Retrive the parent data structure from a member pointer.
 *   @ptr: The member poitner.
 *   @type: The parent type.
 *   @member: The member identifier.
 *   &returns: The parent pointer.
 */
#define getparent(ptr, type, member) ((type *)((void *)(ptr) - offsetof(type, member)))


/**
 * Zero memory.
 *   @ptr: The destination pointer.
 *   @size: The size in bytes.
 */
static inline void memzero(void *ptr, size_t size)
{
	memset(ptr, 0x00, size);
}

/**
 * Determine if `pre` is a prefix in the string `str`.
 *   @str: The string.
 *   @pre: The desired prefix.
 *   &returns: The pointer after the prefix if found, null otherwise.
 */
static inline char *strprefix(const char *str, const char *pre)
{
	size_t len = strlen(pre);

	if(memcmp(str, pre, len) == 0)
		return (char *)(str + len);
	else
		return NULL;
}


/**
 * Macro for creating fuctions that find the maximum of two values. The value
 * `NaN` is always "less".
 *   @NAM: The function name.
 *   @TY: The types.
 */
#define M_MAX_TY(NAM, TY) \
	static inline TY m_max_##NAM(TY a, TY b) { if(a > b) return a; else return b; }

/**
 * Macro for creating fuctions that find the minimum of two values. The value
 * `NaN` is always "less".
 *   @NAM: The function name.
 *   @TY: The types.
 */
#define M_MIN_TY(NAM, TY) \
	static inline TY m_min_##NAM(TY a, TY b) { if(a < b) return a; else return b; }

/**
 * Macro for generating all math functions for a type.
 *   @NAM: The function name.
 *   @TY: The type.
 */
#define M_ALL_TY(NAM, TY) \
	M_MAX_TY(NAM, TY) \
	M_MIN_TY(NAM, TY) \


/*
 * generate math functions
 */
M_ALL_TY(i32, int32_t);
M_ALL_TY(u32, uint32_t);
M_ALL_TY(i64, int64_t);
M_ALL_TY(u64, uint64_t);
