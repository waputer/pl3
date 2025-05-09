#include "common.h"


/**
 * Environment structure.
 *   @up, unit: The parent and code unit environments.
 *   @vars, types: The list of variables and types.
 */
struct env_t {
	struct env_t *up, *unit;
	struct avl_root_t vars, types;
};

/**
 * Variable structure.
 *   @decl: The variable declaration.
 *   @node: The node.
 */
struct var_t {
	struct el_decl_t *decl;
	struct avl_node_t node;
};

/**
 * Type declaration structure.
 *   @tydef: The type defintion.
 *   @node: The node.
 */
struct type_t {
	struct el_tydecl_t *tydecl;
	struct avl_node_t node;
};


/*
 * local declarations
 */
static void chk_stmt(struct el_ctx_t *ctx, struct env_t *env, struct el_stmt_t *stmt, struct el_func_t *sig);
static void chk_decl(struct el_ctx_t *ctx, struct env_t *env, struct el_decl_t *decl, struct el_func_t *sig);
static void chk_block(struct el_ctx_t *ctx, struct env_t *env, struct el_block_t *block, struct el_func_t *sig);
static void chk_expr(struct el_ctx_t *ctx, struct env_t *env, struct el_expr_t *expr, struct el_func_t *sig);
static void chk_init(struct el_ctx_t *ctx, struct env_t *env, struct el_init_t *init, struct el_struct_t *vstruct, struct el_type_t def, struct el_func_t *sig, struct el_tag_t *tag);
static void chk_type(struct el_ctx_t *ctx, struct env_t *env, struct el_type_t type, struct el_tag_t *tag);
static void chk_func(struct el_ctx_t *ctx, struct env_t *env, struct el_func_t *func, struct el_tag_t *tag);
static void chk_fill(struct el_expr_t *expr, struct el_type_t type);
static bool chk_match(struct el_type_t lhs, struct el_type_t rhs);
static bool chk_assign(struct el_type_t lhs, struct el_type_t rhs);

static struct env_t env_new(struct env_t *up, struct env_t *unit);
static void env_delete(struct env_t *env);

static void env_var_add(struct env_t *env, struct el_decl_t *decl);
static struct el_decl_t *env_var_find(struct env_t *env, const char *id);

static void env_type_add(struct env_t *env, struct el_tydecl_t *tydecl);
static struct el_tydecl_t *env_type_find(struct env_t *env, const char *id);

//#define mkerr(ctx, ...) do { el_ctx_error(ctx, __VA_ARGS__); } while(0)
//#define reterr(ctx, ...) do { el_ctx_error(ctx, __VA_ARGS__); return; } while(0)
#define mkerr(ctx, ...) do { printf("fail %s:%u\n", __FILE__, __LINE__); el_ctx_error(ctx, __VA_ARGS__); } while(0)
#define reterr(ctx, ...) do { printf("fail %s:%u\n", __FILE__, __LINE__); el_ctx_error(ctx, __VA_ARGS__); return; } while(0)
#define mktype(ctx, tag, lhs, rhs) \
	do { \
		char _lhs[128], _rhs[128]; \
		el_type_tostr(lhs, _lhs, sizeof(_lhs)); \
		el_type_tostr(rhs, _rhs, sizeof(_rhs)); \
		mkerr(ctx, tag, "Type mismatch between '%s' and '%s'.", _lhs, _rhs); \
	} while(0)
#define rettype(ctx, tag, lhs, rhs) \
	do { \
		char _lhs[128], _rhs[128]; \
		el_type_tostr(lhs, _lhs, sizeof(_lhs)); \
		el_type_tostr(rhs, _rhs, sizeof(_rhs)); \
		reterr(ctx, tag, "Type mismatch between '%s' and '%s'.", _lhs, _rhs); \
	} while(0)


static void chk_add(struct el_ctx_t *ctx, struct env_t *env, struct el_stmt_t *stmt, bool pub)
{
	uint32_t mask = pub ? EL_PUB : 0;

	switch(stmt->type) {
	case el_decl_v: {
		struct el_decl_t *decl = stmt->data.decl;

		if((decl->flags & EL_PUB) == mask) {
			if(env_var_find(env, decl->id) != NULL)
				reterr(ctx, stmt->tag, "Duplicated declaration of '%s'.", decl->id);

			env_var_add(env, decl);
		}
	} break;

	case el_tydecl_v: {
		struct el_tydecl_t *decl = stmt->data.tydecl;

		if((decl->flags & EL_PUB) == mask) {
			if(env_type_find(env, decl->id) != NULL)
				reterr(ctx, stmt->tag, "Duplicated declaration of '%s'.", decl->id);

			env_type_add(env, decl);
		}
	} break;

	default:
		el_ctx_error(ctx, stmt->tag, "Non-declaration at top-level.");
	}
}

/**
 * Typecheck from the top.
 *   @top: The top.
 *   @ctx: The context.
 */
void el_chk_top(struct el_top_t *top, struct el_ctx_t *ctx)
{
	struct env_t env, nest;
	struct el_unit_t *unit;
	struct el_stmt_t *stmt;

	env = env_new(NULL, NULL);

	for(unit = top->unit; unit != NULL; unit = unit->next) {
		for(stmt = unit->block->head; stmt != NULL; stmt = stmt->next)
			chk_add(ctx, &env, stmt, true);
	}

	for(unit = top->unit; unit != NULL; unit = unit->next) {
		nest = env_new(&env, &nest);

		for(stmt = unit->block->head; stmt != NULL; stmt = stmt->next)
			chk_add(ctx, &nest, stmt, false);

		for(stmt = unit->block->head; stmt != NULL; stmt = stmt->next) {
			if(stmt->type == el_decl_v)
				chk_type(ctx, &nest, stmt->data.decl->type, stmt->data.decl->tag);
		}

		env_delete(&nest);
	}

	for(unit = top->unit; unit != NULL; unit = unit->next) {
		nest = env_new(&env, &nest);

		for(stmt = unit->block->head; stmt != NULL; stmt = stmt->next)
			chk_add(ctx, &nest, stmt, false);

		for(stmt = unit->block->head; stmt != NULL; stmt = stmt->next) {
			if(stmt->type == el_decl_v)
				chk_decl(ctx, &nest, stmt->data.decl, NULL);
			else if(stmt->type == el_tydecl_v)
				;//chk_decl(ctx, &nest, stmt->data.tydecl, el_type_void());
		}

		env_delete(&nest);
	}

	env_delete(&env);
}

