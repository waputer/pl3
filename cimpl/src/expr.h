#pragma once

/**
 * Element access structure.
 *   @deref: Dereference flag.
 *   @base: The base expression.
 *   @id: The identifier.
 *   @idx, off: The index and offset.
 */
struct el_elem_t {
	bool deref;

	struct el_expr_t *base;
	char *id;
	int32_t idx, off;
};

/**
 * Function body structure.
 *   @func: The function signature.
 *   @block: The code block.
 */
struct el_body_t {
	struct el_func_t *func;
	struct el_block_t *block;
};

/**
 * Two-operand structure.
 *   @lhs, rhs: The left and right hand sides.
 */
struct el_op2_t {
	struct el_expr_t *lhs, *rhs;
};

struct el_getarg_t {
	struct el_expr_t *expr;
	struct el_type_t type;
};


/**
 * Expression kind enumerator.
 *   @el_const_v: Value.
 *   @el_ident_v: Identifier.
 *   @el_call_v: Call.
 *   @el_pos_v: Positive unary.
 *   @el_neg_v: Negative unary.
 *   @el_not_v: Bitwise not unary.
 *   @el_lnot_v: Logical not unary.
 *   @el_add_v: Add.
 *   @el_sub_v: Subtract.
 *   @el_mul_v: Multiply.
 *   @el_div_v: Divide.
 *   @el_rem_v: Remainder.
 *   @el_shl_v: Shift left.
 *   @el_shr_v: Shift right.
 *   @el_and_v: Bitwise and.
 *   @el_xor_v: BItwise xor.
 *   @el_or_v: Bitwise or.
 *   @el_land_v: Logical and.
 *   @el_lor_v: Logical or.
 *   @el_eq_v: Equal.
 *   @el_neq_v: Not equal.
 *   @el_gt_v: Greater-than
 *   @el_gte_v: Greather-than-or-equal
 *   @el_lt_v: Less-than.
 *   @el_lte_v: Less-than-or-equal.
 *   @el_assign_v: Assign.
 *   @el_pluseq_v: Plus assignment.
 *   @el_cast_v: Cast.
 *   @el_init_v: Initializer.
 *   @el_sizeof_v: Sizeof
 *   @el_index_v: Array subscript.
 */
enum el_expr_e {
	el_const_v,
	el_body_v,
	el_ident_v,
	el_call_v,
	el_deref_v,
	el_addr_v,
	el_elem_v,
	el_pos_v,
	el_neg_v,
	el_not_v,
	el_lnot_v,
	el_preinc_v,
	el_predec_v,
	el_postinc_v,
	el_postdec_v,
	el_mul_v,
	el_div_v,
	el_rem_v,
	el_add_v,
	el_sub_v,
	el_shl_v,
	el_shr_v,
	el_and_v,
	el_xor_v,
	el_or_v,
	el_land_v,
	el_lor_v,
	el_eq_v,
	el_ne_v,
	el_gt_v,
	el_gte_v,
	el_lt_v,
	el_lte_v,
	el_assign_v,
	el_addeq_v,
	el_subeq_v,
	el_muleq_v,
	el_diveq_v,
	el_remeq_v,
	el_shleq_v,
	el_shreq_v,
	el_andeq_v,
	el_xoreq_v,
	el_oreq_v,
	el_cast_v,
	el_init_v,
	el_sizeof_v,
	el_offsetof_v,
	el_getparent_v,
	el_index_v,
	el_initargs_v,
	el_getarg_v,
	el_eget_v,
	el_ereq_v,
	el_arrlen_v,
	el_getref_v,
	el_enval_v,
	el_tern_v
};

/**
 * Expression data union.
 *   @value: Value.
 *   @block: Function block.
 *   @ident: Identifier.
 *   @call: Call.
 *   @op1: One operand.
 *   @op2: Two operands.
 *   @init: Initializer.
 *   @size: Size-of.
 */
union el_expr_u {
	struct el_const_t *vconst;
	struct el_block_t *block;
	struct el_body_t *body;
	struct el_ident_t *ident;
	struct el_call_t *call;
	struct el_expr_t *op1;
	struct el_op2_t op2;
	struct el_init_t *init;
	struct el_sizeof_t *size;
	struct el_offsetof_t *offset;
	struct el_getparent_t *parent;
	struct el_elem_t *elem;
	struct el_index_t *index;
	struct el_getarg_t *getarg;
	struct el_enval_t *enval;
	struct el_tern_t *tern;
};

