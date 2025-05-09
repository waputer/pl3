#pragma once

/**
 * Statement type enumerator.
 *   @el_decl_v: Declaration.
 *   @el_tydecl_v: Type declaration.
 *   @el_expr_v: Expression.
 *   @el_block_v: Block.
 *   @el_return_v: Return.
 *   @el_eret_v: Error and return.
 *   @el_block_v: Scope.
 *   @el_cond_v: Conditional.
 */
enum el_stmt_e {
	el_decl_v,
	el_tydecl_v,
	el_expr_v,
	el_block_v,
	el_return_v,
	el_eret_v,
	el_cond_v,
	el_loop_v,
	el_brk_v,
	el_cont_v,
	el_switch_v,
	el_label_v,
	el_onerr_v
};

/**
 * Statement data union.
 *   @decl: Declaration.
 *   @tydecl: Type declaration.
 *   @expr: Expression.
 *   @block: Nested block.
 *   @cond: The condition.
 */
union el_stmt_u {
	uint32_t u32;
	struct el_decl_t *decl;
	struct el_tydecl_t *tydecl;
	struct el_expr_t *expr;
	struct el_block_t *block;
	struct el_cond_t *cond;
	struct el_loop_t *loop;
	struct el_switch_t *sw;
	struct el_label_t *label;
};

/**
 * Statement structure.
 *   @type: The type.
 *   @data: The data.
 *   @next: The next statement.
 *   @tag: The tag.
 */
struct el_stmt_t {
	enum el_stmt_e type;
	union el_stmt_u data;

	struct el_stmt_t *next;
	struct el_tag_t *tag;
};

/*
 * statement declarations
 */
struct el_stmt_t *el_stmt_new(enum el_stmt_e type, union el_stmt_u data, struct el_tag_t *tag);
void el_stmt_delete(struct el_stmt_t *stmt);

struct el_stmt_t *el_stmt_decl(struct el_decl_t *decl, struct el_tag_t *tag);
struct el_stmt_t *el_stmt_tydecl(struct el_tydecl_t *tydecl, struct el_tag_t *tag);
struct el_stmt_t *el_stmt_expr(struct el_expr_t *expr, struct el_tag_t *tag);
struct el_stmt_t *el_stmt_block(struct el_block_t *block, struct el_tag_t *tag);
struct el_stmt_t *el_stmt_onerr(struct el_block_t *block, struct el_tag_t *tag);
struct el_stmt_t *el_stmt_return(struct el_expr_t *expr, struct el_tag_t *tag);
struct el_stmt_t *el_stmt_eret(struct el_expr_t *expr, struct el_tag_t *tag);
struct el_stmt_t *el_stmt_cond(struct el_cond_t *cond);
struct el_stmt_t *el_stmt_loop(struct el_loop_t *loop);
struct el_stmt_t *el_stmt_block(struct el_block_t *block, struct el_tag_t *tag);
struct el_stmt_t *el_stmt_brk(struct el_tag_t *tag);
struct el_stmt_t *el_stmt_cont(struct el_tag_t *tag);
struct el_stmt_t *el_stmt_switch(struct el_switch_t *sw, struct el_tag_t *tag);
struct el_stmt_t *el_stmt_label(struct el_label_t *label, struct el_tag_t *tag);

/**
 * Declaration structure.
 *   @id: The identifier.
 *   @flags: The flags.
 *   @attr: The attribute list.
 *   @type: The type.
 *   @expr: The expression.
 *   @tag: The tag.
 *   @node: The tree node.
 */
struct el_decl_t {
	char *id;
	uint32_t flags;
	struct el_attr_t *attr;

	struct el_type_t type;
	struct el_expr_t *expr;
	struct el_tag_t *tag;
};

/*
 * declaration flags
 */
#define EL_MUT    0x01
#define EL_AUTO   0x02
#define EL_PRIV   0x00
#define EL_PUB    0x10
#define EL_EXP    0x20
#define EL_VIS    0x30
#define EL_GLOB   0x40
#define EL_EXTERN 0x80

