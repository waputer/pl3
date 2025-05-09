#include "common.h"


struct label_t {
	uint32_t num;
	struct el_label_t *ref;

	struct label_t *next;
};

/**
 * Buffer structure.
 *   @arr: The data array.
 *   @len, cap: The length and maximum capacity.
 *   @env: The environment.
 */
struct buf_t {
	char *arr;
	uint32_t len, cap;
	struct env_t *env;
};

/**
 * LLVM environment.
 *   @global, local: The global and local reference trees.
 *   @file: The output file.
 *   @reg, lbl, dbg: The register, label, and debug IDs.
 *   @vars, code, ext: The variables, code, and extra buffers.
 */
struct env_t {
	struct avl_root_t global, local;

	FILE *file;
	uint32_t reg, lbl, dbg, str, prog;
	struct buf_t vars, code, ext;
	struct el_func_t *func;
	struct label_t *label;

	struct queue_t *queue;
	struct queue_t **iqueue;
};

/**
 * Reference structure.
 *   @id: The identifier.
 *   @decl: The declaration.
 *   @node: The tree node.
 */
struct ref_t {
	uint32_t id;
	struct el_decl_t *decl;

	struct avl_node_t node;
};

struct handler_t {
	struct el_block_t *block;

	struct handler_t *next;
};

/**
 * Frame structure.
 *   @block: The current block.
 *   @cont, brk: The continue and break labels.
 *   @up: The previous (parent/up) block.
 */
struct frame_t {
	int32_t cont, brk;
	struct el_block_t *block;

	struct handler_t *handler;

	struct frame_t *up;
};

/**
 * Function list.
 *   @decl: The declaration.
 *   @next: The next function.
 */
struct queue_t {
	struct el_decl_t *decl;

	struct queue_t *next;
};


/*
 * local declarations
 */
static char *emit_top(struct el_decl_t *decl, struct env_t *env);
static char *emit_func(struct el_decl_t *decl, struct env_t *env);
static char *emit_block(struct el_block_t *block, struct env_t *env, struct frame_t *frame, int32_t brk, int32_t cont);
static char *emit_stmt(struct el_stmt_t *stmt, struct env_t *env, struct frame_t *frame);
static char *emit_expr(struct el_expr_t *expr, struct env_t *env, struct frame_t *frame, uint32_t *out);
static char *emit_left(struct el_expr_t *expr, struct env_t *env, struct frame_t *frame, uint32_t *out);

uint32_t gen_alloc(struct env_t *env, struct el_type_t type);

static const char *str_op2(enum el_expr_e kind, struct el_type_t type);

static void ref_add(struct avl_root_t *root, struct el_decl_t *decl, uint32_t id);
static uint32_t ref_get(struct avl_root_t *root, struct el_decl_t *decl);

/*
 * buffer declarations
 */
static struct buf_t buf_new(struct env_t *env);
static void buf_done(struct buf_t buf, FILE *file);

static void buf_ch(struct buf_t *str, char ch);
static void buf_str(struct buf_t *buf, const char *str);
static void buf_int(struct buf_t *buf, int64_t val);
static void buf_uint(struct buf_t *buf, uint64_t val);
static void buf_type(struct buf_t *buf, struct el_type_t type);
static void buf_expr(struct buf_t *buf, struct el_expr_t *expr);
static void buf_const(struct buf_t *buf, struct el_const_t *vconst);

static void bprintf(struct buf_t *buf, const char *fmt, ...);
static void bprintfv(struct buf_t *buf, const char *fmt, va_list args);

static uint32_t env_label(struct env_t *env, struct el_label_t *ref);

static char *escape(const char *str);

const char *ptype = "i64";


/**
 * Save the top to an LLVM file.
 *   @top: The top.
 *   @path: The output path.
 *   &returns: Error.
 */
char *el_llvm_save(struct el_top_t *top, const char *path)
{
	char *err;
	FILE *file;
	
	file = fopen(path, "w");
	if(file == NULL)
		return mprintf("Failed to open '%s' for writing. %s.", path, strerror(errno));

	err = el_llvm_write(top, file);
	fclose(file);

	return err;
}

/**
 * Write the top to an LLVM file.
 *   @top: The top.
 *   @file: The output file.
 *   &returns: Error.
 */
char *el_llvm_write(struct el_top_t *top, FILE *file)
{
	uint32_t i;
	struct env_t env;
	struct el_stmt_t *stmt;
	struct el_unit_t *unit;

	onerr( );

	env.str = 0;
	env.dbg = 3;
	env.file = file;
	env.ext = buf_new(&env);
	env.global = avl_root_init(avl_cmp_ptr);
	env.queue = NULL;
	env.iqueue = &env.queue;

	//fprintf(file, "declare dso_local void @fatal(i8* %%p0, ...)\n");

	i = 0;
	for(unit = top->unit; unit != NULL; unit = unit->next) {
		for(stmt = unit->block->head; stmt != NULL; stmt = stmt->next) {
			if(stmt->type == el_decl_v)
				ref_add(&env.global, stmt->data.decl, i++);
			else if(stmt->type != el_tydecl_v)
				fail("You can only have declarations at the top level.");
		}
	}

	fprintf(file, "target triple = \"x86_64-pc-linux-gnu\"\n");
	fprintf(file, "%%struct.__va_list_tag = type { i32, i32, ptr, ptr }\n");
	fprintf(file, "declare void @_getarg(i8*, i32, i8*)\n");

	for(unit = top->unit; unit != NULL; unit = unit->next) {
		for(stmt = unit->block->head; stmt != NULL; stmt = stmt->next) {
			if(stmt->type != el_decl_v)
				continue;

			chkret(emit_top(stmt->data.decl, &env));
		}
	}

	while(env.queue != NULL) {
		struct queue_t *tmp;

		emit_top(env.queue->decl, &env);
		env.queue = (tmp = env.queue)->next;
		free(tmp);
	}

	const char extra[] =
		"declare void @llvm.va_start(i8*)\n"
		"declare dso_local void @abort()\n"
		"define hidden i64 @$syscall(i64, i64, i64, i64, i64, i64, i64) local_unnamed_addr #0 {\n"
		"  %8 = tail call i64 asm sideeffect \"syscall\", \"={ax},{ax},{di},{si},{dx},{r10},{r8},{r9},~{rcx},~{r11},~{memory},~{dirflag},~{fpsr},~{flags}\"(i64 %0, i64 %1, i64 %2, i64 %3, i64 %4, i64 %5, i64 %6) #1\n"
		"  ret i64 %8\n"
		"}\n";

	fputs(extra, file);

	fprintf(file, "!llvm.module.flags = !{!0}\n");
	fprintf(file, "!llvm.dbg.cu = !{!2}\n");
	fprintf(file, "!0 = !{i32 2, !\"Debug Info Version\", i32 3}\n");
	fprintf(file, "!1 = !DIFile(filename: \"bar.c\", directory: \".\")\n");
	fprintf(file, "!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, emissionKind: FullDebug)\n");

	buf_done(env.ext, file);
	avl_root_destroy(&env.global, offsetof(struct ref_t, node), free);

	return NULL;
}


static struct buf_t *buf_hack;
static uint32_t *str_hack;

/**
 * Emit a top-level declaration.
 *   @decl: The declaration.
 *   @env: The environment.
 *   &returns: Error.
 */
static char *emit_top(struct el_decl_t *decl, struct env_t *env)
{
	onerr( );

	uint32_t id;

	if(decl->type.kind == el_func_v)
		return emit_func(decl, env);

	env->code = buf_new(env);

	id = ref_get(&env->global, decl);
	if(decl->expr != NULL) {
		buf_hack = &env->ext; 
		str_hack = &env->str;
		bprintf(&env->code, "@\"$g%u\" = global %t %e\n", id, decl->type, decl->expr);
	}
	else
		bprintf(&env->code, "@\"$g%u\" = global %t zeroinitializer\n", id, decl->type);

	if(decl->flags & EL_EXP)
		bprintf(&env->ext, "@\"%s\" = dso_local alias %t, %t* @\"$g%u\"\n", decl->id, decl->type, decl->type, id);
	else if(decl->flags & EL_PUB)
		bprintf(&env->ext, "@\"%s\" = hidden alias %t, %t* @\"$g%u\"\n", decl->id, decl->type, decl->type, id);

	buf_done(env->code, env->file);

	return NULL;
}


/**
 * Emit a function.
 *   @decl: The declaration. Must be a function with body.
 *   @env: The environment.
 *   &returns: Error.
 */