/**
 * Expression structure.
 *   @kind: The expression kind.
 *   @data: The expression data.
 *   @type: The data type.
 *   @tag: The tag.
 */
struct el_expr_t {
	enum el_expr_e kind;
	union el_expr_u data;

	struct el_type_t type;
	struct el_tag_t *tag;
};


/**
 * Identifier type enumerator.
 *   @el_unk_v: Unknown.
 *   @el_local_v: Local variable.
 *   @el_global_v: Global variable.
 *   @el_param_v: Parameter.
 *   @el_builtin_v: Builtin.
 */
enum el_ident_e {
	el_unk_v,
	el_local_v,
	el_global_v,
	el_param_v,
	el_builtin_v,
};

/**
 * Identifier data union.
 *   @off: Offset.
 *   @decl: Declaration.
 */
union el_ident_u {
	uint32_t off;
	struct el_decl_t *decl;
};

/**
 * Identifier structure.
 *   @str: The string ID.
 *   @type: The type.
 *   @data: The data.
 */
struct el_ident_t {
	char *str;

	enum el_ident_e type;
	union el_ident_u data;
};

/**
 * Argument structure.
 *   @expr: The expression.
 *   @next: The next argument.
 */
struct el_arg_t {
	struct el_expr_t *expr;
	struct el_arg_t *next;
};


/*
 * expression declarations
 */
struct el_expr_t *el_expr_new(enum el_expr_e kind, union el_expr_u data, struct el_type_t type, struct el_tag_t *tag);
void el_expr_delete(struct el_expr_t *expr);

