#include "common.h"


/**
 * Create a new top-level unit.
 *   &returns: The top-level unit.
 */
struct el_top_t *el_top_new(void)
{
	struct el_top_t *top;

	top = malloc(sizeof(struct el_top_t));
	top->unit = NULL;

	return top;
}

/**
 * Delete a top-level unit.
 *   @top: The top-level unit.
 */
void el_top_delete(struct el_top_t *top)
{
	struct el_unit_t *unit;

	while(top->unit != NULL) {
		unit = top->unit;
		top->unit = unit->next;

		el_block_delete(unit->block);
		free(unit);
	}

	free(top);
}


/**
 * Create a code unit.
 *   @top: The top-level unit.
 *   &returns: The code unit.
 */
struct el_unit_t *el_top_unit(struct el_top_t *top)
{
	struct el_unit_t *unit;

	unit = malloc(sizeof(struct el_unit_t));
	unit->block = el_block_new(false);
	unit->next = top->unit;
	top->unit = unit;

	return unit;
}
