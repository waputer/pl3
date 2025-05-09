/**
 * Top level structure.
 *   @unit: The list of code units.
 */
tydef[export] st:pl3:top {
	pt:st:pl3:unit unit;
};

/**
 * Code unit structure.
 *   @stmt: The statement list.
 *   @next: The next code unit.
 */
tydef[export] st:pl3:unit {
	pt:st:pl3:stmt stmt;
	pt:st:pl3:unit next;
};


/**
 * Create a blank top-level.
 *   &returns: The top-level.
 */
def[export] pt:st:pl3:top() pl3:top_new = {
	return mem:make(st:pl3:top{ null });
};

/**
 * Delete a top-level.
 *   @top: The top-level.
 */
def[export] void(pt:st:pl3:top top) pl3:top_delete = {
	var pt:st:pl3:unit unit;

	while(top->unit != null) {
		top->unit = (unit = top->unit)->next;

		pl3:stmt_clear(unit->stmt);
		mem:free(unit);
	}

	mem:free(top);
};

/**
 * Add a code unit to the top-level.
 *   @top: The top-level.
 *   &returns: THe new code unit.
 */
def[export] pt:st:pl3:unit(pt:st:pl3:top top) pl3:top_unit = {
	return top->unit = mem:make(st:pl3:unit{ null, top->unit });
};
