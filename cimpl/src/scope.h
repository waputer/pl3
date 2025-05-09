#pragma once


/**
 * Scope structure.
 *   @vars, types: The variable and type trees.
 *   @parent: The parent scope.
 */
struct el_scope_t {
	struct avl_root_t vars, types;

	struct el_scope_t *parent;
};

/*
 * scope declarations
 */
struct el_scope_t *el_scope_new(struct el_scope_t *parent);
void el_scope_delete(struct el_scope_t *scope);

void el_scope_var_add(struct el_scope_t *scope, struct el_decl_t *decl);
struct el_decl_t *el_scope_var_find(struct el_scope_t *scope, const char *id);

void el_scope_type_add(struct el_scope_t *scope, struct el_tydecl_t *tydecl);
struct el_tydecl_t *el_scope_type_find(struct el_scope_t *scope, const char *id);


/**
 * Type declaration structure.
 *   @flags: Flags.
 *   @id: The identifier.
 *   @type: The type.
 *   @node: The node.
 */
struct el_tydecl_t {
	uint32_t flags;
	const char *id;
	struct el_type_t type;

	struct el_attr_t *attr;
	struct avl_node_t node;
};

/*
 * type declarations
 */
struct el_tydecl_t *el_tydecl_new(const char *id, uint32_t flags, struct el_type_t type);
void el_tydecl_delete(struct el_tydecl_t *decl);
