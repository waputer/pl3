#include "common.h"


/**
 * Create an expression.
 *   @kind: The kind.
 *   @data: Consumed. The data.
 *   @type: Consumed. The type.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_new(enum el_expr_e kind, union el_expr_u data, struct el_type_t type, struct el_tag_t *tag)
{
	struct el_expr_t *expr;

	expr = malloc(sizeof(struct el_expr_t));
	*expr = (struct el_expr_t){ kind, data, type, tag };

	return expr;
}

/**
 * Delete an expression.
 *   @expr: The expression.
 */
void el_expr_delete(struct el_expr_t *expr)
{
	switch(expr->kind) {
	case el_const_v: el_const_delete(expr->data.vconst); break;
	case el_body_v: el_body_delete(expr->data.body); break;
	case el_ident_v: el_ident_delete(expr->data.ident); break;
	case el_call_v: el_call_delete(expr->data.call); break;
	case el_addr_v: el_expr_delete(expr->data.op1); break;
	case el_elem_v: el_elem_delete(expr->data.elem); break;
	case el_deref_v: el_expr_delete(expr->data.op1); break;
	case el_pos_v: el_expr_delete(expr->data.op1); break;
	case el_neg_v: el_expr_delete(expr->data.op1); break;
	case el_not_v: el_expr_delete(expr->data.op1); break;
	case el_lnot_v: el_expr_delete(expr->data.op1); break;
	case el_preinc_v: el_expr_delete(expr->data.op1); break;
	case el_predec_v: el_expr_delete(expr->data.op1); break;
	case el_postinc_v: el_expr_delete(expr->data.op1); break;
	case el_postdec_v: el_expr_delete(expr->data.op1); break;
	case el_arrlen_v: el_expr_delete(expr->data.op1); break;
	case el_getref_v: el_expr_delete(expr->data.op1); break;
	case el_mul_v: el_op2_delete(expr->data.op2); break;
	case el_div_v: el_op2_delete(expr->data.op2); break;
	case el_rem_v: el_op2_delete(expr->data.op2); break;
	case el_add_v: el_op2_delete(expr->data.op2); break;
	case el_sub_v: el_op2_delete(expr->data.op2); break;
	case el_shl_v: el_op2_delete(expr->data.op2); break;
	case el_shr_v: el_op2_delete(expr->data.op2); break;
	case el_and_v: el_op2_delete(expr->data.op2); break;
	case el_xor_v: el_op2_delete(expr->data.op2); break;
	case el_or_v: el_op2_delete(expr->data.op2); break;
	case el_land_v: el_op2_delete(expr->data.op2); break;
	case el_lor_v: el_op2_delete(expr->data.op2); break;
	case el_eq_v: el_op2_delete(expr->data.op2); break;
	case el_ne_v: el_op2_delete(expr->data.op2); break;
	case el_gt_v: el_op2_delete(expr->data.op2); break;
	case el_gte_v: el_op2_delete(expr->data.op2); break;
	case el_lt_v: el_op2_delete(expr->data.op2); break;
	case el_lte_v: el_op2_delete(expr->data.op2); break;
	case el_assign_v: el_op2_delete(expr->data.op2); break;
	case el_addeq_v: el_op2_delete(expr->data.op2); break;
	case el_subeq_v: el_op2_delete(expr->data.op2); break;
	case el_muleq_v: el_op2_delete(expr->data.op2); break;
	case el_diveq_v: el_op2_delete(expr->data.op2); break;
	case el_remeq_v: el_op2_delete(expr->data.op2); break;
	case el_shleq_v: el_op2_delete(expr->data.op2); break;
	case el_shreq_v: el_op2_delete(expr->data.op2); break;
	case el_andeq_v: el_op2_delete(expr->data.op2); break;
	case el_xoreq_v: el_op2_delete(expr->data.op2); break;
	case el_oreq_v: el_op2_delete(expr->data.op2); break;
	case el_cast_v: el_expr_delete(expr->data.op1); break;
	case el_init_v: el_init_clear(expr->data.init); break;
	case el_sizeof_v: el_sizeof_delete(expr->data.size); break;
	case el_offsetof_v: el_offsetof_delete(expr->data.offset); break;
	case el_getparent_v: el_getparent_delete(expr->data.parent); break;
	case el_index_v: el_index_delete(expr->data.index); break;
	case el_initargs_v: break;
	case el_getarg_v: el_getarg_delete(expr->data.getarg); break;
	case el_eget_v: el_expr_delete(expr->data.op1); break;
	case el_ereq_v: el_expr_delete(expr->data.op1); break;
	case el_enval_v: el_enval_delete(expr->data.enval); break;
	case el_tern_v: el_tern_delete(expr->data.tern); break;
	}

	el_type_delete(expr->type);
	el_tag_erase(expr->tag);
	free(expr);
}