/**
 * Type check a statement.
 *   @ctx: The context.
 *   @env: The environment.
 *   @stmt: The statement.
 *   @ret: The return type.
 */
static void chk_stmt(struct el_ctx_t *ctx, struct env_t *env, struct el_stmt_t *stmt, struct el_func_t *sig)
{
	switch(stmt->type) {
	case el_decl_v:
		env_var_add(env, stmt->data.decl);
		chk_decl(ctx, env, stmt->data.decl, sig);
		break;

	case el_tydecl_v:
		env_type_add(env, stmt->data.tydecl);
		break;

	case el_expr_v:
		chk_expr(ctx, env, stmt->data.expr, sig);
		break;

	case el_onerr_v:
	case el_block_v:
		chk_block(ctx, env, stmt->data.block, sig);
		break;

	case el_return_v:
		if(stmt->data.expr != NULL) {
			chk_expr(ctx, env, stmt->data.expr, sig);

			if(el_type_is_err(sig->ret) && !el_type_is_err(stmt->data.expr->type)) {
				struct el_init_t *init = el_init_expr(stmt->data.expr, strdup("val"));
				struct el_type_t type;
				struct el_init_t *err;

				if(el_type_is_ptr(el_err_get_err(sig->ret))) {
					type = el_type_ptr(el_ptr_new(0, el_type_u8()));
					err = el_init_expr(el_expr_const(el_const_ptr(el_num_zero(), type), el_tag_copy(stmt->tag)), strdup("err"));
				}
				else {
					type = el_type_i32();
					err = el_init_expr(el_expr_const(el_const_i32(0), el_tag_copy(stmt->tag)), strdup("err"));
				}
                
				if(el_type_is_void(stmt->data.expr->type))
					chk_fill(stmt->data.expr, el_err_get_val(sig->ret));

				init->idx = 0;
				stmt->data.expr = el_expr_init(el_type_copy(sig->ret), init, el_tag_copy(stmt->tag));

				init->next = err;
				err->idx = 1;
				err->data.expr->type = el_type_copy(type);
			}
			else if(el_type_is_void(stmt->data.expr->type))
				chk_fill(stmt->data.expr, sig->ret);

			if(!chk_assign(sig->ret, stmt->data.expr->type))
				mktype(ctx, stmt->tag, sig->ret, stmt->data.expr->type);
		}

		break;

	case el_eret_v:
		chk_expr(ctx, env, stmt->data.expr, sig);
		break;

	case el_cond_v: {
		struct el_cond_t *cond = stmt->data.cond;

		chk_expr(ctx, env, cond->eval, sig);
		if(cond->eval->type.kind != el_bool_v)
			mkerr(ctx, cond->eval->tag, "Expected boolean.");

		if(cond->ontrue != NULL)
			chk_block(ctx, env, cond->ontrue, sig);

		if(cond->onfalse != NULL)
			chk_block(ctx, env, cond->onfalse, sig);
	} break;

	case el_loop_v: {
		struct el_loop_t *loop = stmt->data.loop;

		if(loop->init != NULL)
			chk_expr(ctx, env, loop->init, sig);

		if(loop->cond != NULL) {
			chk_expr(ctx, env, loop->cond, sig);
			if(loop->cond->type.kind != el_bool_v)
				mkerr(ctx, loop->cond->tag, "Expected boolean.");
		}

		if(loop->inc != NULL)
			chk_expr(ctx, env, loop->inc, sig);

		chk_block(ctx, env, loop->body, sig);
	} break;
	
	case el_brk_v:
	case el_cont_v:
	case el_label_v:
		break;

	case el_switch_v: {
		struct el_case_t *vcase;
		struct el_switch_t *sw = stmt->data.sw;

		chk_expr(ctx, env, sw->eval, sig);
		chk_block(ctx, env, sw->block, sig);

		for(vcase = sw->vcase; vcase != NULL; vcase = vcase->next) {
			if(vcase->expr == NULL)
				continue;

			chk_expr(ctx, env, vcase->expr, sig);

			if(el_type_is_void(vcase->expr->type))
				chk_fill(vcase->expr, sw->eval->type);
		}
	} break;
	}
}

/**
 * Type check a declaration.
 *   @ctx: The context.
 *   @env: The environment.
 *   @decl: The declaration.
 *   @ret: The return type.
 */
static void chk_decl(struct el_ctx_t *ctx, struct env_t *env, struct el_decl_t *decl, struct el_func_t *sig)
{
	chk_type(ctx, env, decl->type, decl->tag);

	if(decl->type.kind == el_func_v) {
		struct env_t nest;
		struct el_stmt_t *stmt;
		struct el_body_t *body;

		if(decl->expr == NULL) {
			if((decl->flags & EL_EXTERN) == 0)
				fatal("Empty functions must be extern.");
		}
		else {
			struct avl_node_t *node;

			if(decl->expr->kind != el_body_v)
				fatal("Functions must have a function body.");

			body = decl->expr->data.body;
			nest = env_new(env->unit, env->unit);

			for(node = avl_root_first(&env->vars); node != NULL; node = avl_node_next(node)) {
				struct var_t *var = getparent(node, struct var_t, node);

				if((var->decl->flags & EL_MUT) == 0)
					env_var_add(&nest, var->decl);
			}

			chk_func(ctx, &nest, body->func, decl->tag);

			for(stmt = body->block->head; stmt != NULL; stmt = stmt->next)
				chk_stmt(ctx, &nest, stmt, body->func);

			env_delete(&nest);
		}
	}
	else {
		if(decl->expr != NULL) {
			chk_expr(ctx, env, decl->expr, sig);

			if(el_type_is_void(decl->expr->type))
				chk_fill(decl->expr, decl->type);

			if(!chk_assign(decl->type, decl->expr->type))
				rettype(ctx, decl->tag, decl->type, decl->expr->type);
		}
	}
}

