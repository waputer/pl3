#pragma once

/*
 * string declarations
 */
struct el_str_t *el_str_new(char *buf);
struct el_str_t *el_str_copy(struct el_str_t *str);
void el_str_delete(struct el_str_t *str);

/*
 * tag declarations
 */
struct el_tag_t *el_tag_new(struct el_str_t *file, uint32_t line, uint32_t col);
struct el_tag_t *el_tag_copy(const struct el_tag_t *tag);
void el_tag_delete(struct el_tag_t *tag);
void el_tag_erase(struct el_tag_t *tag);
