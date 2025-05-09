#include "bio.h"

#include "../mdbg/mdbg.h"

#include <string.h>


/*
 * local declarations
 */
static int64_t file_read(void *restrict ref, void *restrict buf, uint64_t size);
static int64_t file_write(void *restrict ref, const void *restrict buf, uint64_t size);

static struct io_file_i file_iface = { file_read, file_write };


/**
 * Wrap a file pointer with a better I/O file.
 *   @file: The file.
 *   &returns: The file.
 */
struct io_file_t io_file_wrap(FILE *file)
{
	return (struct io_file_t){ file, &file_iface };
}

static int64_t file_read(void *restrict ref, void *restrict buf, uint64_t size)
{
	return fread(buf, size, 1, ref);
}

static int64_t file_write(void *restrict ref, const void *restrict buf, uint64_t size)
{
	return fwrite(buf, size, 1, ref);
}


/**
 * Better formatted printing.
 *   @file: The file.
 *   @fmt: The format.
 *   @...: The arguments.
 */
void io_printf(struct io_file_t file, const char *restrict fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	io_vprintf(file, fmt, args);
	va_end(args);
}

/**
 * Better formatted printing.
 *   @file: The file.
 *   @fmt: The format.
 *   @...: The arguments.
 */
void io_vprintf(struct io_file_t file, const char *restrict fmt, va_list args)
{
	const char *find;

	while((find = strchr(fmt, '%')) != NULL) {
		io_file_write(file, fmt, find - fmt);

		fmt = find + 1;

		switch(*fmt) {
		case '%':
			io_file_write(file, fmt, 1);
			break;

		case 's':
			{
				const char *str = va_arg(args, const char *);

				io_file_write(file, str, strlen(str));
			}

			break;

		case 'C':
			io_call(va_arg(args, struct io_callback_t), file);
			break;

		default:
			fatal("Invalid bprintf specifier '%c'.", *fmt);
		}

		fmt++;
	}

	io_file_write(file, fmt, strlen(fmt));
}