/**
 * Type check a block.
 *   @ctx: The context.
 *   @env: The environment.
 *   @expr: The expression.
 *   @ret: The return type.
 */
static void chk_block(struct el_ctx_t *ctx, struct env_t *env, struct el_block_t *block, struct el_func_t *sig)
{
	struct env_t nest;
	struct el_stmt_t *stmt;

	nest = env_new(env, env->unit);

	for(stmt = block->head; stmt != NULL; stmt = stmt->next)
		chk_stmt(ctx, &nest, stmt, sig);

	env_delete(&nest);
}

/**
 * Type check an expression.
 *   @ctx: The context.
 *   @env: The environment.
 *   @expr: The expression.
 *   @ret: The return type.
 */
static void chk_expr(struct el_ctx_t *ctx, struct env_t *env, struct el_expr_t *expr, struct el_func_t *sig)
{
	switch(expr->kind) {
	case el_const_v: {
		struct el_type_t type = el_type_void();
		struct el_const_t *imm = expr->data.vconst;

		switch(imm->type) {
		case el_const_bool_v: type = el_type_bool(); break;
		case el_const_i8_v: type = el_type_i8(); break;
		case el_const_u8_v: type = el_type_u8(); break;
		case el_const_i16_v: type = el_type_i16(); break;
		case el_const_u16_v: type = el_type_u16(); break;
		case el_const_i32_v: type = el_type_i32(); break;
		case el_const_u32_v: type = el_type_u32(); break;
		case el_const_i64_v: type = el_type_i64(); break;
		case el_const_u64_v: type = el_type_u64(); break;
		case el_const_num_v: type = el_type_void(); break;
		case el_const_ptr_v: type = el_type_void(); break;
		case el_const_str_v: type = el_type_ptr(el_ptr_new(EL_CONST, el_type_u8())); break;
		}

		el_type_replace(&expr->type, type);
	} break;

	case el_ident_v: {
		struct el_builtin_t *builtin;
		struct el_ident_t *ident = expr->data.ident;

		if((builtin = el_builtin_find(ident->str)) != NULL) {
			ident->type = el_builtin_v;
			el_type_replace(&expr->type, builtin->func());
		}
		else {
			int32_t idx;
			struct el_decl_t *decl;
			
			if(ident->type != el_unk_v)
				fatal("huh?");

			if((decl = env_var_find(env, ident->str)) != NULL) {
				if(decl->flags & EL_EXTERN) {
					ident->type = el_unk_v;
					el_type_replace(&expr->type, el_type_copy(decl->type));
				}
				else {
					ident->type = (decl->flags & EL_GLOB) ? el_global_v : el_local_v;
					ident->data.decl = decl;
					el_type_replace(&expr->type, el_type_copy(decl->type));
				}
			}
			else if((sig != NULL) && ((idx = el_func_find(sig, ident->str)) >= 0)) {
				ident->type = el_param_v;
				ident->data.off = idx;
				el_type_replace(&expr->type, el_type_copy(sig->param[idx].type));
			}
			else
				mkerr(ctx, expr->tag, "Unknown identifier '%s'.", ident->str);
		}

		//char str[128];
		//el_type_tostr(expr->type, str, sizeof(str));
		//printf("jkldfjlsjflsdj [%s]\n" ,str);
		//chk_type(ctx, env, expr->type, expr->tag);
	} break;

	case el_call_v: {
		uint32_t i;
		struct el_arg_t *arg;
		struct el_type_t type;
		struct el_func_t *func;
		struct el_call_t *call = expr->data.call;

		chk_expr(ctx, env, call->func, sig);

		if(call->deref) {
			if(!el_type_is_ptr(call->func->type))
				reterr(ctx, expr->tag, "Cannot dereference non-pointer type.");

			type = call->func->type.data.ptr->sub;
			chk_type(ctx, env, type, expr->tag);
		}
		else
			type = call->func->type;

		type = el_type_root(type);
		if(type.kind != el_func_v)
			reterr(ctx, call->func->tag, "Can only call function types.");

		func = type.data.func;

		for(i = 0, arg = call->arg; arg != NULL; i++, arg = arg->next) {
			chk_expr(ctx, env, arg->expr, sig);

			if(i < func->cnt) {
				if(el_type_is_void(arg->expr->type))
					chk_fill(arg->expr, func->param[i].type);

				if(!chk_assign(func->param[i].type, arg->expr->type))
					rettype(ctx, arg->expr->tag, func->param[i].type, arg->expr->type);
			}
		}

		if(i < func->cnt)
			reterr(ctx, expr->tag, "Not enough parameters.");

		el_type_replace(&expr->type, el_type_copy(func->ret));
	} break;

	case el_addr_v: {
		chk_expr(ctx, env, expr->data.op1, sig);

		if(el_type_is_arr(expr->data.op1->type))
			el_type_replace(&expr->type, el_type_ptr(el_ptr_new(0, el_type_copy(expr->data.op1->type.data.arr->type))));
		else
			el_type_replace(&expr->type, el_type_ptr(el_ptr_new(0, el_type_copy(expr->data.op1->type))));
	} break;

	case el_deref_v: {
		struct el_expr_t *op1 = expr->data.op1;

		chk_expr(ctx, env, op1, sig);
		if(op1->type.kind == el_ptr_v) {
			chk_type(ctx, env, op1->type.data.ptr->sub, expr->tag);
			el_type_replace(&expr->type, el_type_copy(op1->type.data.ptr->sub));
		}
		else if(op1->type.kind == el_arr_v)
			el_type_replace(&expr->type, el_type_ptr(el_ptr_new(0, el_type_copy(op1->type.data.arr->type))));
		else
			reterr(ctx, expr->tag, "Cannot derefence non-pointer type.");

		chk_type(ctx, env, expr->type, expr->tag);
	} break;

	case el_pos_v:
	case el_neg_v:
	case el_not_v: {
		struct el_expr_t *op1 = expr->data.op1;

		chk_expr(ctx, env, op1, sig);

		el_type_replace(&expr->type, el_type_copy(op1->type));
	} break;

	case el_lnot_v: {
		struct el_expr_t *op1 = expr->data.op1;

		chk_expr(ctx, env, op1, sig);

		if(!el_type_is_bool(op1->type))
			reterr(ctx, expr->tag, "Cannot apply logical not to a non-boolean type.");

		el_type_replace(&expr->type, el_type_copy(op1->type));
	} break;
	case el_preinc_v:
	case el_predec_v:
	case el_postinc_v:
	case el_postdec_v: {
		struct el_expr_t *op1 = expr->data.op1;

		chk_expr(ctx, env, op1, sig);

		el_type_replace(&expr->type, el_type_copy(op1->type));
	} break;

	case el_cast_v: {
		struct el_expr_t *op1 = expr->data.op1;

		chk_expr(ctx, env, op1, sig);

		if(el_type_is_void(op1->type))
			chk_fill(op1, expr->type);
	} break;

	case el_add_v:
	case el_sub_v:
	case el_mul_v:
	case el_div_v:
	case el_rem_v:
	case el_addeq_v:
	case el_subeq_v:
	case el_muleq_v:
	case el_diveq_v:
	case el_remeq_v: {
		struct el_op2_t op2 = expr->data.op2;

		chk_expr(ctx, env, op2.lhs, sig);
		chk_expr(ctx, env, op2.rhs, sig);

		if(el_type_is_ptr(op2.lhs->type) && (el_type_is_int(op2.rhs->type) || el_type_is_void(op2.rhs->type))) {
			chk_type(ctx, env,op2.lhs->type.data.ptr->sub, expr->tag);

			if(el_type_is_void(op2.rhs->type))
				chk_fill(op2.rhs, el_type_i32());

			if(!el_type_is_num(op2.rhs->type))
				reterr(ctx, expr->tag, "Type mismatch.");
		}
		else {
			if(el_type_is_void(op2.lhs->type))
				chk_fill(op2.lhs, op2.rhs->type);

			if(el_type_is_void(op2.rhs->type))
				chk_fill(op2.rhs, op2.lhs->type);

			if(!chk_match(op2.lhs->type, op2.rhs->type))
				rettype(ctx, expr->tag, op2.lhs->type, op2.rhs->type);
		}

		el_type_replace(&expr->type, el_type_copy(op2.lhs->type));
	} break;

	case el_and_v:
	case el_xor_v:
	case el_or_v:
	case el_andeq_v:
	case el_xoreq_v:
	case el_oreq_v: {
		struct el_op2_t op2 = expr->data.op2;

		chk_expr(ctx, env, op2.lhs, sig);
		chk_expr(ctx, env, op2.rhs, sig);

		if(el_type_is_void(op2.lhs->type))
			chk_fill(op2.lhs, op2.rhs->type);

		if(el_type_is_void(op2.rhs->type))
			chk_fill(op2.rhs, op2.lhs->type);

		if(!chk_match(op2.lhs->type, op2.rhs->type))
			reterr(ctx, expr->tag, "Type mismatch.");

		el_type_replace(&expr->type, el_type_copy(op2.lhs->type));
	} break;

	case el_land_v:
	case el_lor_v: {
		struct el_op2_t op2 = expr->data.op2;

		chk_expr(ctx, env, op2.lhs, sig);
		chk_expr(ctx, env, op2.rhs, sig);

		if(!el_type_is_bool(op2.lhs->type))
			mkerr(ctx, op2.lhs->tag, "Expected boolean.");

		if(!el_type_is_bool(op2.rhs->type))
			mkerr(ctx, op2.rhs->tag, "Expected boolean.");

		el_type_replace(&expr->type, el_type_bool());
	} break;

	case el_shl_v:
	case el_shr_v: {
		struct el_op2_t op2 = expr->data.op2;

		chk_expr(ctx, env, op2.lhs, sig);
		chk_expr(ctx, env, op2.rhs, sig);

		if(el_type_is_void(op2.rhs->type))
			chk_fill(op2.rhs, op2.lhs->type);

		el_type_replace(&expr->type, el_type_copy(op2.lhs->type));
	} break;

	case el_shleq_v:
	case el_shreq_v:
		fatal("stub");

	case el_eq_v:
	case el_ne_v:
	case el_gt_v:
	case el_gte_v:
	case el_lt_v:
	case el_lte_v: {
		struct el_op2_t op2 = expr->data.op2;

		chk_expr(ctx, env, op2.lhs, sig);
		chk_expr(ctx, env, op2.rhs, sig);

		if(el_type_is_void(op2.lhs->type))
			chk_fill(op2.lhs, op2.rhs->type);

		if(el_type_is_void(op2.rhs->type))
			chk_fill(op2.rhs, op2.lhs->type);

		if(!chk_match(op2.lhs->type, op2.rhs->type))
			reterr(ctx, expr->tag, "Type mismatch.??");

		el_type_replace(&expr->type, el_type_bool());
	} break;

	case el_assign_v: {
		struct el_op2_t op2 = expr->data.op2;

		chk_expr(ctx, env, op2.lhs, sig);
		chk_expr(ctx, env, op2.rhs, sig);

		if(el_type_is_void(op2.rhs->type))
			chk_fill(op2.rhs, op2.lhs->type);

		if(!chk_assign(op2.lhs->type, op2.rhs->type))
			rettype(ctx, expr->tag, op2.lhs->type, op2.rhs->type);

		el_type_replace(&expr->type, el_type_copy(op2.lhs->type));
	} break;

	case el_sizeof_v: {
		struct el_sizeof_t *size = expr->data.size;

		if(size->expr != NULL) {
			chk_expr(ctx, env, size->expr, sig);
			el_type_replace(&size->type, el_type_copy(size->expr->type));
		}
		else
			chk_type(ctx, env, expr->data.size->type, expr->tag);
	} break;

	case el_offsetof_v: {
		chk_type(ctx, env, expr->data.offset->type, expr->tag);
	} break;

	case el_getparent_v: {
		int32_t idx;
		struct el_struct_t *vstruct;
		struct el_getparent_t *parent = expr->data.parent;

		chk_expr(ctx, env, parent->expr, sig);
		chk_type(ctx, env, parent->type, expr->tag);

		if(!el_type_is_struct(parent->type))
			reterr(ctx, expr->tag, "Can only take an offset of a structure.");

		vstruct = el_type_root(parent->type).data.vstruct;
		idx = el_struct_find(vstruct, parent->id);
		if(idx < 0)
			reterr(ctx, expr->tag, "Unknown member '%s'.", parent->id);

		struct el_type_t type = el_type_ptr(el_ptr_new(0, el_type_copy(el_struct_get(vstruct, idx)->type)));

		if(!chk_assign(parent->expr->type, type))
			rettype(ctx, expr->tag, parent->expr->type, el_struct_get(vstruct, idx)->type);

		el_type_delete(type);
		el_type_replace(&expr->type, el_type_ptr(el_ptr_new(0, el_type_copy(parent->type))));
		parent->off = (el_struct_offset(vstruct, idx) + 7) / 8;
	} break;

	case el_arrlen_v:
		chk_expr(ctx, env, expr->data.op1, sig);

		if(!el_type_is_arr(expr->data.op1->type))
			fatal("Must have array type.");
		break;

	case el_getref_v:
		chk_expr(ctx, env, expr->data.op1, sig);
		el_type_replace(&expr->type, el_type_ptr(el_ptr_new(0, el_type_copy(expr->data.op1->type))));
		break;

	case el_elem_v: {
		struct el_type_t type;
		struct el_elem_t *elem = expr->data.elem;

		chk_expr(ctx, env, elem->base, sig);

		type = elem->base->type;
		if(elem->deref) {
			if(type.kind != el_ptr_v)
				reterr(ctx, elem->base->tag, "Cannot dereference non-pointer type. !!");

			type = type.data.ptr->sub;
			chk_type(ctx, env, type, elem->base->tag);
		}

		if((type.kind == el_struct_v) || (type.kind == el_union_v)) {
			elem->idx = el_struct_find(type.data.vstruct, elem->id);
			if(elem->idx < 0)
				reterr(ctx, elem->base->tag, "Member '%s' not found in type.", elem->id);

			el_type_replace(&expr->type, el_type_copy(el_struct_get(type.data.vstruct, elem->idx)->type));
		}
		else {
			char str[256];
			el_type_tostr(type, str, sizeof(str));
			mkerr(ctx, elem->base->tag, "Cannot access member '%s' of non-compound type. [%s]", elem->id, str);
		}
	} break;

	case el_init_v: {
		struct el_init_t *init = expr->data.init;

		if(el_type_is_int(expr->type) || el_type_is_ptr(expr->type)/* || issame struct */) {
			if(init->nest || (init->next != NULL))
				reterr(ctx, expr->tag, "Invalid cast.");

			expr->kind = el_cast_v;
			expr->data.op1 = init->data.expr;
			free(init);

			struct el_expr_t *op1 = expr->data.op1;
			chk_expr(ctx, env, op1, sig);

			if(el_type_is_void(op1->type))
				chk_fill(op1, expr->type);
		}
		else {
			chk_type(ctx, env, expr->type, expr->tag);
			chk_init(ctx, env, init, (expr->type.kind == el_struct_v || expr->type.kind == el_union_v) ? expr->type.data.vstruct : NULL, (el_type_is_arr(expr->type) ? expr->type.data.arr->type : el_type_void()), sig, expr->tag);
		}
	} break;

	case el_body_v: {
		struct env_t nest;
		struct avl_node_t *node;

		nest = env_new(env->unit, env->unit);

		for(node = avl_root_first(&env->vars); node != NULL; node = avl_node_next(node)) {
			struct var_t *var = getparent(node, struct var_t, node);

			if((var->decl->flags & EL_MUT) == 0)
				env_var_add(&nest, var->decl);
		}

		chk_func(ctx, env, expr->data.body->func, expr->tag);
		chk_block(ctx, &nest, expr->data.body->block, expr->data.body->func);
		env_delete(&nest);
	} break;

	case el_index_v: {
		struct el_index_t *index = expr->data.index;

		chk_expr(ctx, env, index->base, sig);
		chk_expr(ctx, env, index->off, sig);

		if(el_type_is_void(index->off->type))
			chk_fill(index->off, el_type_i32());

		if(el_type_is_ptr(index->base->type)) {
			chk_type(ctx, env, index->base->type.data.ptr->sub, expr->tag);
			el_type_replace(&expr->type, el_type_copy(index->base->type.data.ptr->sub));
		}
		else if(el_type_is_arr(index->base->type))
			el_type_replace(&expr->type, el_type_copy(index->base->type.data.arr->type));
		else
			reterr(ctx, index->base->tag, "Expected pointer or array.");

		if(!el_type_is_int(index->off->type))
			reterr(ctx, index->base->tag, "Expected integer type.");
	} break;

	case el_initargs_v:
		el_type_replace(&expr->type, el_type_args());
		break;

	case el_getarg_v: {
		struct el_expr_t *args = expr->data.getarg->expr;

		chk_type(ctx, env, expr->data.getarg->type, expr->tag);
		chk_expr(ctx, env, args, sig);
		if(args->type.kind != el_ptr_v)
			reterr(ctx, expr->tag, "Expected type `pt:ty:args`.");
		else if(args->type.data.ptr->sub.kind != el_args_v)
			reterr(ctx, expr->tag, "Expected type `pt:ty:args`.");

		el_type_replace(&expr->type, el_type_copy(expr->data.getarg->type));
	} break;

	case el_eget_v:
	case el_ereq_v: {
		int32_t idx;
		struct el_struct_t *vstruct;
		struct el_expr_t *op1 = expr->data.op1;

		chk_expr(ctx, env, expr->data.op1, sig);

		if(op1->type.kind != el_struct_v)
			reterr(ctx, expr->tag, "e:get only applies to error type.");

		vstruct = op1->type.data.vstruct;
		if(strcmp(vstruct->id, "!e") != 0)
			reterr(ctx, expr->tag, "e:get only applies to error type.");

		idx = el_struct_find(vstruct, "val");
		if(idx >= 0)
			el_type_replace(&expr->type, el_type_copy(el_struct_get(vstruct, idx)->type));
		else
			el_type_replace(&expr->type, el_type_void());

	} break;

	case el_enval_v: {
		struct el_tydecl_t *decl;
		struct el_enval_t *val = expr->data.enval;
		struct el_enum_t *venum;
		struct el_repr_t *repr;
		
		decl = env_type_find(env, val->type);
		if(decl == NULL)
			reterr(ctx, expr->tag, "Cannot find enumerator type '%s'.", val->type);

		if(!el_type_is_enum(decl->type))
			reterr(ctx, expr->tag, "Cannot find enumerator type '%s'.", val->type);

		venum = decl->type.data.venum;
		repr = el_enum_find(venum, val->repr);
		if(repr == NULL)
			reterr(ctx, expr->tag, "No such value '%s' for enumerator '%s'.", val->repr, val->type);

		val->val = repr->val;
		el_type_replace(&expr->type, el_type_enum(el_enum_copy(venum)));
	} break;

	case el_tern_v: {
		struct el_tern_t *tern = expr->data.tern;

		chk_expr(ctx, env, tern->cond, sig);
		if(tern->cond->type.kind != el_bool_v)
			mkerr(ctx, tern->cond->tag, "Expected boolean.");

		chk_expr(ctx, env, tern->ontrue, sig);
		chk_expr(ctx, env, tern->onfalse, sig);

		if(el_type_is_void(tern->ontrue->type))
			chk_fill(tern->ontrue, tern->onfalse->type);
		else if(el_type_is_void(tern->onfalse->type))
			chk_fill(tern->onfalse, tern->ontrue->type);

		if(!chk_match(tern->ontrue->type, tern->onfalse->type))
			rettype(ctx, expr->tag, tern->ontrue->type, tern->onfalse->type);

		el_type_replace(&expr->type, el_type_copy(tern->ontrue->type));
	} break;
	}
}

