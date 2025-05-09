#include "common.h"


/**
 * Create an attribute.
 *   @str: Consumed. The string.
 *   @child: Consumed. The children attributes.
 *   &returns: The attribute.
 */
struct el_attr_t *el_attr_new(char *str, struct el_attr_t *child)
{
	struct el_attr_t *attr;

	attr = malloc(sizeof(struct el_attr_t));
	attr->str = str;
	attr->next = NULL;
	attr->child = child;

	return attr;
}

/**
 * Copy a single attribute and its children.
 *   @attr: The attribute.
 *   &returns: The copy.
 */
struct el_attr_t *el_attr_copy(const struct el_attr_t *attr)
{
	return el_attr_new(strdup(attr->str), el_attr_clone(attr->child));
}

/**
 * Clone a list of attributes.
 *   @attr: The attribute list.
 *   &returns: The copiedl ist.
 */
struct el_attr_t *el_attr_clone(const struct el_attr_t *attr)
{
	struct el_attr_t *head = NULL, **iter = &head;

	while(attr != NULL) {
		*iter = el_attr_copy(attr);
		iter = &(*iter)->next;
		attr = attr->next;
	}

	return head;
}

/**
 * Delete a single attribute tree.
 *   @attr: The attribute.
 */
void el_attr_delete(struct el_attr_t *attr)
{
	el_attr_clear(attr->child);
	free(attr->str);
	free(attr);
}

/**
 * Clear an attribute list.
 *   @attr: The attribute.
 */
void el_attr_clear(struct el_attr_t *attr)
{
	struct el_attr_t *tmp;

	while(attr != NULL) {
		tmp = attr;
		attr = tmp->next;

		el_attr_delete(tmp);
	}
}


/**
 * Remove an attribute from a list.
 *   @attr: Ref. The attribute to remove.
 */
void el_attr_remove(struct el_attr_t **attr)
{
	struct el_attr_t *next = (*attr)->next;

	el_attr_delete(*attr);
	*attr = next;
}


/**
 * Apply the standard attributes to a declaration.
 *   @decl: The declaration.
 */
void el_attr_decl(struct el_decl_t *decl)
{
	struct el_attr_t **iter = &decl->attr;;

	while(*iter != NULL) {
		if(strcmp((*iter)->str, "public") == 0) {
			el_attr_remove(iter);
			decl->flags = (decl->flags & ~EL_VIS) | EL_PUB;
		}
		else if(strcmp((*iter)->str, "export") == 0) {
			el_attr_remove(iter);
			decl->flags = (decl->flags & ~EL_VIS) | EL_PUB | EL_EXP;
		}
		else if(strcmp((*iter)->str, "extern") == 0) {
			el_attr_remove(iter);
			decl->flags |= EL_EXTERN;
		}
		else {
			printf("unknown attr '%s'.", (*iter)->str);
			iter = &(*iter)->next;
		}
	}
}

/**
 * Apply the standard attributes to a declaration.
 *   @decl: The declaration.
 */
void el_attr_tydecl(struct el_tydecl_t *decl)
{
	struct el_attr_t **iter = &decl->attr;;

	while(*iter != NULL) {
		if(strcmp((*iter)->str, "public") == 0) {
			el_attr_remove(iter);
			decl->flags = (decl->flags & ~EL_VIS) | EL_PUB;
		}
		else if(strcmp((*iter)->str, "export") == 0) {
			el_attr_remove(iter);
			decl->flags = (decl->flags & ~EL_VIS) | EL_PUB | EL_EXP;
		}
		else {
			printf("unknown attr '%s'.", (*iter)->str);
			iter = &(*iter)->next;
		}
	}
}
