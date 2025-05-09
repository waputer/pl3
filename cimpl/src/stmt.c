#include "common.h"


/**
 * Create a statement.
 *   @type: The type.
 *   @data: Consumed. The data.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_new(enum el_stmt_e type, union el_stmt_u data, struct el_tag_t *tag)
{
	struct el_stmt_t *stmt;

	stmt = malloc(sizeof(struct el_stmt_t));
	stmt->type = type;
	stmt->data = data;
	stmt->tag = tag;
	stmt->next = NULL;

	return stmt;
}

/**
 * Delete a statement.
 *   @stmt: The statement.
 */
void el_stmt_delete(struct el_stmt_t *stmt)
{
	switch(stmt->type) {
	case el_decl_v: el_decl_delete(stmt->data.decl); break;
	case el_tydecl_v: el_tydecl_delete(stmt->data.tydecl); break;
	case el_expr_v: el_expr_delete(stmt->data.expr); break;
	case el_onerr_v: el_block_delete(stmt->data.block); break;
	case el_block_v: el_block_delete(stmt->data.block); break;
	case el_return_v: if(stmt->data.expr != NULL) el_expr_delete(stmt->data.expr); break;
	case el_eret_v: if(stmt->data.expr != NULL) el_expr_delete(stmt->data.expr); break;
	case el_cond_v: el_cond_delete(stmt->data.cond); break;
	case el_loop_v: el_loop_delete(stmt->data.loop); break;
	case el_brk_v: break;
	case el_cont_v: break;
	case el_switch_v: el_switch_delete(stmt->data.sw); break;
	case el_label_v: el_label_delete(stmt->data.label); break;
	}

	el_tag_delete(stmt->tag);
	free(stmt);
}


/**
 * Create a declaration statement.
 *   @decl: Consumed. The declaration.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_decl(struct el_decl_t *decl, struct el_tag_t *tag)
{
	return el_stmt_new(el_decl_v, (union el_stmt_u){ .decl = decl }, tag);
}

/**
 * Create a type declaration statement.
 *   @tydecl: Consumed. The type declaration.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_tydecl(struct el_tydecl_t *tydecl, struct el_tag_t *tag)
{
	return el_stmt_new(el_tydecl_v, (union el_stmt_u){ .tydecl = tydecl }, tag);
}

/**
 * Create an expression statement.
 *   @expr: Consumed. The expression.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_expr(struct el_expr_t *expr, struct el_tag_t *tag)
{
	return el_stmt_new(el_expr_v, (union el_stmt_u){ .expr = expr }, tag);
}

/**
 * Create a block statement.
 *   @block: Consumed. The block.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_block(struct el_block_t *block, struct el_tag_t *tag)
{
	return el_stmt_new(el_block_v, (union el_stmt_u){ .block = block }, tag);
}

/**
 * Create a on error statement.
 *   @block: Consumed. The block.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_onerr(struct el_block_t *block, struct el_tag_t *tag)
{
	return el_stmt_new(el_onerr_v, (union el_stmt_u){ .block = block }, tag);
}

/**
 * Create a return statement.
 *   @expr: Optional. Consumed. The expression.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_return(struct el_expr_t *expr, struct el_tag_t *tag)
{
	return el_stmt_new(el_return_v, (union el_stmt_u){ .expr = expr }, tag);
}

/**
 * Create an error and return statement.
 *   @expr: Optional. Consumed. The expression.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_eret(struct el_expr_t *expr, struct el_tag_t *tag)
{
	return el_stmt_new(el_eret_v, (union el_stmt_u){ .expr = expr }, tag);
}

/**
 * Create a conditional statement.
 *   @cond: Consumed. The conditional.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_cond(struct el_cond_t *cond)
{
	return el_stmt_new(el_cond_v, (union el_stmt_u){ .cond = cond }, el_tag_copy(cond->tag));
}

/**
 * Create a loop statement.
 *   @loop: Consumed. The loop.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_loop(struct el_loop_t *loop)
{
	return el_stmt_new(el_loop_v, (union el_stmt_u){ .loop = loop }, el_tag_copy(loop->tag));
}

/**
 * Create a break statement.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_brk(struct el_tag_t *tag)
{
	return el_stmt_new(el_brk_v, (union el_stmt_u){ }, tag);
}

/**
 * Create a continue statement.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_cont(struct el_tag_t *tag)
{
	return el_stmt_new(el_cont_v, (union el_stmt_u){ }, tag);
}

/**
 * Create a switch statement.
 *   @sw: Consumed. The switch statement.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_switch(struct el_switch_t *sw, struct el_tag_t *tag)
{
	return el_stmt_new(el_switch_v, (union el_stmt_u){ .sw = sw }, tag);
}

/**
 * Create a label statement.
 *   @sw: Consumed. The label statement.
 *   @tag: Consumed. The tag.
 *   &returns: The statement.
 */