static char *emit_func(struct el_decl_t *decl, struct env_t *env)
{
	assert(decl->type.kind == el_func_v);

	uint32_t i, id, prog;
	struct el_body_t *body;

	onerr( );

	env->func = decl->type.data.func;

	if(decl->expr == NULL) {
		env->code = buf_new(env);
		bprintf(&env->code, "declare %t @\"%s\"(", env->func->ret, decl->id);
		for(i = 0; i < env->func->cnt; i++)
			bprintf(&env->code, "%s%t %%p%u", (i != 0) ? ", " : "", env->func->param[i].type, i);
		if(env->func->var)
			bprintf(&env->code, "%s...", (i != 0) ? ", " : "");
		bprintf(&env->code, ")\n");
		buf_done(env->code, env->file);
		return NULL;
	}
	else if(decl->expr->kind != el_body_v)
		fail("Functions must have a body.");

	id = ref_get(&env->global, decl);
	env->prog = prog = env->dbg++;
	bprintf(&env->ext, "!%u = distinct !DISubprogram(name: \"%s\", scope: !1, file: !1, unit: !2, type: !DISubroutineType(types: !{null}))\n", prog, decl->id);

	if(decl->flags & EL_EXP)
		bprintf(&env->ext, "@\"%s\" = dso_local alias %t, %t* @\"$g%u\"\n", decl->id, decl->type, decl->type, id);
	else if(decl->flags & EL_PUB)
		bprintf(&env->ext, "@\"%s\" = hidden alias %t, %t* @\"$g%u\"\n", decl->id, decl->type, decl->type, id);

	body = decl->expr->data.body;

	env->code = buf_new(env);
	bprintf(&env->code, "define hidden %t @\"$g%u\"(", body->func->ret, id);
	for(i = 0; i < body->func->cnt; i++)
		bprintf(&env->code, "%s%t %%p%u", (i != 0) ? ", " : "", body->func->param[i].type, i);
	if(body->func->var)
		bprintf(&env->code, "%s...", (i != 0) ? ", " : "");
	bprintf(&env->code, ") %s !dbg !%u { ; %s:%u\n", (strcmp(decl->id, "fatal") == 0) ? "noreturn" : "", prog, decl->tag->file->buf, decl->tag->line + 1);
	bprintf(&env->code, "entry:\n");
	buf_done(env->code, env->file);

	env->vars = buf_new(env);
	env->code = buf_new(env);
	env->local = avl_root_init(avl_cmp_ptr);

	for(i = 0; i < body->func->cnt; i++) {
		bprintf(&env->vars, "  %%r%u = alloca %t\n", i, body->func->param[i].type);
		bprintf(&env->code, "  store %t %%p%u, %t* %%r%u\n", body->func->param[i].type, i, body->func->param[i].type, i);
	}

	env->lbl = 0;
	env->reg = body->func->cnt;
	env->label = NULL;

	chkret(emit_block(body->block, env, NULL, -1, -1));

	if(el_type_is_void(body->func->ret))
		bprintf(&env->code, "  ret void\n");
	else if(el_type_is_err(env->func->ret)) {
		if(el_type_is_void(el_err_get_val(env->func->ret)))
			bprintf(&env->code, "  ret %t <{ i8* null }>\n", env->func->ret);
		else
			bprintf(&env->code, "  unreachable\n");
	}
	else
		bprintf(&env->code, "  unreachable\n");

	avl_root_destroy(&env->local, offsetof(struct ref_t, node), free);
	buf_done(env->vars, env->file);
	buf_done(env->code, env->file);

	fprintf(env->file, "}\n");

	struct label_t *label;

	while(env->label != NULL) {
		env->label = (label = env->label)->next;
		free(label);
	}

	return NULL;
}

/**
 * Emit a block.
 *   @block: The block.
 *   @env: The environment.
 *   @frame: The frame.
 *   @brk: The break label.
 *   @cont: The continue label.
 *   &returns: Error.
 */
static char *emit_block(struct el_block_t *block, struct env_t *env, struct frame_t *frame, int32_t brk, int32_t cont)
{
	struct frame_t next;
	struct el_stmt_t *stmt;

	next.up = frame;
	next.brk = brk;
	next.cont = cont;
	next.block = block;
	next.handler = NULL;

	onerr( );

	for(stmt = block->head; stmt != NULL; stmt = stmt->next)
		chkret(emit_stmt(stmt, env, &next));

	struct handler_t *handler;

	while(next.handler != NULL) {
		next.handler = (handler = next.handler)->next;
		free(handler);
	}

	return NULL;
}

/**
 * Emit a statement.
 *   @stmt: The statement.
 *   @env: The environment.
 *   @frame: The frame.
 *   &returns: Error.
 */
static char *emit_stmt(struct el_stmt_t *stmt, struct env_t *env, struct frame_t *frame)
{
	onerr( );

	switch(stmt->type) {
	case el_decl_v: {
		uint32_t id = env->reg++;
		struct el_decl_t *decl = stmt->data.decl;

		if(decl->type.kind == el_func_v || ((decl->flags & EL_MUT) == 0)) {
			ref_add(&env->global, decl, env->global.count);
			*env->iqueue = malloc(sizeof(struct queue_t));
			**env->iqueue = (struct queue_t){ decl, NULL };
			env->iqueue = &(*env->iqueue)->next;

			bprintf(&env->code, "  %%r%u = bitcast %t* @$g%u to %t*\n", id, decl->type, ref_get(&env->global, decl), decl->type);
		}
		else {
			bprintf(&env->vars, "  %%r%u = alloca %t\n", id, decl->type);

			if(decl->expr != NULL) {
				uint32_t val;

				chkret(emit_expr(decl->expr, env, frame, &val));

				if(decl->type.kind == el_ptr_v) {
					uint32_t tmp = val;

					val = env->reg++;
					bprintf(&env->code, "  %%r%u = bitcast %t %%r%u to %t\n", val, decl->expr->type, tmp, decl->type);
				}

				bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", decl->type, val, decl->type, id);
			}

			/*
			   if(el_type_is_ptr(decl->type)) {
			   uint32_t tmp;

			   tmp = env->reg++;
			   bprintf(&env->code, "  %%r%u = bitcast i8** %%r%u to i8*\n", tmp, id);

			   id = tmp;
			   }
			 */
		}

		ref_add(&env->local, decl, id);
	} break;

	case el_expr_v: {
		uint32_t reg;

		chkret(emit_expr(stmt->data.expr, env, frame, &reg));
	} break;

	case el_return_v: {
		struct el_expr_t *expr = stmt->data.expr;

		if(stmt->data.expr != NULL) {
			uint32_t val;

			chkret(emit_expr(stmt->data.expr, env, frame, &val));

			if(el_type_is_void(env->func->ret))
				bprintf(&env->code, "  ret void\n");
			else
				bprintf(&env->code, "  ret %t %%r%u\n", expr->type, val);
		}
		else if(el_type_is_err(env->func->ret)) {
			if(el_type_is_void(el_err_get_val(env->func->ret)))
				bprintf(&env->code, "  ret %t <{ %t zeroinitializer }>\n", env->func->ret, el_err_get_err(env->func->ret));
			else
				failmsg(el_error(stmt->tag, "Invalid return type"));
		}
		else
			bprintf(&env->code, "  ret void\n");
	} break;

	case el_eret_v: {
		struct el_expr_t *expr = stmt->data.expr;

		uint32_t val = 0;
		struct handler_t *handler;
		struct frame_t *iter;

		if(stmt->data.expr != NULL)
			chkret(emit_expr(stmt->data.expr, env, frame, &val));

		for(iter = frame; iter != NULL; iter = iter->up) {
			for(handler = iter->handler; handler != NULL; handler = handler->next)
				emit_block(handler->block, env, frame, -1, -1);
		}

		if(stmt->data.expr != NULL)
			bprintf(&env->code, "  ret %t %%r%u\n", expr->type, val);
		else
			bprintf(&env->code, "  ret void\n");
	} break;

	case el_cond_v: {
		uint32_t eval, ontrue, onfalse, after;
		struct el_cond_t *cond = stmt->data.cond;

		chkret(emit_expr(cond->eval, env, frame, &eval));

		after = env->lbl++;
		ontrue = cond->ontrue ? env->lbl++ : after;
		onfalse = cond->onfalse ? env->lbl++ : after;

		bprintf(&env->code, "  br i1 %%r%u, label %%l%u, label %%l%u\n", eval, ontrue, onfalse);

		if(cond->ontrue != NULL) {
			bprintf(&env->code, "l%u:\n", ontrue);
			chkret(emit_block(cond->ontrue, env, frame, -1, -1));
			bprintf(&env->code, "  br label %%l%u\n", after);
		}

		if(cond->onfalse != NULL) {
			bprintf(&env->code, "l%u:\n", onfalse);
			chkret(emit_block(cond->onfalse, env, frame, -1, -1));
			bprintf(&env->code, "  br label %%l%u\n", after);
		}

		bprintf(&env->code, "l%u:\n", after);
	} break;

	case el_loop_v: {
		uint32_t ign, cond, inc, begin, end, body;
		struct el_loop_t *loop = stmt->data.loop;

		if(loop->init != NULL)
			chkret(emit_expr(loop->init, env, frame, &ign));

		inc = env->lbl++;
		begin = env->lbl++;
		end = env->lbl++;

		if(loop->post) {
			bprintf(&env->code, "  br label %%l%u\n", begin);
			bprintf(&env->code, "l%u:\n", begin);

			chkret(emit_block(loop->body, env, frame, end, begin));

			if(loop->cond != NULL) {
				chkret(emit_expr(loop->cond, env, frame, &cond));
				bprintf(&env->code, "  br i1 %%r%u, label %%l%u, label %%l%u\n", cond, begin, end);
				bprintf(&env->code, "l%u:\n", end);
			}
		}
		else {
			bprintf(&env->code, "  br label %%l%u\n", begin);
			bprintf(&env->code, "l%u:\n", begin);

			if(loop->cond != NULL) {
				body = env->lbl++;
				chkret(emit_expr(loop->cond, env, frame, &cond));
				bprintf(&env->code, "  br i1 %%r%u, label %%l%u, label %%l%u\n", cond, body, end);
				bprintf(&env->code, "l%u:\n", body);
			}

			chkret(emit_block(loop->body, env, frame, end, inc));

			bprintf(&env->code, "  br label %%l%u\n", inc);
			bprintf(&env->code, "l%u:\n", inc);

			if(loop->inc != NULL)
				chkret(emit_expr(loop->inc, env, frame, &ign));

			bprintf(&env->code, "  br label %%l%u\n", begin);
			bprintf(&env->code, "l%u:\n", end);
		}
	} break;

	case el_brk_v: {
		uint32_t cnt = 0;
		struct frame_t *iter;

		for(iter = frame; iter != NULL; iter = iter->up) {
			if(iter->brk >= 0) {
				if(cnt-- == 0)
					break;
			}
		}

		if(iter == NULL)
			failmsg(el_error(stmt->tag, "Invalid break level."));

		bprintf(&env->code, "  br label %%l%u\n", iter->brk);
		bprintf(&env->code, "l%u:\n", env->lbl++);
	} break;

	case el_cont_v: {
		uint32_t cnt = 0;
		struct frame_t *iter;

		for(iter = frame; iter != NULL; iter = iter->up) {
			if(iter->cont >= 0) {
				if(cnt-- == 0)
					break;
			}
		}

		if(iter == NULL)
			failmsg(el_error(stmt->tag, "Invalid continue level."));

		bprintf(&env->code, "  br label %%l%u\n", iter->cont);
		bprintf(&env->code, "l%u:\n", env->lbl++);
	} break;

	case el_switch_v: {
		struct el_case_t *vcase, *vdef;
		struct el_switch_t *sw = stmt->data.sw;
		uint32_t eval, lab, def, after;

		chkret(emit_expr(sw->eval, env, frame, &eval));

		for(vdef = sw->vcase; vdef != NULL; vdef = vdef->next) {
			if(vdef->expr == NULL) {
				def = env_label(env, vdef->label);
				break;
			}
		}

		after = env->lbl++;
		if(vdef == NULL)
			def = after;

		bprintf(&env->code, "  switch %t %%r%u, label %%l%u [ ", sw->eval->type, eval, def);
		for(vcase = sw->vcase; vcase != NULL; vcase = vcase->next) {
			struct el_expr_t *expr = vcase->expr;

			if(expr == NULL)
				continue;

			while(expr->kind == el_ident_v) {
				struct el_decl_t *decl;
				struct el_ident_t *ident = expr->data.ident;

				if((ident->type != el_local_v) && (ident->type != el_global_v))
					fatal("Case values can only be local or global constants.");

				decl = ident->data.decl;
				if(decl->flags & EL_MUT)
					fatal("Case values must be constant.");

				expr = decl->expr;
			}

			if(expr->kind == el_const_v) {
				lab = env_label(env, vcase->label);
				bprintf(&env->code, "%t %v, label %%l%u ", expr->type, expr->data.vconst, lab);
			}
			else if(expr->kind == el_enval_v) {
				lab = env_label(env, vcase->label);
				bprintf(&env->code, "%t %d, label %%l%u ", expr->type, (int)expr->data.enval->val, lab);
			}
			else
				fatal("Case values must be constant.");
		}
		bprintf(&env->code, "]\n");

		chkret(emit_block(sw->block, env, frame, after, -1));

		bprintf(&env->code, "  br label %%l%u\n", after);
		bprintf(&env->code, "l%u:\n", after);
	} break;

	case el_label_v: {
		uint32_t num;
		struct el_label_t *label = stmt->data.label;

		num = env_label(env, label);

		bprintf(&env->code, "  br label %%l%u\n", num);
		bprintf(&env->code, "l%u:\n", num);
	} break;

	case el_onerr_v: {
		struct handler_t *handler;

		handler = malloc(sizeof(struct handler_t));
		handler->block = stmt->data.block;
		handler->next = frame->handler;
		frame->handler = handler;
	} break;

	case el_block_v: {
		chkret(emit_block(stmt->data.block, env, frame, frame->brk, frame->cont));
	} break;

	default:
		fatal("stub stmt %d\n", stmt->type);
		break;
	}

	return NULL;
}

