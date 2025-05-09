#pragma once

/*
 * parser declarations
 */
char *el_parse_path(struct el_unit_t *unit, const char *path);
char *el_parse_file(struct el_unit_t *unit, const char *path, FILE *file);
