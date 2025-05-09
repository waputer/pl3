#pragma once

/**
 * Builtin structure.
 *   @id: The identifier.
 *   @func: The type function.
 */
struct el_builtin_t {
	char *id;
	struct el_type_t (*func)(void);
};

/*
 * builtins declarations
 */
struct el_builtin_t *el_builtin_find(const char *id);

struct el_type_t el_syscall_type(void);