static char *emit_init(struct el_init_t *init, struct el_type_t type, struct env_t *env, struct frame_t *frame, uint32_t *out)
{
	onerr();

	if(type.kind == el_struct_v) {
		uint32_t tmp, val, agg = 0;

		agg = env->reg++;
		bprintf(&env->code, "  %%r%u = insertvalue %t undef, %t undef, 0\n", agg, type, el_struct_get(type.data.vstruct, 0)->type);

		while(init != NULL) {
			struct el_type_t sub = el_struct_get(type.data.vstruct, init->idx)->type;

			if(init->nest)
				chkret(emit_init(init->data.nest, sub, env, frame, &val));
			else
				chkret(emit_expr(init->data.expr, env, frame, &val));

			tmp = env->reg++;
			bprintf(&env->code, "  %%r%u = insertvalue %t %%r%u, %t %%r%u, %u\n", tmp, type, agg, sub, val, init->idx);
			agg = tmp;

			init = init->next;
		}

		*out = agg;
	}
	else if(type.kind == el_union_v) {
		uint32_t mem, src, sz = el_type_size(type);

		mem = env->reg++;
		bprintf(&env->vars, "  %%r%u = alloca i%u\n", mem, sz);

		if(init != NULL) {
			int32_t idx;
			uint32_t val;
			struct el_type_t sub;
			struct el_struct_t *vstruct = type.data.vstruct;

			if(init->id == NULL)
				fatal("Can initialize unions by name.");

			idx = el_struct_find(vstruct, init->id);
			if(idx < 0)
				fatal("Unknown member '%s'.", init->id);

			sub = el_struct_get(vstruct, idx)->type;

			src = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i%u* %%r%u to %t*\n", src, sz, mem, sub);

			if(init->nest)
				chkret(emit_init(init->data.nest, sub, env, frame, &val));
			else
				chkret(emit_expr(init->data.expr, env, frame, &val));

			bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", sub, val, sub, src);
		}

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = load i%u, i%u* %%r%u\n", *out, sz, sz, mem);
	}
	else
		fatal("Unknown");

	return NULL;
}

/**
 * Emit an expression.
 *   @expr: The expression.
 *   @env: The environment.
 *   @out: Out. The result register.
 *   &returns: Error.
 */