/**
 * Create a value expression.
 *   @value: Consumed. The value.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_const(struct el_const_t *vconst, struct el_tag_t *tag)
{
	return el_expr_new(el_const_v, (union el_expr_u){ .vconst = vconst }, el_type_void(), tag);
}

/**
 * Create a block expression.
 *   @block. Consumed. The block.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_block(struct el_block_t *block, struct el_tag_t *tag)
{
	return el_expr_new(el_block_v, (union el_expr_u){ .block = block }, el_type_void(), tag);
}

/**
 * Create a body expression.
 *   @body: Consumed. The function body.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_body(struct el_body_t *body, struct el_tag_t *tag)
{
	return el_expr_new(el_body_v, (union el_expr_u){ .body = body }, el_type_void(), tag);
}

/**
 * Create an identifier expression.
 *   @ident: Consumed. The identifier.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_ident(struct el_ident_t *ident, struct el_tag_t *tag)
{
	return el_expr_new(el_ident_v, (union el_expr_u){ .ident = ident }, el_type_void(), tag);
}

/**
 * Create a call expression.
 *   @call: Consumed. The call.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_call(struct el_call_t *call, struct el_tag_t *tag)
{
	return el_expr_new(el_call_v, (union el_expr_u){ .call = call }, el_type_void(), tag);
}

/**
 * Create a cast expression.
 *   @in: The input expression.
 *   @type: The type.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_cast(struct el_expr_t *in, struct el_type_t type, struct el_tag_t *tag)
{
	return el_expr_new(el_cast_v, (union el_expr_u){ .op1 = in }, type, tag);
}

/**
 * Create an address (get reference) expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_addr(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_addr_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create an dereference expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_deref(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_deref_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create a positive expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_pos(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_pos_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create a negation expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_neg(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_neg_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create a bitwise not expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_not(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_not_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create a logical not expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_lnot(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_lnot_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create a pre-increment expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_preinc(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_preinc_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create a pre-decrement expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_predec(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_predec_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create a post-increment expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_postinc(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_postinc_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create a post-decrement expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_postdec(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_postdec_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create an array length expression.
 *   @in: The input expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_arrlen(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_arrlen_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

struct el_expr_t *el_expr_getref(struct el_expr_t *in, struct el_tag_t *tag)
{
	return el_expr_new(el_getref_v, (union el_expr_u){ .op1 = in }, el_type_void(), tag);
}

/**
 * Create an add expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_add(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_add_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create a subtract expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_sub(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_sub_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create a multiply expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_mul(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_mul_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create a divide expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_div(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_div_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create a remainder expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_rem(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_rem_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an and expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_and(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_and_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an or expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_or(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_or_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}
struct el_expr_t *el_expr_land(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_land_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}
struct el_expr_t *el_expr_lor(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_lor_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}
struct el_expr_t *el_expr_shl(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_shl_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}
struct el_expr_t *el_expr_shr(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_shr_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an equality expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_eq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_eq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create a not-equal expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_ne(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_ne_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an greater-than expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_gt(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_gt_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an greater-than-or-equal expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_gte(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_gte_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an less-than expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_lt(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_lt_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an less-than-or-equal expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_lte(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_lte_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an xor expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_xor(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_xor_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an assign expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_assign(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_assign_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an add and assign expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_addeq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_addeq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an subtract and assign expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_subeq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_subeq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an multiply and assign expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_muleq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_muleq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an divide and assign expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_diveq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_diveq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an remainder and assign expression.
 *   @lhs: Consumed. The left-hand expression.
 *   @rhs: Consumed. The right-hand expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_remeq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_remeq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}
struct el_expr_t *el_expr_shleq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_shleq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}
struct el_expr_t *el_expr_shreq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_shreq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}
struct el_expr_t *el_expr_andeq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_andeq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}
struct el_expr_t *el_expr_oreq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_oreq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}
struct el_expr_t *el_expr_xoreq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag)
{
	return el_expr_new(el_xoreq_v, (union el_expr_u){ .op2 = { lhs, rhs } }, el_type_void(), tag);
}

/**
 * Create an initializer expression.
 *   @type: Consumed. The type.
 *   @init: Consumed. The initializer.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_init(struct el_type_t type, struct el_init_t *init, struct el_tag_t *tag)
{
	return el_expr_new(el_init_v, (union el_expr_u){ .init = init }, type, tag);
}

/**
 * Create a sizeof expression.
 *   @vsizeof: Consumed. The sizeof expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_sizeof(struct el_sizeof_t *size, struct el_tag_t *tag)
{
	return el_expr_new(el_sizeof_v, (union el_expr_u){ .size = size }, el_type_void(), tag);
}

/**
 * Create a offsetof expresoffset.
 *   @voffsetof: Consumed. The offsetof expresoffset.
 *   @tag: Consumed. The tag.
 *   &returns: The expresoffset.
 */
