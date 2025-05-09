#pragma once

struct el_ctx_t;
struct el_tag_t;

void el_ctx_error(struct el_ctx_t *ctx, struct el_tag_t *tag, const char *restrict fmt, ...);
