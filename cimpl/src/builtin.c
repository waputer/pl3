#include "common.h"


struct el_builtin_t el_builtins[] = {
	{ "bi:syscall", el_syscall_type },
	{ NULL,         NULL }
};


/**
 * Find a builtin by ID.
 *   @id: The identifier.
 *   &returns: The builtin.
 */
struct el_builtin_t *el_builtin_find(const char *id)
{
	uint32_t i;

	for(i = 0; el_builtins[i].id != NULL; i++) {
		if(strcmp(el_builtins[i].id, id) == 0)
			return &el_builtins[i];
	}

	return NULL;
}

struct el_type_t el_syscall_type(void)
{
	uint32_t i;
	struct el_func_t *func;

	func = el_func_new(el_type_i64());
	func->check = el_check_syscall();

	for(i = 0; i < 7; i++)
		el_func_append(func, el_type_i64(), NULL);

	return el_type_func(func);
}