static char *emit_expr(struct el_expr_t *expr, struct env_t *env, struct frame_t *frame, uint32_t *out)
{
	onerr( );

	switch(expr->kind) {
	case el_const_v: {
		struct el_const_t *vconst = expr->data.vconst;

		*out = env->reg++;

		if(vconst->type == el_const_str_v) {
			uint32_t nam = env->str++, len = strlen(vconst->data.str) + 1;
			char *str = escape(vconst->data.str);

			bprintf(&env->ext, "  @s%u = hidden local_unnamed_addr constant [%u x i8] c\"%s\\00\", align 1\n", nam, len, str);
			bprintf(&env->code, "  %%r%u = getelementptr inbounds [%u x i8], [%u x i8]* @s%u, i64 0, i64 0\n", *out, len, len, nam);

			free(str);
		}
		else
			bprintf(&env->code, "  %%r%u = bitcast %t %v to %t\n", *out, expr->type, vconst, expr->type);
	} break;

	case el_init_v: {
		chkret(emit_init(expr->data.init, expr->type, env, frame, out));
	} break;

	case el_call_v: {
		struct el_arg_t *arg;
		struct el_func_t *func;
		struct el_call_t *call = expr->data.call;
		uint32_t i, ptr = 0, lst[call->cnt];

		for(i = 0, arg = call->arg; arg != NULL; i++, arg = arg->next)
			chkret(emit_expr(arg->expr, env, frame, &lst[i]));

		if(call->deref) {
			uint32_t tmp;
			struct el_type_t type = el_type_root(call->func->type);
			if(type.kind != el_ptr_v)
				fatal("Cannot derefeence non-pointer type");

			type = el_type_root(type.data.ptr->sub);
			if(type.kind != el_func_v) {
				char str[256];
				el_type_tostr(call->func->type, str, sizeof(str));
				fatal(el_error(expr->tag, "Cannot call non-function type (%s).", str));
			}

			chkret(emit_expr(call->func, env, frame, &tmp));

			ptr = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to %t*\n", ptr, tmp, type);

			func = type.data.func;
		}
		else {
			chkret(emit_left(call->func, env, frame, &ptr));

			if(call->func->type.kind != el_func_v)
				fatal("Cannot call non-function type.");

			func = call->func->type.data.func;
		}
			
		//if(call->func->type.kind != el_func_v)
			//fatal("Must call a function.");

		if(!el_type_is_void(func->ret)) {
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = call %t %%r%u(", *out, func->var ? el_type_func(func) : func->ret, ptr);
		}
		else if(func->var)
			bprintf(&env->code, "  call %t %%r%u(", el_type_func(func), ptr);
		else
			bprintf(&env->code, "  call void %%r%u(", ptr);

		for(i = 0, arg = call->arg; arg != NULL; i++, arg = arg->next)
			bprintf(&env->code, "%s%t %%r%u", (i > 0) ? ", " : "", arg->expr->type, lst[i]);

		uint32_t dbg = env->dbg++;
		bprintf(&env->ext, "!%u = distinct !DILocation(line: %u, column: %u, scope: !%u)\n", dbg, expr->tag->line, expr->tag->col, env->prog);
		bprintf(&env->code, "), !dbg !%u\n", dbg);
	} break;

	case el_cast_v: {
		uint32_t val;
		struct el_expr_t *cast = expr->data.op1;
		struct el_type_t dst = expr->type, src = cast->type;

		chkret(emit_expr(cast, env, frame, &val));
		*out = env->reg++;

		if((src.kind == el_ptr_v) && (dst.kind == el_ptr_v))
			bprintf(&env->code, "  %%r%u = bitcast %t %%r%u to %t\n", *out, src, val, dst);
		else if(src.kind == el_ptr_v)
			bprintf(&env->code, "  %%r%u = ptrtoint %t %%r%u to %t\n", *out, src, val, dst);
		else if(dst.kind == el_ptr_v)
			bprintf(&env->code, "  %%r%u = inttoptr %t %%r%u to %t\n", *out, src, val, dst);
		else if(el_type_size(dst) > el_type_size(src))
			bprintf(&env->code, "  %%r%u = %cext %t %%r%u to %t\n", *out, el_type_is_signed(dst) ? 's' : 'z', src, val, dst);
		else if(el_type_size(dst) < el_type_size(src))
			bprintf(&env->code, "  %%r%u = trunc %t %%r%u to %t\n", *out, src, val, dst);
		else
			*out = val;
	} break;

	case el_ident_v: {
		struct el_ident_t *ident = expr->data.ident;

		switch(ident->type) {
		case el_unk_v:
			fail("Unknown identifier '%s'.", ident->str);
			break;

		case el_local_v:
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", *out, expr->type, expr->type, ref_get(&env->local, ident->data.decl));
			break;

		case el_global_v:
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* @$g%u\n", *out, expr->type, expr->type, ref_get(&env->global, ident->data.decl));
			break;

		case el_param_v:
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", *out, expr->type, expr->type, ident->data.off);
			break;

		case el_builtin_v:
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* @$g%s\n", *out, expr->type, expr->type, expr->data.ident->str + 3);
			break;
		}
	} break;

	case el_elem_v: {
		uint32_t tmp, base;
		struct el_type_t type;
		struct el_elem_t *elem = expr->data.elem;

		chkret(emit_expr(elem->base, env, frame, &base));

		type = elem->base->type;
		if(elem->deref) {
			assert(type.kind == el_ptr_v);

			type = type.data.ptr->sub;
			tmp = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to %t*\n", tmp, base, type);
			base = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", base, type, type, tmp);
		}

		if(el_type_root(type).kind == el_union_v) {
			uint32_t mem, src, dst, size;

			size = el_type_size(type);

			mem = env->reg++;
			bprintf(&env->vars, "  %%r%u = alloca i%u\n", mem, size);

			src = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i%u* %%r%u to %t* ; size %d\n", src, size, mem, elem->base->type, size);

			dst = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i%u* %%r%u to %t*\n", dst, size, mem, expr->type);

			bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", elem->base->type, base, elem->base->type, src);

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", *out, expr->type, expr->type, dst);
		}
		else {
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = extractvalue %t %%r%u, %u\n", *out, type, base, elem->idx);
		}
	} break;

	case el_index_v: {
		struct el_index_t *index = expr->data.index;

		if(index->base->type.kind == el_ptr_v) {
			uint32_t base, ptr, off, tmp;
			struct el_type_t type = index->base->type.data.ptr->sub;

			chkret(emit_expr(index->base, env, frame, &base));
			chkret(emit_expr(index->off, env, frame, &off));

			/*
			ptr = env->reg++;
			bprintf(&env->code, "  %%r%u = ptrtoint i8* %%r%u to %s\n", ptr, base, ptype);

			tmp = env->reg++;
			bprintf(&env->code, "  %%r%u = add %s %%r%u, %d\n", tmp, ptype, ptr, off * el_type_size(index->off->type) / 8);

			ptr = env->reg++;
			bprintf(&env->code, "  %%r%u = inttoptr %s %%r%u to %t*\n", ptr, ptype, tmp, type);

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", *out, type, type, ptr);
			*/

			ptr = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to %t*\n", ptr, base, type);

			tmp = env->reg++;
			bprintf(&env->code, "  %%r%u = getelementptr %t, %t* %%r%u, %t %%r%u\n", tmp, type, type, ptr, index->off->type, off);

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", *out, type, type, tmp);
		}
		else if(index->base->type.kind == el_arr_v) {
			uint32_t base, off, var, ptr;
			struct el_type_t type = index->base->type;

			chkret(emit_expr(index->base, env, frame, &base));
			chkret(emit_expr(index->off, env, frame, &off));

			var = env->reg++;
			bprintf(&env->vars, "  %%r%u = alloca %t\n", var, type);
			bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", type, base, type, var);

			ptr = env->reg++;
			bprintf(&env->code, "  %%r%u = getelementptr %t, %t* %%r%u, i64 0, %t %%r%u\n", ptr, type, type, var, index->off->type, off);

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", *out, expr->type, expr->type, ptr);
		}
		else {
			fatal("stub");
		}
	} break;

	case el_deref_v: {
		uint32_t tmp, cast;

		if(expr->data.op1->type.kind == el_ptr_v) {
			chkret(emit_expr(expr->data.op1, env, frame, &tmp));

			cast = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to %t*\n", cast, tmp, expr->data.op1->type.data.ptr->sub);

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u; type:%t*\n", *out, expr->type, expr->type, cast, expr->data.op1->type.data.ptr->sub);
		}
		else if(expr->data.op1->type.kind == el_arr_v) {
			chkret(emit_left(expr->data.op1, env, frame, &tmp));

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = getelementptr %t, %t* %%r%u, i32 0, i32 0\n", *out, expr->data.op1->type, expr->data.op1->type, tmp);
		}
		else
			fatal("uh-oh");
	} break;

	case el_neg_v: {
		uint32_t tmp;

		chkret(emit_expr(expr->data.op1, env, frame, &tmp));

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = sub %t 0, %%r%u\n", *out, expr->type, tmp);
	} break;

	case el_lnot_v: {
		uint32_t tmp;

		chkret(emit_expr(expr->data.op1, env, frame, &tmp));

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = xor %t %%r%u, true\n", *out, expr->type, tmp);
	} break;

	case el_add_v:
	case el_sub_v:
		if(el_type_is_ptr(expr->data.op2.lhs->type) && el_type_is_int(expr->data.op2.rhs->type)) {
			uint32_t ptr, res, lhs, rhs; // off, bytes; 
			struct el_type_t type;
			struct el_op2_t op2 = expr->data.op2;

			chkret(emit_expr(op2.lhs, env, frame, &lhs));
			chkret(emit_expr(op2.rhs, env, frame, &rhs));

			type = op2.lhs->type.data.ptr->sub;
			if(el_type_is_void(type))
				type = el_type_u8();

			/*
			ptr = env->reg++;
			bprintf(&env->code, "  %%r%u = ptrtoint i8* %%r%u to %s\n", ptr, lhs, ptype);

			off = env->reg++;
			bprintf(&env->code, "  %%r%u = xt %t %%r%u to %s\n", off, op2.rhs->type, rhs, ptype);

			bytes = env->reg++;
			bprintf(&env->code, "  %%r%u = mul %s %%r%u, %u\n", bytes, ptype, off, el_type_size(type) / 8);

			res = env->reg++;
			bprintf(&env->code, "  %%r%u = add %s %%r%u, %%r%u\n", res, ptype, ptr, bytes);

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = inttoptr %s %%r%u to i8*\n", *out, ptype, res);
			*/

			ptr = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to %t*\n", ptr, lhs, type);

			if(expr->kind == el_sub_v) {
				uint32_t tmp;

				tmp = env->reg++;
				bprintf(&env->code, "  %%r%u = sub %t 0, %%r%u\n", tmp, op2.rhs->type, rhs);
				rhs = tmp;
			}

			res = env->reg++;
			bprintf(&env->code, "  %%r%u = getelementptr %t, %t* %%r%u, %t %%r%u\n", res, type, type, ptr, op2.rhs->type, rhs);

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast %t* %%r%u to i8*\n", *out, type, res);

			break;
		}

	case el_eq_v:
	case el_ne_v:
	case el_gt_v:
	case el_gte_v:
	case el_lt_v:
	case el_lte_v:
	case el_mul_v:
	case el_div_v:
	case el_rem_v:
	case el_or_v:
	case el_xor_v:
	case el_and_v:
	case el_shl_v:
	case el_shr_v: {
		const char *name;
		uint32_t tmp, lhs, rhs;
		struct el_type_t type = expr->data.op2.lhs->type;
		bool ptr = el_type_is_ptr(expr->data.op2.rhs->type) && !el_is_cmp(expr->kind);

		name = str_op2(expr->kind, type);
		chkret(emit_expr(expr->data.op2.lhs, env, frame, &lhs));
		chkret(emit_expr(expr->data.op2.rhs, env, frame, &rhs));

		if(ptr) {
			tmp = env->reg++;
			bprintf(&env->code, "  %%r%u = ptrtoint i8* %%r%u to i64\n", tmp, lhs);
			lhs = tmp;

			tmp = env->reg++;
			bprintf(&env->code, "  %%r%u = ptrtoint i8* %%r%u to i64\n", tmp, rhs);
			rhs = tmp;

			type = el_type_u64();
		}

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = %s %t %%r%u, %%r%u\n", *out, name, type, lhs, rhs);

		if(ptr) {
			tmp = env->reg++;
			bprintf(&env->code, "  %%r%u = inttoptr i64 %%r%u to i8*\n", tmp, *out);
			*out = tmp;
		}
	} break;

	case el_land_v:
	case el_lor_v: {
		uint32_t tmp, lhs, rhs, enter, before, mid, after;
		struct el_type_t type = expr->data.op2.lhs->type;

		enter = env->lbl++;
		before = env->lbl++;
		mid = env->lbl++;
		after = env->lbl++;

		chkret(emit_expr(expr->data.op2.lhs, env, frame, &tmp));

		bprintf(&env->code, "  br label %%l%u\n", enter);
		bprintf(&env->code, "l%u:\n", enter);

		lhs = env->reg++;
		bprintf(&env->code, "  %%r%u = bitcast %t %%r%u to %t\n", lhs, type, tmp, type);

		if(expr->kind == el_land_v)
			bprintf(&env->code, "  br i1 %%r%u, label %%l%u, label %%l%u\n", lhs, before, after);
		else
			bprintf(&env->code, "  br i1 %%r%u, label %%l%u, label %%l%u\n", lhs, after, before);

		bprintf(&env->code, "l%u:\n", before);

		chkret(emit_expr(expr->data.op2.rhs, env, frame, &tmp));

		bprintf(&env->code, "  br label %%l%u\n", mid);
		bprintf(&env->code, "l%u:\n", mid);

		rhs = env->reg++;
		bprintf(&env->code, "  %%r%u = bitcast %t %%r%u to %t\n", rhs, type, tmp, type);

		bprintf(&env->code, "  br label %%l%u\n", after);
		bprintf(&env->code, "l%u:\n", after);

		*out = env->reg++;
		if(expr->kind == el_land_v)
			bprintf(&env->code, "  %%r%u = phi i1 [ false, %%l%u ], [ %%r%u, %%l%u ]\n", *out, enter, rhs, mid);
		else
			bprintf(&env->code, "  %%r%u = phi i1 [ true, %%l%u ], [ %%r%u, %%l%u ]\n", *out, enter, rhs, mid);
	} break;

	case el_assign_v: {
		uint32_t lhs, rhs;

		chkret(emit_left(expr->data.op2.lhs, env, frame, &lhs));
		chkret(emit_expr(expr->data.op2.rhs, env, frame, &rhs));

		*out = rhs;
		bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", expr->type, rhs, expr->type, lhs);
	} break;

	case el_addeq_v:
	case el_subeq_v:
		if(el_type_is_ptr(expr->data.op2.lhs->type) && el_type_is_int(expr->data.op2.rhs->type)) {
			assert(el_type_is_int(expr->data.op2.rhs->type));

			uint32_t tmp, lhs, rhs;
			struct el_type_t type;
			struct el_op2_t op2 = expr->data.op2;

			chkret(emit_left(op2.lhs, env, frame, &lhs));
			chkret(emit_expr(op2.rhs, env, frame, &rhs));

			type = op2.lhs->type.data.ptr->sub;
			if(el_type_is_void(type))
				type = el_type_u8();

			if(expr->kind == el_subeq_v) {
				tmp = env->reg++;
				bprintf(&env->code, "  %%r%u = sub %t 0, %%r%u\n", tmp, op2.rhs->type, rhs);
				rhs = tmp;
			}

			tmp = env->reg++;
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", tmp, expr->type, expr->type, lhs);
			bprintf(&env->code, "  %%r%u = getelementptr %t, %t %%r%u, %t %%r%u\n", *out, type, op2.lhs->type, tmp, op2.rhs->type, rhs);
			bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", expr->type, *out, expr->type, lhs);

			break;
		}

	case el_muleq_v:
	case el_diveq_v:
	case el_remeq_v:
	case el_shleq_v:
	case el_shreq_v:
	case el_andeq_v:
	case el_oreq_v:
	case el_xoreq_v: {
		const char *name;
		uint32_t tmp, lhs, rhs;

		name = str_op2(expr->kind, expr->type);
		chkret(emit_left(expr->data.op2.lhs, env, frame, &lhs));
		chkret(emit_expr(expr->data.op2.rhs, env, frame, &rhs));

		tmp = env->reg++;
		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", tmp, expr->type, expr->type, lhs);
		bprintf(&env->code, "  %%r%u = %s %t %%r%u, %%r%u\n", *out, name, expr->type, tmp, rhs);
		bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", expr->type, *out, expr->type, lhs);
	} break;

	case el_preinc_v:
	case el_predec_v:
	case el_postinc_v:
	case el_postdec_v: {
		const char *name;
		uint32_t tmp, lhs;

		name = str_op2(expr->kind, expr->type);
		chkret(emit_left(expr->data.op2.lhs, env, frame, &lhs));

		tmp = env->reg++;
		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", *out, expr->type, expr->type, lhs);

		if(el_type_is_int(expr->type))
			bprintf(&env->code, "  %%r%u = %s %t %%r%u, 1\n", tmp, name, expr->type, *out);
		else if(expr->type.kind == el_ptr_v)
			bprintf(&env->code, "  %%r%u = getelementptr %t, %t %%r%u, i32 %s\n", tmp, expr->type.data.ptr->sub, expr->type, *out, (expr->kind == el_postinc_v) ? "1 " : "-1");
		else
			fatal("Invalid increment.");

		bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", expr->type, tmp, expr->type, lhs);

		if((expr->kind == el_preinc_v) || (expr->kind == el_predec_v))
			*out = tmp;
	} break;

	case el_initargs_v: {
		uint32_t tmp;

		tmp = env->reg++;
		*out = env->reg++;

		bprintf(&env->vars, "  %%r%u = alloca %%struct.__va_list_tag\n", tmp);
		bprintf(&env->code, "  %%r%u = bitcast %%struct.__va_list_tag* %%r%u to i8*\n", *out, tmp);
		bprintf(&env->code, "  call void @llvm.va_start(i8* %%r%u)\n", *out);
	} break;

	case el_getarg_v: {
		uint32_t args, cast, ptr;
		struct el_type_t type = expr->data.getarg->type;

		chkret(emit_expr(expr->data.getarg->expr, env, frame, &args));

		cast = env->reg++;
		bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to i8**\n", cast, args);

		ptr = env->reg++;
		bprintf(&env->code, "  %%r%u = load i8*, i8** %%r%u\n", ptr, cast);

		uint32_t tmp, var, sz = el_type_size(type);

		var = env->reg++;
		bprintf(&env->vars, "  %%r%u = alloca i%u\n", var, sz);

		tmp = env->reg++;
		bprintf(&env->vars, "  %%r%u = bitcast i%u* %%r%u to i8*\n", tmp, sz, var);

		bprintf(&env->code, "  call void @_getarg(i8* %%r%u, i32 %u, i8* %%r%u)\n", ptr, sz, tmp);

		cast = env->reg++;
		bprintf(&env->code, "  %%r%u = bitcast i%u* %%r%u to %t*\n", cast, sz, var, type);

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = load %t, %t* %%r%u\n", *out, type, type, cast);
	} break;

	case el_addr_v: {
		chkret(emit_left(expr->data.op1, env, frame, out));

		//if(expr->data.op1->type.kind == el_struct_v) {
			uint32_t tmp = *out;

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast %t* %%r%u to i8*\n", *out, expr->data.op1->type, tmp);
		//}
	} break;

	case el_arrlen_v: {
		struct el_expr_t *op1 = expr->data.op1;

		assert(el_type_is_arr(op1->type));

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = add %t %u, 0\n", *out, expr->type, op1->type.data.arr->len);
	} break;

	case el_getref_v: {
		uint32_t val, tmp;
		struct el_expr_t *op1 = expr->data.op1;

		chkret(emit_expr(expr->data.op1, env, frame, &val));

		tmp = env->reg++;
		bprintf(&env->vars, "  %%r%u = alloca %t\n", tmp, op1->type);
		bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", op1->type, val, op1->type, tmp);

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = bitcast %t* %%r%u to %t\n", *out, op1->type, tmp, expr->type);
	} break;

	case el_offsetof_v: {
		struct el_struct_t *vstruct;
		struct el_offsetof_t *offset = expr->data.offset;
		int32_t idx;

		if(!el_type_is_struct(offset->type))
			fatal("%s:%u: Can only take an offset of a structure.", expr->tag->file->buf, expr->tag->line);

		vstruct = el_type_root(offset->type).data.vstruct;
		idx = el_struct_find(vstruct, offset->id);
		if(idx < 0)
			fatal("%s:%u: Unknown element '%s'.", expr->tag->file->buf, expr->tag->line, offset->id);

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = add %t %u, 0\n", *out, expr->type, (el_struct_offset(vstruct, idx) + 7) / 8);
	} break;

	case el_getparent_v: {
		uint32_t off, ptr, in;
		struct el_getparent_t *parent = expr->data.parent;

		chkret(emit_expr(parent->expr, env, frame, &in));

		ptr = env->reg++;
		bprintf(&env->code, "  %%r%u = bitcast %t %%r%u to i8*\n", ptr, parent->expr->type, in);

		off = env->reg++;
		bprintf(&env->code, "  %%r%u = getelementptr i8, i8* %%r%u, i32 %d\n", off, in, -parent->off);

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to %t\n", *out, off, expr->type);
	} break;

	case el_sizeof_v: {
		struct el_sizeof_t *size = expr->data.size;

		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = add %t %u, 0\n", *out, expr->type, (el_type_size(size->type) + 7) / 8);
	} break;

	case el_eget_v: {
		int32_t eret, evar;
		uint32_t val, err, pack, eval, onerr, onsuc;
		struct el_expr_t *op1 = expr->data.op1;
		struct el_type_t etype;
		struct el_struct_t *vstruct;

		vstruct = op1->type.data.vstruct;
		eret = el_struct_find(env->func->ret.data.vstruct, "err");
		evar = el_struct_find(op1->type.data.vstruct, "err");
		//vtype = el_struct_get(vstruct, 0)->type;
		etype = el_struct_get(vstruct, evar)->type;

		chkret(emit_expr(op1, env, frame, &val));

		err = env->reg++;
		bprintf(&env->code, "  %%r%u = extractvalue %t %%r%u, %u\n", err, op1->type, val, evar);

		eval = env->reg++;
		bprintf(&env->code, "  %%r%u = icmp eq %t %%r%u, %s\n", eval, etype, err, (etype.kind == el_ptr_v) ? "null" : "0");

		onerr = env->lbl++;
		onsuc = env->lbl++;

		bprintf(&env->code, "  br i1 %%r%u, label %%l%u, label %%l%u\n", eval, onsuc, onerr);
		bprintf(&env->code, "l%u:\n", onerr);

		struct handler_t *handler;
		struct frame_t *iter;

		for(iter = frame; iter != NULL; iter = iter->up) {
			for(handler = iter->handler; handler != NULL; handler = handler->next)
				emit_block(handler->block, env, frame, -1, -1);
		}

		pack = env->reg++;
		bprintf(&env->code, "  %%r%u = insertvalue %t undef, %t %%r%u, %d\n", pack, env->func->ret, el_struct_get(env->func->ret.data.vstruct, eret)->type, err, eret);

		bprintf(&env->code, "  ret %t %%r%u\n", env->func->ret, pack);
		bprintf(&env->code, "l%u:\n", onsuc);

		*out = env->reg++;
		if(el_struct_find(vstruct, "val") >= 0)
			bprintf(&env->code, "  %%r%u = extractvalue %t %%r%u, 0\n", *out, op1->type, val);
	} break;

	case el_ereq_v: {
		int32_t evar;
		uint32_t val, err, eval, onerr, onsuc;
		struct el_expr_t *op1 = expr->data.op1;
		struct el_type_t etype;
		struct el_struct_t *vstruct;

		vstruct = op1->type.data.vstruct;
		//vtype = el_struct_get(vstruct, 0)->type;
		evar = el_struct_find(op1->type.data.vstruct, "err");
		etype = el_struct_get(vstruct, evar)->type;

		chkret(emit_expr(op1, env, frame, &val));

		err = env->reg++;
		bprintf(&env->code, "  %%r%u = extractvalue %t %%r%u, %u\n", err, op1->type, val, evar);

		eval = env->reg++;
		bprintf(&env->code, "  %%r%u = icmp eq %t %%r%u, %s\n", eval, etype, err, (etype.kind == el_ptr_v) ? "null" : "0");

		onerr = env->lbl++;
		onsuc = env->lbl++;

		bprintf(&env->code, "  br i1 %%r%u, label %%l%u, label %%l%u\n", eval, onsuc, onerr);
		bprintf(&env->code, "l%u:\n", onerr);

		uint32_t dbg = env->dbg++;
		bprintf(&env->code, "  call void(i8*, ...) @fatal(%t %%r%u), !dbg !%u\n", etype, err, dbg);
		bprintf(&env->ext, "!%u = distinct !DILocation(line: %u, column: %u, scope: !%u)\n", dbg, expr->tag->line, expr->tag->col, env->prog);
		//bprintf(&env->code, "  call void @abort()\n");
		bprintf(&env->code, "  unreachable\n");
		bprintf(&env->code, "l%u:\n", onsuc);

		*out = env->reg++;
		if(el_struct_find(vstruct, "val") >= 0)
			bprintf(&env->code, "  %%r%u = extractvalue %t %%r%u, 0\n", *out, op1->type, val);
	} break;

	case el_enval_v: {
		struct el_enval_t *val = expr->data.enval;

		*out = env->reg++;
		bprintf(&env->code, "%%r%u = add %t %d, 0\n", *out, expr->type, (int)val->val);
	} break;

	case el_tern_v: {
		uint32_t cond, ontrue, onfalse, after, lhs, rhs, left, right;
		struct el_tern_t *tern = expr->data.tern;

		chkret(emit_expr(tern->cond, env, frame, &cond));

		after = env->lbl++;
		ontrue = tern->ontrue ? env->lbl++ : after;
		onfalse = tern->onfalse ? env->lbl++ : after;
		left = env->lbl++;
		right = env->lbl++;

		bprintf(&env->code, "  br i1 %%r%u, label %%l%u, label %%l%u\n", cond, ontrue, onfalse);

		bprintf(&env->code, "l%u:\n", ontrue);
		chkret(emit_expr(tern->ontrue, env, frame, &lhs));
		bprintf(&env->code, "  br label %%l%u\n", left);
		bprintf(&env->code, "l%u:\n", left);
		bprintf(&env->code, "  br label %%l%u\n", after);

		bprintf(&env->code, "l%u:\n", onfalse);
		chkret(emit_expr(tern->onfalse, env, frame, &rhs));
		bprintf(&env->code, "  br label %%l%u\n", right);
		bprintf(&env->code, "l%u:\n", right);
		bprintf(&env->code, "  br label %%l%u\n", after);

		bprintf(&env->code, "l%u:\n", after);
		*out = env->reg++;
		bprintf(&env->code, "  %%r%u = phi %t [ %%r%u, %%l%u ], [ %%r%u, %%l%u ]", *out, expr->type, lhs, left, rhs, right);
	} break;

	default:
		fatal("stub expr %d", expr->kind);
		break;
	}

	return NULL;
}

/**
 * Emit an left-value.
 *   @expr: The expression.
 *   @env: The environment.
 *   @out: Out. The result register.
 *   &returns: Error.
 */
static char *emit_left(struct el_expr_t *expr, struct env_t *env, struct frame_t *frame, uint32_t *out)
{
	onerr( );

	switch(expr->kind) {
	case el_ident_v: {
		struct el_ident_t *ident = expr->data.ident;

		switch(ident->type) {
		case el_unk_v:
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast %t* @\"%s\" to %t*\n", *out, expr->type, expr->data.ident->str, expr->type);
			break;

		case el_local_v:
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast %t* %%r%u to %t*\n", *out, expr->type, ref_get(&env->local, ident->data.decl), expr->type);
			break;

		case el_global_v:
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast %t* @$g%u to %t*\n", *out, expr->type, ref_get(&env->global, ident->data.decl), expr->type);
			break;

		case el_param_v:
			*out = ident->data.off;
			break;

		case el_builtin_v:
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast %t* @$%s to %t*\n", *out, expr->type, expr->data.ident->str + 3, expr->type);
			break;
		}
	} break;

	case el_deref_v:
		if(expr->data.op1->type.kind == el_arr_v) {
			fatal("stub");
		}
		else if(expr->data.op1->type.kind == el_ptr_v) {
			uint32_t tmp;

			chkret(emit_expr(expr->data.op1, env, frame, &tmp));
			
			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to %t*\n", *out, tmp, expr->type);

			return NULL;
		}
		else
			fatal("%s:%u: uh-oh: %d", expr->tag->file->buf, expr->tag->line, expr->type.kind);

	case el_elem_v: {
		struct el_elem_t *elem = expr->data.elem;
		struct el_type_t type = elem->base->type;

		if(elem->deref) {
			assert(type.kind == el_ptr_v);

			uint32_t tmp, base;

			type = type.data.ptr->sub;
			chkret(emit_expr(elem->base, env, frame, &base));

			if(el_type_root(type).kind == el_union_v) {
				fatal("stub");
			}
			else {
				tmp = env->reg++;
				bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to %t*\n", tmp, base, type);

				*out = env->reg++;
				bprintf(&env->code, "  %%r%u = getelementptr %t, %t* %%r%u, i32 0, i32 %u\n", *out, type, type, tmp, elem->idx);
			}
		}
		else {
			uint32_t base;

			chkret(emit_left(elem->base, env, frame, &base));

			//tmp = env->reg++;
			//bprintf(&env->code, "  %%r%u = bitcast <{}>* %%r%u to %t*\n", tmp, base, type);

			if(el_type_root(type).kind == el_union_v) {
				*out = env->reg++;
				bprintf(&env->code, "  %%r%u = bitcast %t* %%r%u to %t* ; this\n", *out, type, base, expr->type);
			}
			else {
				*out = env->reg++;
				bprintf(&env->code, "  %%r%u = getelementptr %t, %t* %%r%u, i32 0, i32 %u\n", *out, type, type, base, elem->idx);
			}
		}
	} break;

	case el_index_v: {
		struct el_index_t *index = expr->data.index;

		if(index->base->type.kind == el_ptr_v) {
			uint32_t base, off, ptr;
			struct el_type_t type = index->base->type.data.ptr->sub;

			chkret(emit_expr(index->base, env, frame, &base));
			chkret(emit_expr(index->off, env, frame, &off));

			ptr = env->reg++;
			bprintf(&env->code, "  %%r%u = bitcast i8* %%r%u to %t*\n", ptr, base, type);

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = getelementptr %t, %t* %%r%u, %t %%r%u\n", *out, type, type, ptr, index->off->type, off);
		}
		else if(index->base->type.kind == el_arr_v) {
			uint32_t base, off;
			struct el_type_t type = index->base->type;
			//struct el_arr_t *arr = index->base->type.data.arr;

			chkret(emit_left(index->base, env, frame, &base));
			chkret(emit_expr(index->off, env, frame, &off));

			*out = env->reg++;
			bprintf(&env->code, "  %%r%u = getelementptr %t, %t* %%r%u, i64 0, %t %%r%u\n", *out, type, type, base, index->off->type, off);
		}
		else {
			fatal("stub");
		}
	} break;

	default: {
		uint32_t var, eval;

		chkret(emit_expr(expr, env, frame, &eval));

		var = env->reg++;
		bprintf(&env->vars, "  %%r%u = alloca %t\n", var, expr->type);
		bprintf(&env->code, "  store %t %%r%u, %t* %%r%u\n", expr->type, eval, expr->type, var);

		*out = var;
	} break;
	}

	return NULL;
}


