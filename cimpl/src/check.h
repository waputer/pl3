#pragma once

void el_chk_top(struct el_top_t *top, struct el_ctx_t *ctx);

/*
 * typecheck declarations
 */
void el_check_top(struct el_top_t *top, struct el_ctx_t *ctx);

char *el_error(struct el_tag_t *tag, const char *fmt, ...);

/*
 * custom checker declarations
 */

struct el_check_t el_check_init(void *ref, const struct el_check_i *iface);
struct el_check_t el_check_copy(struct el_check_t check);
void el_check_delete(struct el_check_t check);
char *el_check_proc(struct el_check_t check, struct el_func_t *func, struct el_arg_t *arg);

struct el_check_t el_check_none(void);
struct el_check_t el_check_syscall(void);
