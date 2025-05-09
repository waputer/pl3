#pragma once

/*
 * llvm function declarations
 */
char *el_llvm_save(struct el_top_t *top, const char *path);
char *el_llvm_write(struct el_top_t *top, FILE *file);