/**
 * Generate an allocation, returning the pointer in a register.
 *   @env: The environment.
 *   @type: The type.
 *   &returns: The register.
 */
uint32_t gen_alloc(struct env_t *env, struct el_type_t type)
{
	uint32_t id = env->reg++;

	bprintf(&env->vars, "  %%r%u = alloca %t\n", id, type);

	return id;
}


/**
 * Get the opcode name for an expression kind.
 *   @kind: The kind.
 *   @type: The type.
 *   &returns: The operation string.
 */
static const char *str_op2(enum el_expr_e kind, struct el_type_t type)
{
	switch(kind) {
	case el_eq_v: return "icmp eq";
	case el_ne_v: return "icmp ne";
	case el_gt_v: return el_type_is_signed(type) ? "icmp sgt" : "icmp ugt";
	case el_gte_v: return el_type_is_signed(type) ? "icmp sge" : "icmp uge";
	case el_lt_v: return el_type_is_signed(type) ? "icmp slt" : "icmp ult";
	case el_lte_v: return el_type_is_signed(type) ? "icmp sle" : "icmp ule";
	case el_add_v: return "add";
	case el_sub_v: return "sub";
	case el_mul_v: return "mul";
	case el_div_v: return el_type_is_signed(type) ? "sdiv" : "udiv";
	case el_rem_v: return el_type_is_signed(type) ? "srem" : "urem";
	case el_or_v: return "or";
	case el_xor_v: return "xor";
	case el_and_v: return "and";
	case el_addeq_v: return "add";
	case el_subeq_v: return "sub";
	case el_muleq_v: return "mul";
	case el_diveq_v: return el_type_is_signed(type) ? "sdiv" : "udiv";
	case el_remeq_v: return el_type_is_signed(type) ? "srem" : "urem";
	case el_andeq_v: return "and";
	case el_oreq_v: return "or";
	case el_xoreq_v: return "xor";
	case el_postinc_v: return "add";
	case el_preinc_v: return "add";
	case el_postdec_v: return "sub";
	case el_predec_v: return "sub";
	case el_shl_v: return "shl";
	case el_shr_v: return el_type_is_signed(type) ? "ashr" : "lshr";
	default: fatal("Invalid op2 name.");
	}
}