/**
 * Resolve an initializer.
 *   @init: The initializer.
 *   @tag: The tag.
 *   @env; The environment.
 */
static void chk_init(struct el_ctx_t *ctx, struct env_t *env, struct el_init_t *init, struct el_struct_t *vstruct, struct el_type_t def, struct el_func_t *sig, struct el_tag_t *tag)
{
	int32_t i = 0;
	struct el_init_t *iter;
	struct el_type_t type;

	for(iter = init; iter != NULL; iter = iter->next) {
		if(iter->id != NULL) {
			if(vstruct == NULL)
				reterr(ctx, tag, "Invalid initializer.");

			iter->idx = el_struct_find(vstruct, iter->id);
			if(iter->idx < 0)
				fatal("Unknown field '%s'.", iter->id);

			type = el_struct_get(vstruct, iter->idx)->type;
		}
		else {
			iter->idx = i++;

			if(vstruct != NULL) {
				struct el_member_t *memb = el_struct_get(vstruct, iter->idx);

				if(memb == NULL)
					reterr(ctx, tag, "Too many initializers.");

				type = memb->type;
			}
			else
				type = def;
		}

		if(iter->nest) {
			chk_init(ctx, env, iter->data.nest, (type.kind == el_struct_v || type.kind == el_union_v) ? type.data.vstruct : NULL, (el_type_is_arr(type) ? type.data.arr->type : el_type_void()), sig, tag);
		}
		else {
			chk_expr(ctx, env, iter->data.expr, sig);

			if(el_type_is_void(iter->data.expr->type))
				chk_fill(iter->data.expr, type);

			if(!chk_assign(type, iter->data.expr->type))
				rettype(ctx, tag, type, iter->data.expr->type);
		}
	}
}