struct el_expr_t *el_expr_offsetof(struct el_offsetof_t *offset, struct el_tag_t *tag)
{
	return el_expr_new(el_offsetof_v, (union el_expr_u){ .offset = offset }, el_type_void(), tag);
}

/**
 * Create a getparent expresoffset.
 *   @voffsetof: Consumed. The offsetof expresoffset.
 *   @tag: Consumed. The tag.
 *   &returns: The expresoffset.
 */
struct el_expr_t *el_expr_getparent(struct el_getparent_t *parent, struct el_tag_t *tag)
{
	return el_expr_new(el_getparent_v, (union el_expr_u){ .parent = parent }, el_type_void(), tag);
}

/**
 * Create an element access expression.
 *   @elem: Consumed. The lement access.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_elem(struct el_elem_t *elem, struct el_tag_t *tag)
{
	return el_expr_new(el_elem_v, (union el_expr_u){ .elem = elem }, el_type_void(), tag);
}

/**
 * Create an index expression.
 *   @index: Consumed. The index expression.
 *   @tag: Consumed. The tag.
 *   &returns: The expression.
 */
struct el_expr_t *el_expr_index(struct el_index_t *index, struct el_tag_t *tag)
{
	return el_expr_new(el_index_v, (union el_expr_u){ .index = index }, el_type_void(), tag);
}

struct el_expr_t *el_expr_initargs(struct el_tag_t *tag)
{
	return el_expr_new(el_initargs_v, (union el_expr_u){  }, el_type_args(), tag);
}

struct el_expr_t *el_expr_getarg(struct el_getarg_t *get, struct el_tag_t *tag)
{
	return el_expr_new(el_getarg_v, (union el_expr_u){ .getarg = get }, el_type_copy(get->type), tag);
}

struct el_expr_t *el_expr_eget(struct el_expr_t *get, struct el_tag_t *tag)
{
	return el_expr_new(el_eget_v, (union el_expr_u){ .op1 = get }, el_type_void(), tag);
}

struct el_expr_t *el_expr_ereq(struct el_expr_t *get, struct el_tag_t *tag)
{
	return el_expr_new(el_ereq_v, (union el_expr_u){ .op1 = get }, el_type_void(), tag);
}

struct el_expr_t *el_expr_enval(struct el_enval_t *enval, struct el_tag_t *tag)
{
	return el_expr_new(el_enval_v, (union el_expr_u){ .enval = enval }, el_type_void(), tag);
}