/**
 * Add a reference.
 *   @tree: The reference root.
 *   @decl: The declaration.
 *   @id: The identifier.
 */
static void ref_add(struct avl_root_t *root, struct el_decl_t *decl, uint32_t id)
{
	struct ref_t *ref;

	ref = malloc(sizeof(struct ref_t));
	ref->id = id;
	ref->decl = decl;
	avl_node_init(&ref->node, decl);
	avl_root_add(root, &ref->node);
}

/**
 * Retrieve a reference.
 *   @tree: The reference root.
 *   @decl: The declaration.
 *   &returns: The register ID.
 */
static uint32_t ref_get(struct avl_root_t *root, struct el_decl_t *decl)
{
	struct avl_node_t *node;

	node = avl_root_get(root, decl);
	if(node == NULL)
		fatal("Missing declaration.");

	return getparent(node, struct ref_t, node)->id;
}


/**
 * Create a string.
 *   &returns: The string.
 */
static struct buf_t buf_new(struct env_t *env)
{
	return (struct buf_t){ malloc(32), 0, 32, env };
}

/**
 * Finish a buffer by printing to the file.
 *   @buf: The buffer.
 *   @file: The file.
 */
static void buf_done(struct buf_t buf, FILE *file)
{
	if(buf.len > 0) {
		if(fwrite(buf.arr, buf.len, 1, file) < 1)
			fatal("Failed to write to file.");
	}

	free(buf.arr);
}