struct el_stmt_t *el_stmt_label(struct el_label_t *label, struct el_tag_t *tag)
{
	return el_stmt_new(el_label_v, (union el_stmt_u){ .label = label }, tag);
}


/**
 * Create a new block.
 *   @esc: The escapable flag.
 *   &returns: The block.
 */
struct el_block_t *el_block_new(bool esc)
{
	struct el_block_t *block;

	block = malloc(sizeof(struct el_block_t));
	block->esc = esc;
	block->head = NULL;
	block->tail = &block->head;
	block->enter = block->leave = block->error = block->clean = NULL;
	block->up = NULL;

	return block;
}

/**
 * Delete a block.
 *   @block: The block.
 */
void el_block_delete(struct el_block_t *block)
{
	struct el_stmt_t *stmt;

	while(block->head != NULL) {
		stmt = block->head;
		block->head = stmt->next;
		el_stmt_delete(stmt);
	}

	if(block->enter != NULL)
		el_block_delete(block->enter);

	if(block->leave != NULL)
		el_block_delete(block->leave);

	if(block->error != NULL)
		el_block_delete(block->error);

	if(block->clean != NULL)
		el_block_delete(block->clean);

	free(block);
}


/**
 * Retrieve the enter block, creating it if needed.
 *   @block: The block.
 *   &returns: The enter block.
 */
struct el_block_t *el_block_enter(struct el_block_t *block)
{
	if(block->enter == NULL)
		block->enter = el_block_new(false);

	return block->enter;
}


/**
 * Add a statement to the block.
 *   @block: The block.
 *   @stmt: The statement.
 */
void el_block_add(struct el_block_t *block, struct el_stmt_t *stmt)
{
	*block->tail = stmt;
	block->tail = &stmt->next;
}


/**
 * Create a conditional.
 *   @eval: Consumed. The evalution.
 *   @ontrue: Consumed. Optional. The true block
 *   @onfalse: Consumed. Optional. The false block.
 *   @tag: Consumed. The tag.
 */
struct el_cond_t *el_cond_new(struct el_expr_t *eval, struct el_block_t *ontrue, struct el_block_t *onfalse, struct el_tag_t *tag)
{
	struct el_cond_t *cond;

	cond = malloc(sizeof(struct el_cond_t));
	cond->eval = eval;
	cond->ontrue = ontrue;
	cond->onfalse = onfalse;
	cond->tag = tag;

	return cond;
}

/**
 * Delete a conditional.
 *   @cond: The conditional.
 */
void el_cond_delete(struct el_cond_t *cond)
{
	if(cond->ontrue != NULL)
		el_block_delete(cond->ontrue);

	if(cond->onfalse != NULL)
		el_block_delete(cond->onfalse);

	el_expr_delete(cond->eval);
	el_tag_delete(cond->tag);
	free(cond);
}


/**
 * Create a declaration.
 *   @id: Consumed. The identifier.
 *   @flags: The flags.
 *   @type: Consumed. The type.
 *   @expr: Optional. Consumed. The expression.
 *   @tag: Optional. Consumed. The tag.
 *   &returns: The declaration.
 */
struct el_decl_t *el_decl_new(char *id, uint32_t flags, struct el_type_t type, struct el_expr_t *expr, struct el_tag_t *tag)
{
	struct el_decl_t *decl;

	decl = malloc(sizeof(struct el_decl_t));
	decl->id = id;
	decl->flags = flags;
	decl->attr = NULL;
	decl->type = type;
	decl->expr = expr;
	decl->tag = tag;

	return decl;
}

/**
 * Delete a declaration.
 *   @decl: The declaration.
 */
void el_decl_delete(struct el_decl_t *decl)
{
	if(decl->expr != NULL)
		el_expr_delete(decl->expr);

	el_tag_erase(decl->tag);
	el_type_delete(decl->type);
	el_attr_clear(decl->attr);
	free(decl->id);
	free(decl);
}


/**
 * Create a loop.
 *   @post: The post-test flag.
 *   @init: Optional. Consumed. The initializer expression.
 *   @cond: Optional. Consumed. The conditional expression.
 *   @inc: Optional. Consumed. The increment expression.
 *   @body: Consumed. The body statement.
 *   @tag; Consumed. The tag.
 *   &returns: The loop.
 */