struct el_expr_t *el_expr_tern(struct el_tern_t *tern, struct el_tag_t *tag)
{
	return el_expr_new(el_tern_v, (union el_expr_u){ .tern = tern }, el_type_void(), tag);
}


/**
 * Create a new identifier.
 *   @str: Consumed. The string ID.
 *   @type: The type.
 *   @data: The data.
 *   &returns: The identifier.
 */
struct el_ident_t *el_ident_new(char *str, enum el_ident_e type, union el_ident_u data)
{
	struct el_ident_t *ident;

	ident = malloc(sizeof(struct el_ident_t));
	ident->str = str;
	ident->type = type;
	ident->data = data;

	return ident;
}

/**
 * Delete an identifier.
 *   @ident: The identifier.
 */
void el_ident_delete(struct el_ident_t *ident)
{
	free(ident->str);
	free(ident);
}


/**
 * Create an unknown identifier.
 *   @str: Consumed. The string.
 *   &returns: The identifier.
 */
struct el_ident_t *el_ident_unk(char *str)
{
	return el_ident_new(str, el_unk_v, (union el_ident_u){ });
}

/**
 * Create a local identifier.
 *   @str: Consumed. The string.
 *   @decl: The declaration.
 *   &returns: The identifier.
 */
struct el_ident_t *el_ident_local(char *str, struct el_decl_t *decl)
{
	return el_ident_new(str, el_local_v, (union el_ident_u){ .decl = decl });
}

/**
 * Create a paramter identifier.
 *   @str: Consumed. The string.
 *   @off: The offset.
 *   &returns: The identifier.
 */
struct el_ident_t *el_ident_param(char *str, uint32_t off)
{
	return el_ident_new(str, el_param_v, (union el_ident_u){ .off = off });
}


/**
 * Create a call.
 *   @func: Consumed. The function expression.
 *   @deref: Dereference before calling.
 *   &returns: The call.
 */
struct el_call_t *el_call_new(struct el_expr_t *func, bool deref)
{
	struct el_call_t *call;

	call = malloc(sizeof(struct el_call_t));
	call->deref = deref;
	call->func = func;
	call->cnt = 0;
	call->arg = NULL;

	return call;
}

/**
 * Delete a call.
 *   @call: The call.
 */
void el_call_delete(struct el_call_t *call)
{
	struct el_arg_t *arg;

	while(call->arg != NULL) {
		arg = call->arg;
		call->arg = arg->next;

		el_expr_delete(arg->expr);
		free(arg);
	}

	el_expr_delete(call->func);
	free(call);
}


/**
 * Create an argument.
 *   @expr: Consumed. The expression.
 *   &returns: The argument.
 */
struct el_arg_t *el_arg_new(struct el_expr_t *expr)
{
	struct el_arg_t *arg;

	arg = malloc(sizeof(struct el_arg_t));
	arg->expr = expr;
	arg->next = NULL;

	return arg;
}


/**
 * Create a sizeof expression.
 *   @type: The type.
 *   @expr: Optional. The expression.
 *   &returns: The sizeof expression.
 */
struct el_sizeof_t *el_sizeof_new(struct el_type_t type, struct el_expr_t *expr)
{
	struct el_sizeof_t *size;

	size = malloc(sizeof(struct el_sizeof_t));
	size->type = type;
	size->expr = expr;

	return size;
}

/**
 * Delete a sizeof expression.
 *   @macro: The sizeof expression.
 */
void el_sizeof_delete(struct el_sizeof_t *size)
{
	if(size->expr != NULL)
		el_expr_delete(size->expr);

	el_type_delete(size->type);
	free(size);
}

struct el_offsetof_t *el_offsetof_new(struct el_type_t type, char *id)
{
	struct el_offsetof_t *offset;

	offset = malloc(sizeof(struct el_offsetof_t));
	offset->type = type;
	offset->id = id;

	return offset;
}
void el_offsetof_delete(struct el_offsetof_t *offset)
{
	el_type_delete(offset->type);
	free(offset->id);
	free(offset);
}