/*
 * declaration declarations
 */
struct el_decl_t *el_decl_new(char *id, uint32_t flags, struct el_type_t type, struct el_expr_t *expr, struct el_tag_t *tag);
void el_decl_delete(struct el_decl_t *decl);


/**
 * Block structure.
 *   @esc: Escapable flag.
 *   @head, tail: The head and tail statements.
 *   @enter, leave, error, clean: The entry and exit blocks.
 *   @up: The previous block.
 */
struct el_block_t {
	bool esc;
	struct el_stmt_t *head, **tail;

	struct el_block_t *enter, *leave, *error, *clean;
	struct el_block_t *up;
};

/*
 * block declarations
 */
struct el_block_t *el_block_new(bool esc);
void el_block_delete(struct el_block_t *block);

void el_block_add(struct el_block_t *block, struct el_stmt_t *stmt);

struct el_block_t *el_block_enter(struct el_block_t *block);


/**
 * Conditional structure.
 *   @eval: The evaluated expression.
 *   @ontrue, onfalse: The true and false expression.
 *   @tag: The tag.
 */
struct el_cond_t {
	struct el_expr_t *eval;
	struct el_block_t *ontrue, *onfalse;
	struct el_tag_t *tag;
};

/*
 * condition declarations
 */
struct el_cond_t *el_cond_new(struct el_expr_t *eval, struct el_block_t *ontrue, struct el_block_t *onfalse, struct el_tag_t *tag);
void el_cond_delete(struct el_cond_t *cond);
void el_loop_delete(struct el_loop_t *loop);


/**
 * Loop structure.
 *   @post: Post-test flag.
 *   @init, cond, inc: The initialize, condition, and increment expressions.
 *   @body: The body block.
 *   @tag: The tag.
 */
struct el_loop_t {
	bool post;
	struct el_expr_t *init, *cond, *inc;
	struct el_block_t *body;
	struct el_tag_t *tag;
};

/*
 * loop declarations
 */
struct el_loop_t *el_loop_new(bool post, struct el_expr_t *init, struct el_expr_t *cond, struct el_expr_t *inc, struct el_block_t *body, struct el_tag_t *tag);
void el_loop_delete(struct el_loop_t *loop);


/**
 * Switch structure.
 *   @eval: The evaluator.
 *   @vcase: The case list.
 *   @tag: The tag.
 */
struct el_switch_t {
	struct el_expr_t *eval;
	struct el_case_t *vcase;
	struct el_block_t *block;

	struct el_tag_t *tag;
};

/**
 * Case structure.
 *   @expr; The expression.
 *   @label: The target label.
 *   @tag: The tag.
 */
struct el_case_t {
	struct el_expr_t *expr;
	struct el_label_t *label;
	struct el_case_t *next;

	struct el_tag_t *tag;
};

/*
 * switch declarations
 */
struct el_switch_t *el_switch_new(struct el_expr_t *eval, struct el_tag_t *tag);
void el_switch_delete(struct el_switch_t *sw);

struct el_case_t *el_case_new(struct el_expr_t *expr, struct el_label_t *label, struct el_tag_t *tag);
void el_case_delete(struct el_case_t *vcase);


/**
 * Label structure.
 *   @id: The identifier.
 *   @tag: The tag.
 */
struct el_label_t {
	char *id;
	struct el_tag_t *tag;
};

/*
 * label declarations
 */
struct el_label_t *el_label_new(char *id, struct el_tag_t *tag);
void el_label_delete(struct el_label_t *label);


/**
 * Block link.
 *   @block: The block.
 *   @prev: The previous link.
 */
struct el_link_t {
	struct el_block_t *block;
	struct el_link_t *prev;
};

/*
 * link declarations
 */
struct el_link_t *el_link_new(struct el_block_t *block, struct el_link_t *prev);
void el_link_delete(struct el_link_t *link);

struct el_link_t *el_link_clone(struct el_link_t *link);
void el_link_clear(struct el_link_t *link);
