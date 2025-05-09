#pragma once

/*
 * common headers
 */
#include <stdint.h>

/**
 * Memory buffer structure.
 *   @arr: The data array.
 *   @len, max: The current length and capacity.
 */
struct membuf_t {
	uint8_t *arr;
	uint64_t len, max;
};

/*
 * memory buffer declarations
 */
struct membuf_t membuf_init(uint64_t init);
void membuf_destroy(struct membuf_t buf);
void *membuf_done(struct membuf_t *buf, uint64_t *len);

void membuf_addu8(struct membuf_t *buf, uint8_t val);
void membuf_addbuf(struct membuf_t *restrict buf, const void *restrict data, uint64_t nbytes);

void membuf_addch(struct membuf_t *buf, char ch);
void membuf_addstr(struct membuf_t *restrict buf, const char *restrict str);

uint8_t membuf_last(struct membuf_t *restrict buf);