struct el_getparent_t *el_getparent_new(struct el_expr_t *expr, struct el_type_t type, char *id)
{
	struct el_getparent_t *parent;

	parent = malloc(sizeof(struct el_getparent_t));
	parent->expr = expr;
	parent->type = type;
	parent->id = id;

	return parent;
}

void el_getparent_delete(struct el_getparent_t *parent)
{
	el_expr_delete(parent->expr);
	el_type_delete(parent->type);
	free(parent->id);
	free(parent);
}


/**
 * Create a function body.
 *   @func: Consumed. The signature type.
 *   @block: Consumed. The block.
 *   &Returns: The body.
 */
struct el_body_t *el_body_new(struct el_func_t *func, struct el_block_t *block)
{
	struct el_body_t *body;

	body = malloc(sizeof(struct el_body_t));
	body->func = func;
	body->block = block;

	return body;
}

/**
 * Delete a function body.
 *   @body: The function body.
 */
void el_body_delete(struct el_body_t *body)
{
	el_func_delete(body->func);
	el_block_delete(body->block);
	free(body);
}


/**
 * Create an element access.
 *   @deref: The dereference flag.
 *   @base: The base expression.
 *   @id: The element identifier.
 *   &returns: The element access.
 */
struct el_elem_t *el_elem_new(bool deref, struct el_expr_t *base, char *id)
{
	struct el_elem_t *elem;

	elem = malloc(sizeof(struct el_elem_t));
	*elem = (struct el_elem_t){ deref, base, id, -1, -1 };

	return elem;
}

/**
 * Delete an element access.
 *   @elem: The element access.
 */
void el_elem_delete(struct el_elem_t *elem)
{
	el_expr_delete(elem->base);
	free(elem->id);
	free(elem);
}


/**
 * Create an index access.
 *   @deref: The derefence flag.
 *   @base: The base expression.
 *   @off: The offset expression.
 */
struct el_index_t *el_index_new(bool deref, struct el_expr_t *base, struct el_expr_t *off)
{
	struct el_index_t *index;

	index = malloc(sizeof(struct el_index_t));
	index->deref = deref;
	index->base = base;
	index->off = off;

	return index;
}

/**
 * Delete an index access.
 *   @index: The index access.
 */
void el_index_delete(struct el_index_t *index)
{
	el_expr_delete(index->base);
	el_expr_delete(index->off);
	free(index);
}


struct el_getarg_t *el_getarg_new(struct el_expr_t *expr, struct el_type_t type)
{
	struct el_getarg_t *get;

	get = malloc(sizeof(struct el_getarg_t));
	*get = (struct el_getarg_t){ expr, type };

	return get;
}

void el_getarg_delete(struct el_getarg_t *get)
{
	el_expr_delete(get->expr);
	el_type_delete(get->type);
	free(get);
}


struct el_enval_t *el_enval_new(char *type, char *repr)
{
	struct el_enval_t *val;

	val = malloc(sizeof(struct el_enval_t));
	val->type = type;
	val->repr = repr;

	return val;
}

struct el_enval_t *el_enval_const(char *type, int64_t ival)
{
	struct el_enval_t *val;

	val = malloc(sizeof(struct el_enval_t));
	val->type = type;
	val->repr = NULL;
	val->val = ival;

	return val;

}

void el_enval_delete(struct el_enval_t *val)
{
	if(val->repr != NULL)
		free(val->repr);

	free(val->type);
	free(val);
}


struct el_tern_t *el_tern_new(struct el_expr_t *cond, struct el_expr_t *ontrue, struct el_expr_t *onfalse)
{
	struct el_tern_t *tern;

	tern = malloc(sizeof(struct el_tern_t));
	*tern = (struct el_tern_t){ cond, ontrue, onfalse };

	return tern;
}

void el_tern_delete(struct el_tern_t *tern)
{
	el_expr_delete(tern->cond);
	el_expr_delete(tern->ontrue);
	el_expr_delete(tern->onfalse);
	free(tern);
}