struct el_expr_t *el_expr_const(struct el_const_t *vconst, struct el_tag_t *tag);
struct el_expr_t *el_expr_block(struct el_block_t *block, struct el_tag_t *tag);
struct el_expr_t *el_expr_body(struct el_body_t *body, struct el_tag_t *tag);
struct el_expr_t *el_expr_ident(struct el_ident_t *ident, struct el_tag_t *tag);
struct el_expr_t *el_expr_call(struct el_call_t *call, struct el_tag_t *tag);
struct el_expr_t *el_expr_addr(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_cast(struct el_expr_t *in, struct el_type_t type, struct el_tag_t *tag);
struct el_expr_t *el_expr_deref(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_neg(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_not(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_lnot(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_preinc(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_predec(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_postinc(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_postdec(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_arrlen(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_getref(struct el_expr_t *in, struct el_tag_t *tag);
struct el_expr_t *el_expr_add(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_sub(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_mul(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_div(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_rem(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_and(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_or(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_xor(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_land(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_lor(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_shr(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_shl(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_eq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_ne(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_gt(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_gte(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_lt(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_lte(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_assign(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_addeq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_subeq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_muleq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_diveq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_remeq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_shreq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_shleq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_andeq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_oreq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_xoreq(struct el_expr_t *lhs, struct el_expr_t *rhs, struct el_tag_t *tag);
struct el_expr_t *el_expr_init(struct el_type_t type, struct el_init_t *init, struct el_tag_t *tag);
struct el_expr_t *el_expr_sizeof(struct el_sizeof_t *size, struct el_tag_t *tag);
struct el_expr_t *el_expr_offsetof(struct el_offsetof_t *offset, struct el_tag_t *tag);
struct el_expr_t *el_expr_getparent(struct el_getparent_t *parent, struct el_tag_t *tag);
struct el_expr_t *el_expr_elem(struct el_elem_t *elem, struct el_tag_t *tag);
struct el_expr_t *el_expr_index(struct el_index_t *index, struct el_tag_t *tag);
struct el_expr_t *el_expr_initargs(struct el_tag_t *tag);
struct el_expr_t *el_expr_getarg(struct el_getarg_t *getarg, struct el_tag_t *tag);
struct el_expr_t *el_expr_eget(struct el_expr_t *get, struct el_tag_t *tag);
struct el_expr_t *el_expr_ereq(struct el_expr_t *get, struct el_tag_t *tag);
struct el_expr_t *el_expr_enval(struct el_enval_t *enval, struct el_tag_t *tag);
struct el_expr_t *el_expr_tern(struct el_tern_t *tern, struct el_tag_t *tag);

/*
 * identifier declarations
 */
struct el_ident_t *el_ident_new(char *str, enum el_ident_e type, union el_ident_u data);
void el_ident_delete(struct el_ident_t *ident);

struct el_ident_t *el_ident_unk(char *str);
struct el_ident_t *el_ident_local(char *str, struct el_decl_t *decl);
struct el_ident_t *el_ident_param(char *str, uint32_t off);


/**
 * Call structure.
 *   @deref: Dereference flag.
 *   @func: The function.
 *   @cnt: THe number of argumenst.
 *   @arg: The argument list.
 */
struct el_call_t {
	bool deref;
	struct el_expr_t *func;

	uint32_t cnt;
	struct el_arg_t *arg;
};

/*
 * call declarations
 */
struct el_call_t *el_call_new(struct el_expr_t *func, bool deref);
void el_call_delete(struct el_call_t *call);

struct el_arg_t *el_arg_new(struct el_expr_t *expr);


/**
 * Sizeof structure.
 *   @type: The type.
 *   @expr: The expression.
 */
struct el_sizeof_t {
	struct el_type_t type;
	struct el_expr_t *expr;
};

/*
 * sizeof declarations
 */
struct el_sizeof_t *el_sizeof_new(struct el_type_t type, struct el_expr_t *expr);
void el_sizeof_delete(struct el_sizeof_t *size);


/**
 * Offsetof structure.
 *   @type: The type.
 *   @id: The identifier.
 */
struct el_offsetof_t {
	struct el_type_t type;
	char *id;
};

/*
 * sizeof declarations
 */
struct el_offsetof_t *el_offsetof_new(struct el_type_t type, char *id);
void el_offsetof_delete(struct el_offsetof_t *size);


/**
 * Get parent structure.
 *   @expr: The expression.
 *   @type: The type.
 *   @id: The identifier.
 */
struct el_getparent_t {
	struct el_expr_t *expr;
	struct el_type_t type;
	char *id;
	int32_t off;
};

/*
 * sizeof declarations
 */
struct el_getparent_t *el_getparent_new(struct el_expr_t *expr, struct el_type_t type, char *id);
void el_getparent_delete(struct el_getparent_t *parent);


/**
 * Initialize a binary expression.
 *   @lhs: The left-hand side.
 *   @rhs: The right-hand side.
 *   &returns: The binary expression.
 */
static inline struct el_op2_t el_op2_init(struct el_expr_t *lhs, struct el_expr_t *rhs)
{
	return (struct el_op2_t){ lhs, rhs };
}

/**
 * Delete a binary operation.
 *   @op2: The binary operation.
 */
static inline void el_op2_delete(struct el_op2_t op2)
{
	el_expr_delete(op2.lhs);
	el_expr_delete(op2.rhs);
}


/*
 * body declarations
 */
struct el_body_t *el_body_new(struct el_func_t *func, struct el_block_t *block);
void el_body_delete(struct el_body_t *body);

/*
 * element declarations
 */
struct el_elem_t *el_elem_new(bool deref, struct el_expr_t *base, char *id);
void el_elem_delete(struct el_elem_t *elem);


/**
 * Index structure.
 *   @deref: Dereference structure.
 *   @base, off: The base and offset values.
 */
struct el_index_t {
	bool deref;
	struct el_expr_t *base, *off;
};

/*
 * index declarations
 */
struct el_index_t *el_index_new(bool deref, struct el_expr_t *base, struct el_expr_t *off);
void el_index_delete(struct el_index_t *index);


/*
 * get argument declarations
 */
struct el_getarg_t *el_getarg_new(struct el_expr_t *expr, struct el_type_t type);
void el_getarg_delete(struct el_getarg_t *get);

struct el_enval_t {
	char *type, *repr;
	int64_t val;
};

struct el_enval_t *el_enval_new(char *type, char *repr);
struct el_enval_t *el_enval_const(char *type, int64_t ival);
void el_enval_delete(struct el_enval_t *val);


struct el_tern_t {
	struct el_expr_t *cond, *ontrue, *onfalse;
};

struct el_tern_t *el_tern_new(struct el_expr_t *cond, struct el_expr_t *ontrue, struct el_expr_t *onfalse);
void el_tern_delete(struct el_tern_t *tern);


static inline bool el_is_cmp(enum el_expr_e tag) {
	switch(tag) {
	case el_eq_v:
	case el_ne_v:
	case el_gt_v:
	case el_gte_v:
	case el_lt_v:
	case el_lte_v:
		return true;

	default:
		return false;
	}
}
