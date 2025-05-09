#include "common.h"


/**
 * Variable structure.
 *   @decl: The variable declaration.
 *   @node: The node.
 */
struct var_t {
	struct el_decl_t *decl;
	struct avl_node_t node;
};

/**
 * Type declaration structure.
 *   @tydef: The type defintion.
 *   @node: The node.
 */
struct type_t {
	struct el_tydecl_t *tydecl;
	struct avl_node_t node;
};


/**
 * Create a scope.
 *   @parent: Optional. The parent scope.
 *   &returns: The scope.
 */
struct el_scope_t *el_scope_new(struct el_scope_t *parent)
{
	struct el_scope_t *scope;

	scope = malloc(sizeof(struct el_scope_t));
	scope->parent = parent;
	scope->vars = avl_root_init(avl_cmp_str);
	scope->types = avl_root_init(avl_cmp_str);

	return scope;
}

/**
 * Delete a scope.
 *   @scope: The scope.
 */
void el_scope_delete(struct el_scope_t *scope)
{
	avl_root_destroy(&scope->vars, offsetof(struct var_t, node), free);
	avl_root_destroy(&scope->types, offsetof(struct type_t, node), free);
	free(scope);
}


/**
 * Add a variable to the scope.
 *   @scope: The scope.
 *   @ref: The reference.
 */
void el_scope_var_add(struct el_scope_t *scope, struct el_decl_t *decl)
{
	struct var_t *var;

	var = malloc(sizeof(struct var_t));
	var->decl = decl;
	avl_node_init(&var->node, decl->id);
	avl_root_add(&scope->vars, &var->node);
}

/**
 * Find a variable in the scope.
 *   @scope: The scope.
 *   @id: The variable identifier.
 *   &returns: The declaration or null.
 */
struct el_decl_t *el_scope_var_find(struct el_scope_t *scope, const char *id)
{
	struct avl_node_t *node;

	while(scope != NULL) {
		node = avl_root_get(&scope->vars, id);
		if(node != NULL)
			return getparent(node, struct var_t, node)->decl;

		scope = scope->parent;
	}

	return NULL;
}


/**
 * Add a type to the scope.
 *   @scope: The scope.
 *   @ref: The reference.
 */
void el_scope_type_add(struct el_scope_t *scope, struct el_tydecl_t *tydecl)
{
	struct type_t *type;

	type = malloc(sizeof(struct type_t));
	type->tydecl = tydecl;
	avl_node_init(&type->node, tydecl->id);
	avl_root_add(&scope->types, &type->node);
}

/**
 * Find a type in the scope.
 *   @scope: The scope.
 *   @id: The type identifier.
 *   &returns: The declaration or null.
 */
struct el_tydecl_t *el_scope_type_find(struct el_scope_t *scope, const char *id)
{
	struct avl_node_t *node;

	while(scope != NULL) {
		node = avl_root_get(&scope->types, id);
		if(node != NULL)
			return getparent(node, struct type_t, node)->tydecl;

		scope = scope->parent;
	}

	return NULL;
}


/**
 * Create a type declaration.
 *   @id: The identifier.
 *   @flags: The flags.
 *   @type: Consumed. The type.
 *   &returns: The declaration.
 */
struct el_tydecl_t *el_tydecl_new(const char *id, uint32_t flags, struct el_type_t type)
{
	struct el_tydecl_t *decl;

	decl = malloc(sizeof(struct el_tydecl_t));
	decl->id = id;
	decl->flags = flags;
	decl->type = type;
	decl->attr = NULL;
	avl_node_init(&decl->node, id);

	return decl;
}

/**
 * Delete a type declaration.
 *   @decl: The declaration.
 */
void el_tydecl_delete(struct el_tydecl_t *decl)
{
	el_type_delete(decl->type);
	el_attr_clear(decl->attr);
	free(decl);
}