/**
 * Type check a type.
 *   @ctx: The context.
 *   @env: The environment.
 *   @type: The type.
 *   @ret: The return type.
 */
static void chk_type(struct el_ctx_t *ctx, struct env_t *env, struct el_type_t type, struct el_tag_t *tag)
{
	switch(type.kind) {
	case el_struct_v:
	case el_union_v: {
		struct el_tydecl_t *decl;
		struct el_member_t *member;
		struct el_struct_t *vstruct = type.data.vstruct;

		if(vstruct->member == NULL) {
			decl = env_type_find(env, vstruct->id);
			if(decl == NULL)
				reterr(ctx, tag, "Unknown structure '%s'.", vstruct->id);

			if(decl->type.kind != el_struct_v)
				fatal("Invalid expanded type.");

			vstruct->member = el_member_clone(decl->type.data.vstruct->member);
		}

		for(member = vstruct->member; member != NULL; member = member->next)
			chk_type(ctx, env, member->type, tag);

	} break;

	case el_arr_v: {
		struct el_arr_t *arr = type.data.arr;

		chk_type(ctx, env, arr->type, tag);
	} break;

	case el_func_v:
		chk_func(ctx, env, type.data.func, tag);
		break;

	case el_alias_v: {
		struct el_tydecl_t *decl;
		struct el_alias_t *alias = type.data.alias;

		if(alias->type.kind == el_void_v) {
			decl = env_type_find(env, alias->id);
			if(decl == NULL)
				reterr(ctx, tag, "Unknown alias '%s'.", alias->id);

			if(decl->type.kind != el_alias_v)
				fatal("Invalid expanded type.");

			alias->type = el_type_copy(decl->type);
		}

		chk_type(ctx, env, type.data.alias->type, tag);
	} break;

	case el_ptr_v: {
		struct el_ptr_t *ptr = type.data.ptr;

		if((ptr->sub.kind == el_struct_v) || (ptr->sub.kind == el_union_v))
			break;

		chk_type(ctx, env, ptr->sub, tag);
	} break;

	default:
		break;
	}
}