/**
 * Add a character.
 *   @buf: The buffer.
 *   @ch: The character.
 */
static void buf_ch(struct buf_t *str, char ch)
{
	if(str->len == str->cap)
		str->arr = realloc(str->arr, str->cap *= 2);

	str->arr[str->len++] = ch;
}

/**
 * Add a string to the buffer.
 *   @buf: The buffer.
 *   @str: The string.
 */
static void buf_str(struct buf_t *buf, const char *str)
{
	while(*str != '\0')
		buf_ch(buf, *str++);
}

/**
 * Write a signed integer to a buffer.
 *   @buf: The buffer.
 *   @val: The value.
 */
static void buf_int(struct buf_t *buf, int64_t val)
{
	char str[32];

	sprintf(str, "%ld", val);

	buf_str(buf, str);
}

/**
 * Write an unsigned integer to a buffer.
 *   @buf: The buffer.
 *   @val: The value.
 */
static void buf_uint(struct buf_t *buf, uint64_t val)
{
	char str[32];

	sprintf(str, "%lu", val);

	buf_str(buf, str);
}

/**
 * Add a type to the buffer.
 *   @buf: The buffer.
 *   @expr: The expression.
 */
static void buf_type(struct buf_t *buf, struct el_type_t type)
{
	switch(type.kind) {
	case el_void_v: buf_str(buf, "void"); break;
	case el_bool_v: buf_str(buf, "i1"); break;
	case el_i8_v: buf_str(buf, "i8"); break;
	case el_u8_v: buf_str(buf, "i8"); break;
	case el_i16_v: buf_str(buf, "i16"); break;
	case el_u16_v: buf_str(buf, "i16"); break;
	case el_i32_v: buf_str(buf, "i32"); break;
	case el_u32_v: buf_str(buf, "i32"); break;
	case el_i64_v: buf_str(buf, "i64"); break;
	case el_u64_v: buf_str(buf, "i64"); break;
	case el_args_v: buf_str(buf, "i8*"); break;
	//case el_args_v: buf_str(buf, "%struct.__va_list_tag"); break;
	case el_alias_v: buf_type(buf, type.data.alias->type); break;
	case el_arr_v: bprintf(buf, "[%u x %t]", type.data.arr->len, type.data.arr->type); break;

		       /*
	case el_ptr_v:
		if(el_type_is_comp(type.data.ptr->sub))
			buf_str(buf, "<{}>*");
		else if(!el_type_is_void(type.data.ptr->sub)) {
			buf_type(buf, type.data.ptr->sub);
			buf_ch(buf, '*');
		}
		else
			buf_str(buf, "i8*");

		break;
		*/
	case el_ptr_v: buf_str(buf, "i8*"); break;

	case el_struct_v: {
		struct el_member_t *member;

		buf_str(buf, "<{");

		for(member = type.data.vstruct->member; member != NULL; member = member->next)
			bprintf(buf, "%t%s", member->type, member->next ? "," : "");

		buf_str(buf, "}>");
	} break;

	case el_num_v: fatal("Cannot emit a bigint.");
	case el_func_v: {
		uint32_t i;
		struct el_func_t *func = type.data.func;

		buf_type(buf, func->ret);
		buf_ch(buf, '(');
		for(i = 0; i < func->cnt; i++)
			bprintf(buf, "%s%t", (i > 0) ? ", " : "", func->param[i].type);
		if(func->var)
			bprintf(buf, "%s...", (i != 0) ? ", " : "");
		buf_ch(buf, ')');
	} break;

	default:
		bprintf(buf, "i%u", el_type_size(type));
		break;
	}
}

