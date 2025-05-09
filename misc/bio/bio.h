#pragma once

/*
 * require headers
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

/**
 * File interface structure.
 *   @read: Read from file.
 *   @write: Write to file.
 */
struct io_file_i {
	int64_t (*read)(void *restrict , void *restrict , uint64_t);
	int64_t (*write)(void *restrict , const void *restrict , uint64_t);
};

/**
 * File structure.
 *   @ref: The reference.
 *   @iface: The interface.
 */
struct io_file_t {
	void *ref;
	const struct io_file_i *iface;
};

/**
 * Read to a file.
 *   @file: The file.
 *   @buf: The buffer.
 *   @nbytes: The number of bytes.
 *   &returns: The number of bytes read.
 */
static inline int64_t io_file_read(struct io_file_t file, void *buf, uint64_t nbytes)
{
	return file.iface->read(file.ref, buf, nbytes);
}

/**
 * Write to a file.
 *   @file: The file.
 *   @buf: The buffer.
 *   @nbytes: The number of bytes.
 *   &returns: The number of bytes written.
 */
static inline int64_t io_file_write(struct io_file_t file, const void *buf, uint64_t nbytes)
{
	return file.iface->write(file.ref, buf, nbytes);
}


/**
 * Callback structure.
 *   @func: The callback function.
 *   @arg: The callback argument.
 */
struct io_callback_t {
	void (*func)(struct io_file_t, void *);
	void *arg;
};
static inline struct io_callback_t io_callback(void (*proc)(struct io_file_t, void *), void *ref)
{
	return (struct io_callback_t){ proc, ref };
}

/**
 * Execute a callback.
 *   @callback: The callback.
 *   @file: The file.
 */
static inline void io_call(struct io_callback_t callback, struct io_file_t file)
{
	callback.func(file, callback.arg);
}


#define IO_CALLBACK_IMPL(NAM, FUN) \
	static void NAM##_callback_handler(struct io_file_t file, void *arg) { FUN(arg, file); } \
	void NAM(const void *ptr) { return io_callback(NAM##_callback_handler, (void *)ptr); } \

#define IO_CALLBACK_DECL(NAM, FUN) \
	void NAM(const void *ptr);

#define io_chunk_t io_callback_t
#define io_chunk io_callback

/*
 * file declarations
 */
struct io_file_t io_file_wrap(FILE *file);

/*
 * formatted declarations
 */
void io_printf(struct io_file_t file, const char *restrict fmt, ...);
void io_vprintf(struct io_file_t file, const char *restrict fmt, va_list args);