/**
 * Resolve a function type.
 *   @ctx: The context.
 *   @env: The environment.
 *   @func: The function type.
 *   @ret: The return type.
 */
static void chk_func(struct el_ctx_t *ctx, struct env_t *env, struct el_func_t *func, struct el_tag_t *tag)
{
	uint32_t i;

	chk_type(ctx, env, func->ret, tag);

	for(i = 0; i < func->cnt; i++)
		chk_type(ctx, env, func->param[i].type, tag);
}

static void chk_fill(struct el_expr_t *expr, struct el_type_t type)
{
	if(el_type_is_void(type) || !el_type_is_void(expr->type))
		return;

	switch(expr->kind) {
	case el_const_v: {
		struct el_const_t **imm = &expr->data.vconst;

		if((*imm)->type == el_const_num_v) {
			struct el_const_t *repl;

			if(type.kind == el_enum_v)
				type = el_type_i32();

			switch(type.kind) {
			case el_bool_v: repl = el_const_bool(mpz_get_si((*imm)->data.num->mpz)); break;
			case el_i8_v: repl = el_const_i8(mpz_get_si((*imm)->data.num->mpz)); break; 
			case el_u8_v: repl = el_const_u8(mpz_get_ui((*imm)->data.num->mpz)); break; 
			case el_i16_v: repl = el_const_i16(mpz_get_si((*imm)->data.num->mpz)); break; 
			case el_u16_v: repl = el_const_u16(mpz_get_ui((*imm)->data.num->mpz)); break; 
			case el_i32_v: repl = el_const_i32(mpz_get_si((*imm)->data.num->mpz)); break; 
			case el_u32_v: repl = el_const_u32(mpz_get_ui((*imm)->data.num->mpz)); break; 
			case el_i64_v: repl = el_const_i64(mpz_get_si((*imm)->data.num->mpz)); break; 
			case el_u64_v: repl = el_const_u64(mpz_get_ui((*imm)->data.num->mpz)); break; 
			case el_ptr_v: repl = el_const_ptr(el_num_copy((*imm)->data.num), el_type_copy(type)); break; 
			default: {
				char str[256];
				el_type_tostr(type, str, 256);
				fatal("%s:%u: stub? %s", expr->tag->file->buf, expr->tag->line, str);
			} break;
			}

			el_const_delete(*imm);
			*imm = repl;

			el_type_replace(&expr->type, el_type_copy(type));
		}
	 } break;

	case el_pos_v:
	case el_neg_v:
	case el_not_v:
		chk_fill(expr->data.op1, type);
		el_type_replace(&expr->type, el_type_copy(type));
		break;

	case el_add_v:
	case el_sub_v:
	case el_mul_v:
	case el_div_v:
	case el_rem_v:
	case el_and_v:
	case el_xor_v:
	case el_or_v:
	case el_shl_v:
	case el_shr_v:
		chk_fill(expr->data.op2.lhs, type);
		chk_fill(expr->data.op2.rhs, type);
		el_type_replace(&expr->type, el_type_copy(type));
		break;

	case el_sizeof_v:
	case el_offsetof_v:
		el_type_replace(&expr->type, el_type_copy(type));
		break;

	case el_getparent_v:
		break;

	case el_arrlen_v:
		el_type_replace(&expr->type, el_type_copy(type));
		break;

	case el_getref_v:
		break;

	case el_tern_v:
		chk_fill(expr->data.tern->ontrue, type);
		chk_fill(expr->data.tern->onfalse, type);
		el_type_replace(&expr->type, el_type_copy(type));
		break;

	case el_ident_v:
	case el_body_v:
	case el_call_v:
	case el_deref_v:
	case el_addr_v:
	case el_elem_v:
	case el_lnot_v:
	case el_land_v:
	case el_lor_v:
	case el_eq_v:
	case el_ne_v:
	case el_gt_v:
	case el_gte_v:
	case el_lt_v:
	case el_lte_v:
	case el_preinc_v:
	case el_predec_v:
	case el_postinc_v:
	case el_postdec_v:
	case el_assign_v:
	case el_addeq_v:
	case el_subeq_v:
	case el_muleq_v:
	case el_diveq_v:
	case el_remeq_v:
	case el_shleq_v:
	case el_shreq_v:
	case el_andeq_v:
	case el_xoreq_v:
	case el_oreq_v:
	case el_cast_v:
	case el_init_v:
	case el_index_v:
	case el_initargs_v:
	case el_getarg_v:
	case el_eget_v:
	case el_ereq_v:
	case el_enval_v:
		break;
	}
}

