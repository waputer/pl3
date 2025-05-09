#pragma once

/**
 * Attribute structure.
 *   @str: The string.
 *   @next, child: The next and child attributes.
 */
struct el_attr_t {
	char *str;
	struct el_attr_t *next, *child;
};

/*
 * attribute declarations
 */
struct el_attr_t *el_attr_new(char *str, struct el_attr_t *child);
struct el_attr_t *el_attr_copy(const struct el_attr_t *attr);
struct el_attr_t *el_attr_clone(const struct el_attr_t *attr);
void el_attr_delete(struct el_attr_t *attr);
void el_attr_clear(struct el_attr_t *attr);

void el_attr_remove(struct el_attr_t **attr);

void el_attr_decl(struct el_decl_t *decl);
void el_attr_tydecl(struct el_tydecl_t *decl);