struct el_loop_t *el_loop_new(bool post, struct el_expr_t *init, struct el_expr_t *cond, struct el_expr_t *inc, struct el_block_t *body, struct el_tag_t *tag)
{
	struct el_loop_t *loop;

	loop = malloc(sizeof(struct el_loop_t));
	loop->post = post;
	loop->init = init;
	loop->cond = cond;
	loop->inc = inc;
	loop->body = body;
	loop->tag = tag;

	return loop;
}

/**
 * Delete a loop.
 *   @loop: The loop.
 */
void el_loop_delete(struct el_loop_t *loop)
{
	if(loop->init != NULL)
		el_expr_delete(loop->init);

	if(loop->cond != NULL)
		el_expr_delete(loop->cond);

	if(loop->inc != NULL)
		el_expr_delete(loop->inc);

	el_block_delete(loop->body);
	el_tag_delete(loop->tag);
	free(loop);
}


/**
 * Create a new switch.
 *   @eval: Consumed. The evaluated expression.
 *   @tag: Consumed. The tag.
 *   &returns: The switch.
 */
struct el_switch_t *el_switch_new(struct el_expr_t *eval, struct el_tag_t *tag)
{
	struct el_switch_t *sw;
	
	sw = malloc(sizeof(struct el_switch_t));
	*sw = (struct el_switch_t){ eval, NULL, NULL, tag };

	return sw;
}

/**
 * Delete a switch.
 *   @switch: The switch.
 */
void el_switch_delete(struct el_switch_t *sw)
{
	struct el_case_t *vcase;

	while(sw->vcase != NULL) {
		sw->vcase = (vcase = sw->vcase)->next;
		el_case_delete(vcase);
	}

	if(sw->block != NULL)
		el_block_delete(sw->block);

	el_expr_delete(sw->eval);
	el_tag_delete(sw->tag);
	free(sw);
}


/**
 * Create a new case.
 *   @expr: Consumed. The expression.
 *   @label: The target label.
 *   @tag: Consumed. The tag.
 *   &returns: The case.
 */
struct el_case_t *el_case_new(struct el_expr_t *expr, struct el_label_t *label, struct el_tag_t *tag)
{
	struct el_case_t *vcase;
	
	vcase = malloc(sizeof(struct el_case_t));
	*vcase = (struct el_case_t){ expr, label, NULL, tag };

	return vcase;
}

/**
 * Delete a case.
 *   @case: The case.
 */
void el_case_delete(struct el_case_t *vcase)
{
	if(vcase->expr != NULL)
		el_expr_delete(vcase->expr);

	el_tag_delete(vcase->tag);
	free(vcase);
}


/**
 * Create a new label.
 *   @id: Consumed. Optional. The identifier.
 *   @tag: Consumed. The tag.
 *   &returns: The label.
 */
struct el_label_t *el_label_new(char *id, struct el_tag_t *tag)
{
	struct el_label_t *label;
	
	label = malloc(sizeof(struct el_label_t));
	*label = (struct el_label_t){ id, tag };

	return label;
}

/**
 * Delete a label.
 *   @label: The label.
 */
void el_label_delete(struct el_label_t *label)
{
	if(label->id != NULL)
		free(label->id);

	el_tag_delete(label->tag);
	free(label);
}


/**
 * Create a single link.
 *   @block: The block.
 *   @prev: The previous link.
 *   &returns: The link.
 */
struct el_link_t *el_link_new(struct el_block_t *block, struct el_link_t *prev)
{
	struct el_link_t *link;

	link = malloc(sizeof(struct el_link_t));
	link->block = block;
	link->prev = prev;

	return link;
}

/**
 * Delete a single link.
 *   @link: The link.
 */
void el_link_delete(struct el_link_t *link)
{
	free(link);
}


/**
 * Clone an entire list of links.
 *   @link: The head link.
 *   &returns: The cloned list.
 */
struct el_link_t *el_link_clone(struct el_link_t *link)
{
	struct el_link_t *head = NULL, **cur = &head, *last = NULL;

	while(link != NULL) {
		*cur = el_link_new(link->block, last);
		last = *cur;
		cur = &(*cur)->prev;
	}

	return head;
}

/**
 * Clear a list of links.
 *   @link: The link.
 */
void el_link_clear(struct el_link_t *link)
{
	struct el_link_t *tmp;

	while(link != NULL) {
		tmp = link;
		link = tmp->prev;
		el_link_delete(tmp);
	}
}