static bool chk_match(struct el_type_t lhs, struct el_type_t rhs)
{
	while(true) {
		if(lhs.kind != rhs.kind)
			return false;

		lhs = el_type_root(lhs);
		rhs = el_type_root(rhs);

		if(lhs.kind == el_ptr_v) {
			//if((lhs.data.ptr->flags & EL_CONST) != (rhs.data.ptr->flags & EL_CONST))
				//return false;

			lhs = lhs.data.ptr->sub;
			rhs = rhs.data.ptr->sub;

			if((lhs.kind == el_void_v) || (rhs.kind == el_void_v))
				break;
		}
		else if(lhs.kind == el_struct_v || lhs.kind == el_union_v) {
			if(strcmp(lhs.data.vstruct->id, rhs.data.vstruct->id) != 0)
				return false;

			break;
		}
		else if(lhs.kind == el_alias_v) {
			if(strcmp(lhs.data.alias->id, rhs.data.alias->id) != 0)
				return false;

			break;
		}
		else
			break;
	}

	return true;
}

static bool chk_assign(struct el_type_t lhs, struct el_type_t rhs)
{
	bool strict = false;

	while(true) {
		lhs = el_type_root(lhs);
		rhs = el_type_root(rhs);

		if(lhs.kind != rhs.kind)
			return false;

		if(lhs.kind == el_ptr_v) {
			if(!(lhs.data.ptr->flags & EL_CONST) && (rhs.data.ptr->flags & EL_CONST))
				return false;
			else if(strict && !(rhs.data.ptr->flags & EL_CONST))
				return false;

			strict |= rhs.data.ptr->flags & EL_CONST;
			lhs = lhs.data.ptr->sub;
			rhs = rhs.data.ptr->sub;

			if((lhs.kind == el_void_v) || (rhs.kind == el_void_v))
				break;
		}
		else if(lhs.kind == el_func_v) {
			uint32_t i;

			if(!chk_assign(lhs.data.func->ret, rhs.data.func->ret))
				return false;

			if(lhs.data.func->cnt != rhs.data.func->cnt)
				return false;

			for(i = 0; i < lhs.data.func->cnt; i++) {
				if(!chk_assign(rhs.data.func->param[i].type, lhs.data.func->param[i].type))
					return false;
			}

			break;
		}
		else if(lhs.kind == el_struct_v || lhs.kind == el_union_v) {
			if(strcmp(lhs.data.vstruct->id, rhs.data.vstruct->id) != 0)
				return false;

			break;
		}
		else
			break;
	}

	return true;
}


