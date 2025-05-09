#pragma once

/**
 * Top structure.
 *   @unit: The code unit list.
 */
struct el_top_t {
	struct el_unit_t *unit;
};

/**
 * Code unit structure.
 *   @block: The code block.
 *   @next: The next code unit.
 */
struct el_unit_t {
	struct el_block_t *block;
	struct el_unit_t *next;
};

/*
 * top-level declarations
 */
struct el_top_t *el_top_new(void);
void el_top_delete(struct el_top_t *top);

struct el_unit_t *el_top_unit(struct el_top_t *top);