/**
 * Add an expression to the buffer.
 *   @buf: The buffer.
 *   @type: The type.
 */
static void buf_init(struct buf_t *buf, struct el_init_t *head, struct el_type_t base)
{
	if(base.kind == el_struct_v) {
		uint32_t i, n;
		struct el_init_t *init;
		struct el_type_t type;
		struct el_struct_t *vstruct = base.data.vstruct;

		bprintf(buf, "<{");

		n = el_struct_cnt(vstruct);
		for(i = 0; i < n; i++) {
			if(i > 0)
				bprintf(buf, ", ");

			type = el_struct_get(vstruct, i)->type;
			init = el_init_get(head, i);
			if(init != NULL) {
				if(init->nest) {
					fatal("stub");
				}
				else
					bprintf(buf, "%t %e", type, init->data.expr);
			}
			else
				bprintf(buf, "%t zeroinitializer", type);
		}

		bprintf(buf, "}>");
	}
	else if(base.kind == el_arr_v) {
		struct el_init_t *init;
		struct el_type_t type = base.data.arr->type;

		bprintf(buf, "[");

		for(init = head; init != NULL; init = init->next) {
			if(init->nest) {
				bprintf(buf, "%t ", type);
				buf_init(buf, init->data.nest, type);
			}
			else
				bprintf(buf, "%t %e", type, init->data.expr);

			if(init->next != NULL)
				bprintf(buf, ", ");
		}

		bprintf(buf, "]");
	}
	else
		fatal("stub");
}

static void buf_expr(struct buf_t *buf, struct el_expr_t *expr)
{
	switch(expr->kind) {
	case el_const_v:
		buf_const(buf, expr->data.vconst);
		break;

	case el_init_v: {
		buf_init(buf, expr->data.init, expr->type);
	} break;

	case el_addr_v: {
		struct el_expr_t *sub = expr->data.op1;

		switch(sub->kind) {
		case el_ident_v: {
			struct el_ident_t *ident = sub->data.ident;
		
			//if(sub->type.kind == el_struct_v)
				//bprintf(buf, "bitcast (%t* @$g%u to <{}>*)", sub->type, ref_get(&buf->env->global, ident->data.decl));
			//else
				//bprintf(buf, "@$g%u", ref_get(&buf->env->global, ident->data.decl));

			bprintf(buf, "bitcast (%t* @$g%u to i8*)", sub->type, ref_get(&buf->env->global, ident->data.decl));
		} break;

		default:
			fatal("stub");
		}
	} break;

	case el_cast_v: {
		struct el_expr_t *cast = expr->data.op1;
		struct el_type_t dst = expr->type, src = cast->type;

		if((src.kind == el_ptr_v) && (dst.kind == el_ptr_v))
			bprintf(buf, "bitcast (%t %e to %t)", src, cast, dst);
		else
			fatal("%s:%u: stub", expr->tag->file->buf, expr->tag->line);
	} break;

	case el_ident_v: {
		struct el_decl_t *decl;
		struct el_ident_t *ident = expr->data.ident;

		if(ident->type != el_global_v)
			fatal("Cannot use non-global in a global initializer.");

		decl = ident->data.decl;
		if(decl->flags & EL_MUT)
			fatal("Cannot use mutable value in a global initializer.");
		else if(decl->expr == NULL)
			fatal("Cannot use uninitalized definition in a global initializer.");

		buf_expr(buf, decl->expr);
	} break;

	case el_enval_v: {
		struct el_enval_t *val = expr->data.enval;

		bprintf(buf, "%d", (int)val->val);
	} break;

	case el_neg_v:
		bprintf(buf, "sub (%t 0, %t %e)", expr->type, expr->type, expr->data.op1);
		break;

	default:
		fatal("stub %d %d", expr->kind, el_init_v);
	}
}

/**
 * Add a value to the buffer.
 *   @buf: The buffer.
 *   @value: The value.
 */
static void buf_const(struct buf_t *buf, struct el_const_t *vconst)
{
	switch(vconst->type) {
	case el_const_bool_v: buf_int(buf, vconst->data.flag); break;
	case el_const_i8_v: buf_int(buf, vconst->data.i8); break;
	case el_const_u8_v: buf_uint(buf, vconst->data.u8); break;
	case el_const_i16_v: buf_int(buf, vconst->data.i16); break;
	case el_const_u16_v: buf_uint(buf, vconst->data.u16); break;
	case el_const_i32_v: buf_int(buf, vconst->data.i32); break;
	case el_const_u32_v: buf_uint(buf, vconst->data.u32); break;
	case el_const_i64_v: buf_int(buf, vconst->data.i64); break;
	case el_const_u64_v: buf_uint(buf, vconst->data.u64); break;
	case el_const_ptr_v: bprintf(buf, "inttoptr (i64 %d to %t)", mpz_get_si(vconst->data.num->mpz), vconst->dty); break;
	case el_const_str_v: {
		uint32_t nam = (*str_hack)++, len = strlen(vconst->data.str) + 1;
		char *str = escape(vconst->data.str);

		bprintf(buf_hack, "  @s%u = hidden local_unnamed_addr constant [%u x i8] c\"%s\\00\", align 1\n", nam, len, str);
		bprintf(buf, "getelementptr inbounds ([%u x i8], [%u x i8]* @s%u, i64 0, i64 0)", len, len, nam);

		free(str);
	} break;

	default: fatal("stub %d %d", vconst->type, vconst->type); break;
	}
}


/**
 * Printf to a buffer.
 *   @buf: The buffer.
 *   @fmt: The format string.
 *   @...: The arguments.
 */
static void bprintf(struct buf_t *buf, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	bprintfv(buf, fmt, args);
	va_end(args);
}

/**
 * Printf to a buffer.
 *   @buf: The buffer.
 *   @fmt: The format string.
 *   @args: The arguments.
 */
static void bprintfv(struct buf_t *buf, const char *fmt, va_list args)
{
	while(*fmt != '\0') {
		if(*fmt == '%') {
			fmt++;
			switch(*fmt) {
			case 'u': {
				char tmp[16];

				snprintf(tmp, sizeof(tmp), "%u", va_arg(args, uint32_t));
				buf_str(buf, tmp);
			} break;

			case 'd': {
				char tmp[16];

				snprintf(tmp, sizeof(tmp), "%d", va_arg(args, int32_t));
				buf_str(buf, tmp);
			} break;

			case 'c':
				buf_ch(buf, va_arg(args, int));
				break;

			case 's':
				buf_str(buf, va_arg(args, const char *));
				break;

			case 't':
				buf_type(buf, va_arg(args, struct el_type_t));
				break;

			case 'e':
				buf_expr(buf, va_arg(args, struct el_expr_t *));
				break;

			case 'v':
				buf_const(buf, va_arg(args, struct el_const_t *));
				break;

			case '%':
				buf_ch(buf, '%');
				break;

			default:
				fatal("Invalid format '%c'.", *fmt);
			}
			fmt++;
		}
		else
			buf_ch(buf, *fmt++);
	}
}

static uint32_t env_label(struct env_t *env, struct el_label_t *ref)
{
	struct label_t **label;

	for(label = &env->label; *label != NULL; label = &(*label)->next) {
		if((*label)->ref == ref)
			return (*label)->num;
	}

	*label = malloc(sizeof(struct label_t));
	(*label)->ref = ref;
	(*label)->num = env->lbl++;
	(*label)->next = NULL;

	return (*label)->num;
}

static char *escape(const char *str)
{
	struct membuf_t buf;

	buf = membuf_init(strlen(str) + 1);

	while(*str != '\0') {
		if(*str == '"')
			membuf_addstr(&buf, "\\22");
		else
			membuf_addch(&buf, *str);

		str++;
	}

	membuf_addch(&buf, '\0');

	return membuf_done(&buf, NULL);
}