/**
 * Create an envionrment.
 *   @up: The parent environment.
 *   @unit: The code unit environment.
 *   &returns: The environment.
 */
static struct env_t env_new(struct env_t *up, struct env_t *unit)
{
	struct env_t env;

	env.up = up;
	env.unit = unit;
	env.vars = avl_root_init(avl_cmp_str);
	env.types = avl_root_init(avl_cmp_str);

	return env;
}

/**
 * Delete an environment.
 *   @env: The environment.
 */
static void env_delete(struct env_t *env)
{
	avl_root_destroy(&env->vars, offsetof(struct var_t, node), free);
	avl_root_destroy(&env->types, offsetof(struct type_t, node), free);
}


/**
 * Add a variable to the environment.
 *   @env: The environment.
 *   @ref: The reference.
 */
static void env_var_add(struct env_t *env, struct el_decl_t *decl)
{
	struct var_t *var;

	var = malloc(sizeof(struct var_t));
	var->decl = decl;
	avl_node_init(&var->node, decl->id);
	avl_root_add(&env->vars, &var->node);
}

/**
 * Find a variable in the environment.
 *   @env: The environment.
 *   @id: The variable identifier.
 *   &returns: The declaration or null.
 */
static struct el_decl_t *env_var_find(struct env_t *env, const char *id)
{
	struct avl_node_t *node;

	while(env != NULL) {
		node = avl_root_get(&env->vars, id);
		if(node != NULL)
			return getparent(node, struct var_t, node)->decl;

		env = env->up;
	}

	return NULL;
}


/**
 * Add a type to the environment.
 *   @env: The environment.
 *   @ref: The reference.
 */
static void env_type_add(struct env_t *env, struct el_tydecl_t *tydecl)
{
	struct type_t *type;

	type = malloc(sizeof(struct type_t));
	type->tydecl = tydecl;
	avl_node_init(&type->node, tydecl->id);
	avl_root_add(&env->types, &type->node);
}

/**
 * Find a type in the environment.
 *   @env: The environment.
 *   @id: The type identifier.
 *   &returns: The declaration or null.
 */
static struct el_tydecl_t *env_type_find(struct env_t *env, const char *id)
{
	struct avl_node_t *node;

	while(env != NULL) {
		node = avl_root_get(&env->types, id);
		if(node != NULL)
			return getparent(node, struct type_t, node)->tydecl;

		env = env->up;
	}

	return NULL;
}


/**
 * Initialize a custom type checker.
 *   @ref: The reference.
 *   @iface: The interface.
 *   &returns: The checker.
 */
struct el_check_t el_check_init(void *ref, const struct el_check_i *iface)
{
	return (struct el_check_t){ ref, iface };
}

/**
 * Coipy a checker.
 *   @check: The original checker.
 *   &returns: The copy.
 */
struct el_check_t el_check_copy(struct el_check_t check)
{
	return el_check_init(check.iface->copy ? check.iface->copy(check.ref) : NULL, check.iface);
}

/**
 * Delete a checker.
 *   @check: The checker.
 */
void el_check_delete(struct el_check_t check)
{
	if(check.iface->delete)
		check.iface->delete(check.ref);
}

/**
 * Process a custom checker.
 *   @check: The checker.
 *   @func: The function.
 *   @arg: The argument.
 *   &returns: Error.
 */
char *el_check_proc(struct el_check_t check, struct el_func_t *func, struct el_arg_t *arg)
{
	return check.iface->proc ? check.iface->proc(check.ref, func, arg) : NULL;
}



/*
 * local declarations
 */
static char *none_check(void *ref, struct el_func_t *func, struct el_arg_t *arg);
static struct el_check_i none_iface = { none_check, NULL, NULL };

static char *syscall_check(void *ref, struct el_func_t *func, struct el_arg_t *arg);
static struct el_check_i syscall_iface = { syscall_check, NULL, NULL };


/**
 * Create no custom checker.
 *   &returns: The checker.
 */
struct el_check_t el_check_none(void)
{
	return el_check_init(NULL, &none_iface);
}

/**
 * Check a system call.
 *   @ref: Reference.
 *   @func: The function type.
 *   @arg: The argument list.
 *   &returns: Error.
 */
static char *none_check(void *ref, struct el_func_t *func, struct el_arg_t *arg)
{
	return NULL;
}


/**
 * Create a syscall checker.
 *   &return: The checker.
 */
struct el_check_t el_check_syscall(void)
{
	return el_check_init(NULL, &syscall_iface);
}

/**
 * Check a system call.
 *   @ref: Reference.
 *   @func: The function type.
 *   @arg: The argument list.
 *   &returns: Error.
 */
static char *syscall_check(void *ref, struct el_func_t *func, struct el_arg_t *arg)
{
	/*
	struct el_arg_t *iter;

	for(iter = arg; iter != NULL; iter = iter->next) {
		if(iter->expr->type.kind == el_num_v)
			fill_expr(iter->expr, el_type_i64());
	}
	*/

	return NULL;
}

char *el_error(struct el_tag_t *tag, const char *fmt, ...)
{
	char *tmp, *err;
	va_list args;

	va_start(args, fmt);
	tmp = vmprintf(fmt, args);
	va_end(args);

	err = mprintf("%s:%u:%u: %s", tag->file->buf, tag->line + 1, tag->col + 1, tmp);
	free(tmp);

	return err;
}



/**
 * Post-process a type.
 *   @type: The type.
 */
void post_type(struct el_type_t *type)
{
	struct el_type_t base;

	if(type->kind != el_alias_v)
		return;

	base = *type;
	do
		base = base.data.alias->type;
	while(base.kind == el_alias_v);

	el_type_replace(type, el_type_copy(base));
}
