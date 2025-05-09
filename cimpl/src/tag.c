#include "common.h"


/**
 * Create a string.
 *   @buf: Consumed. The character buffer.
 *   &returns: The string.
 */
struct el_str_t *el_str_new(char *buf)
{
	struct el_str_t *str;

	str = malloc(sizeof(struct el_str_t));
	*str = (struct el_str_t){ buf, 1 };

	return str;
}

/**
 * Copy a string.
 *   @str: The string.
 *   &returns: The copy.
 */
struct el_str_t *el_str_copy(struct el_str_t *str)
{
	str->refcnt++;

	return str;
}

/**
 * Delete a string, freeing only if there are no more references.
 *   @str: The string.
 */
void el_str_delete(struct el_str_t *str)
{
	if(str->refcnt <= 1) {
		free(str->buf);
		free(str);
	}
	else
		str->refcnt--;
}


/**
 * Create a tag.
 *   @file: Consumed. The file.
 *   @line: The line.
 *   @col: The column.
 *   &returns: The tag.
 */
struct el_tag_t *el_tag_new(struct el_str_t *file, uint32_t line, uint32_t col)
{
	struct el_tag_t *tag;

	tag = malloc(sizeof(struct el_tag_t));
	*tag = (struct el_tag_t){ file, line, col };

	return tag;
}

/**
 * Copy a tag.
 *   @tag: The tag.
 *   &returns: The copy.
 */
struct el_tag_t *el_tag_copy(const struct el_tag_t *tag)
{
	return el_tag_new(el_str_copy(tag->file), tag->line, tag->col);
}

/**
 * Delete a tag.
 *   @tag: The tag.
 */
void el_tag_delete(struct el_tag_t *tag)
{
	el_str_delete(tag->file);
	free(tag);
}

/**
 * Erase a tag if not null.
 *   @tag: The tag.
 */
void el_tag_erase(struct el_tag_t *tag)
{
	if(tag != NULL)
		el_tag_delete(tag);
}
