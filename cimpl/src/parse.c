#include "common.h"

/**
 * Context structure.
 *   @path: The file path.
 *   @lex: The lexer.
 *   @func: The current function.
 */
struct parse_t {
	struct el_str_t *path;
	struct lex_parse_t *lex;

	struct el_func_t *func;
};

/**
 * Binary association structure.
 *   @inc: Increment precedence.
 *   @tok: The token.
 *   @func: The expression function.
 */
struct bin_t {
	bool inc;
	int32_t tok;
	struct el_expr_t *(*func)(struct el_expr_t *, struct el_expr_t *, struct el_tag_t *);
};

/**
 * Environment.
 */
struct env_t {
	struct el_case_t **vcase;
};


/*
 * local declarations
 */
static char *parse_top(struct parse_t *parse, struct el_unit_t *unit);
static char *parse_block(struct parse_t *parse, struct env_t *env, struct el_block_t *block);
static char *parse_stmt(struct parse_t *parse, struct env_t *env, struct el_block_t *block);
static char *parse_single(struct parse_t *parse, struct env_t *env, struct el_block_t **block, bool esc);
static char *parse_tydef(struct parse_t *parse, struct env_t *env, struct el_unit_t *unit, struct el_block_t *block);
static char *parse_struct(struct parse_t *parse, struct env_t *env, struct el_member_t **member);
static char *parse_return(struct parse_t *parse, struct env_t *env, struct el_block_t *block);
static char *parse_if(struct parse_t *parse, struct env_t *env, struct el_block_t *block);
static char *parse_while(struct parse_t *parse, struct env_t *env, struct el_block_t *block);
static char *parse_do(struct parse_t *parse, struct env_t *env, struct el_block_t *block);
static char *parse_for(struct parse_t *parse, struct env_t *env, struct el_block_t *block);

static char *parse_ident(struct parse_t *parse, char **ident);
static char *parse_type(struct parse_t *ctx, struct el_type_t *type, bool func);
static char *parse_attr(struct parse_t *ctx, struct el_attr_t **attr);

static char *parse_decl(struct parse_t *parse, struct env_t *env, struct el_unit_t *unit, struct el_block_t *block, uint32_t life);
static char *parse_func(struct parse_t *parse, struct env_t *env, struct el_func_t *func, struct el_expr_t **expr);

static char *parse_expr(struct parse_t *parse, struct el_expr_t **expr);
static char *parse_assign(struct parse_t *parse, struct el_expr_t **expr);
static char *parse_tern(struct parse_t *parse, struct el_expr_t **expr);
static char *parse_bin(struct parse_t *parse, struct el_expr_t **expr, struct bin_t *bin);
static char *parse_call(struct parse_t *parse, struct el_expr_t **expr);
static char *parse_pre(struct parse_t *ctx, struct el_expr_t **expr);
static char *parse_post(struct parse_t *ctx, struct el_expr_t **expr);
static char *parse_term(struct parse_t *parse, struct el_expr_t **expr);

static char *parse_sizeof(struct parse_t *ctx, struct el_tag_t *tag, struct el_expr_t **expr);

static struct el_tag_t *parse_tag(struct parse_t *parse, struct lex_token_t *token);

static const char *parse_prefix(int32_t kind);

static bool peek_type(struct parse_t *ctx);

static char *parse_init(struct parse_t *ctx, struct el_init_t **init);

/*
 * token declarations
 */
#define TOK_TYDEF (0x10000)
#define TOK_DEF   (0x10001)
#define TOK_LET   (0x10002)
#define TOK_VAR   (0x10003)
#define TOK_TY    (0x10100)
#define TOK_ST    (0x10101)
#define TOK_UN    (0x10102)
#define TOK_EN    (0x10103)
#define TOK_PT    (0x10104)
#define TOK_CPT   (0x10105)
#define TOK_AR    (0x10106)
#define TOK_EV    (0x10107)

#define TOK_VOID (0x11000)
#define TOK_I8   (0x11100)
#define TOK_U8   (0x11101)
#define TOK_I16  (0x11102)
#define TOK_U16  (0x11003)
#define TOK_I32  (0x11104)
#define TOK_U32  (0x11105)
#define TOK_I64  (0x11106)
#define TOK_U64  (0x11107)
#define TOK_CHAR (0x11108)
#define TOK_BOOL (0x11109)

#define TOK_RETURN  (0x12000)
#define TOK_IF      (0x12001)
#define TOK_ELSE    (0x12002)
#define TOK_LOOP    (0x12003)
#define TOK_FOR     (0x12004)
#define TOK_WHILE   (0x12005)
#define TOK_DO      (0x12006)
#define TOK_BRK     (0x12007)
#define TOK_CONT    (0x12008)
#define TOK_SWITCH  (0x12009)
#define TOK_CASE    (0x1200A)
#define TOK_DEFAULT (0x1200B)

#define TOK_EQ     (0x20000)
#define TOK_NE     (0x20001)
#define TOK_GTE    (0x20002)
#define TOK_LTE    (0x20003)
#define TOK_OR     (0x21000)
#define TOK_AND    (0x21001)
#define TOK_ADDEQ  (0x22002)
#define TOK_SUBEQ  (0x22003)
#define TOK_MULEQ  (0x22004)
#define TOK_DIVEQ  (0x22005)
#define TOK_REMEQ  (0x22006)
#define TOK_SHLEQ  (0x22007)
#define TOK_SHREQ  (0x22008)
#define TOK_ANDEQ  (0x22009)
#define TOK_XOREQ  (0x2200A)
#define TOK_OREQ   (0x2200B)
#define TOK_SHL    (0x2200C)
#define TOK_SHR    (0x2200D)
#define TOK_DOTS   (0x23000)

#define TOK_ARROW (0x23000)
#define TOK_INC   (0x23001)
#define TOK_DEC   (0x23002)
#define TOK_CALL  (0x23003)

#define TOK_ARGSTYPE  (0x024000)
#define TOK_ARGSINIT  (0x024001)
#define TOK_ARGSGET   (0x024002)

#define TOK_ERET    (0x25000)
#define TOK_EFMT    (0x25001)
#define TOK_EGET    (0x25002)
#define TOK_EREQ    (0x25003)
#define TOK_MEMNEW  (0x25100)
#define TOK_MEMMAKE (0x25101)

#define TOK_SLASHSLASH (0x30000)

#define TOK_NULL   (0x40000)
#define TOK_TRUE   (0x40001)
#define TOK_FALSE  (0x40002)
#define TOK_SIZEOF (0x40003)
#define TOK_ARRLEN (0x40004)
#define TOK_ONERR  (0x40005)
#define TOK_OFFSETOF (0x40006)
#define TOK_GETPARENT (0x40007)
#define TOK_GETREF    (0x40008)

#define TOK_QUOTE1 (0x50000)
#define TOK_QUOTE2 (0x50001)

static struct lex_map_t keywords[] = {
	{ TOK_ARGSTYPE, "ty:args"   },
	{ TOK_ARGSINIT, "args:init" },
	{ TOK_ARGSGET,  "args:get"  },
	{ TOK_ERET,     "e:ret"     },
	{ TOK_EFMT,     "e:fmt"     },
	{ TOK_EGET,     "e:get"     },
	{ TOK_EREQ,     "e:req"     },
	{ TOK_MEMNEW,   "mem:new"   },
	{ TOK_MEMMAKE,  "mem:make"  },
	{ TOK_TYDEF,    "tydef"     },
	{ TOK_DEF,      "def"       },
	{ TOK_LET,      "let"       },
	{ TOK_VAR,      "var"       },
	{ TOK_TY,       "ty"        },
	{ TOK_ST,       "st"        },
	{ TOK_UN,       "un"        },
	{ TOK_EN,       "en"        },
	{ TOK_PT,       "pt"        },
	{ TOK_CPT,      "cpt"       },
	{ TOK_AR,       "ar"        },
	{ TOK_EV,       "ev"        },
	{ TOK_VOID,     "void"      },
	{ TOK_I8,       "i8"        },
	{ TOK_U8,       "u8"        },
	{ TOK_I16,      "i16"       },
	{ TOK_U16,      "u16"       },
	{ TOK_I32,      "i32"       },
	{ TOK_U32,      "u32"       },
	{ TOK_I64,      "i64"       },
	{ TOK_U64,      "u64"       },
	{ TOK_CHAR,     "char"      },
	{ TOK_BOOL,     "bool"      },
	{ TOK_RETURN,   "return"    },
	{ TOK_IF,       "if"        },
	{ TOK_ELSE,     "else"      },
	//{ TOK_LOOP,     "loop"      },
	{ TOK_DO,       "do"        },
	{ TOK_WHILE,    "while"     },
	{ TOK_FOR,      "for"       },
	{ TOK_BRK,      "break"     },
	{ TOK_CONT,     "continue"  },
	{ TOK_SWITCH,   "switch"    },
	{ TOK_CASE,     "case"      },
	{ TOK_DEFAULT,  "default"   },
	{ TOK_NULL,     "null"      },
	{ TOK_TRUE,     "true"      },
	{ TOK_FALSE,    "false"     },
	{ TOK_SIZEOF,   "sizeof"    },
	{ TOK_OFFSETOF, "offsetof"  },
	{ TOK_GETPARENT,"getparent" },
	{ TOK_GETREF,   "getref"    },
	{ TOK_ARRLEN,   "arrlen"    },
	{ TOK_ONERR,    "onerr"     },
};

static struct lex_map_t symbols[] = {
	{ TOK_DOTS, "..." },
	{ TOK_ADDEQ, "+=" },
	{ TOK_SUBEQ, "-=" },
	{ TOK_MULEQ, "*=" },
	{ TOK_DIVEQ, "/=" },
	{ TOK_REMEQ, "%=" },
	{ TOK_SHLEQ, "<<=" },
	{ TOK_SHREQ, ">>=" },
	{ TOK_ANDEQ, "&=" },
	{ TOK_XOREQ, "^=" },
	{ TOK_OREQ,  "|=" },
	{ TOK_EQ,    "==" },
	{ TOK_NE,    "!=" },
	{ TOK_GTE,   ">=" },
	{ TOK_LTE,   "<=" },
	{ TOK_OR,    "||" },
	{ TOK_AND,   "&&" },
	{ TOK_CALL , "->(" },
	{ TOK_ARROW, "->" },
	{ TOK_INC,   "++" },
	{ TOK_DEC,   "--" },
	{ TOK_SHL,   "<<" },
	{ TOK_SHR,   ">>" },
	{ '=',     "="  },
	{ '<',     "<"  },
	{ '>',     ">"  },
	{ '+',     "+"  },
	{ '-',     "-"  },
	{ '*',     "*"  },
	{ '/',     "/"  },
	{ '%',     "%"  },
	{ '|',     "|"  },
	{ '~',     "~"  },
	{ '&',     "&"  },
	{ '^',     "^"  },
	{ '!',     "!"  },
	{ '~',     "~"  },
	{ '.',     "."  },
	{ ',',     ","  },
	{ ';',     ";"  },
	{ ':',     ":"  },
	{ '(',     "("  },
	{ ')',     ")"  },
	{ '{',     "{"  },
	{ '}',     "}"  },
	{ '[',     "["  },
	{ ']',     "]"  },
	{ '?',     "?"  },
	{ ':',     ":"  },
};


struct bin_t bins[] = {
	// logical or
	{ true,  TOK_OR, el_expr_lor  },
	// logical and
	{ true,  TOK_AND, el_expr_land },
	// bitwise or
	{ true,  '|', el_expr_or  },
	// bitwise xor
	{ true,  '^', el_expr_xor },
	// bitwise and
	{ true,  '&', el_expr_and },
	// less/greater than
	{ false, '>', el_expr_gt },
	{ false, '<', el_expr_lt },
	{ false, TOK_GTE, el_expr_gte },
	{ true,  TOK_LTE, el_expr_lte },
	// equal/not equal
	{ false, TOK_EQ, el_expr_eq },
	{ true,  TOK_NE, el_expr_ne },
	// shift
	{ false, TOK_SHL, el_expr_shl },
	{ true,  TOK_SHR, el_expr_shr },
	// add/sub
	{ false, '+', el_expr_add },
	{ true,  '-', el_expr_sub },
	// mul/div/mod
	{ false, '*', el_expr_mul },
	{ false, '/', el_expr_div },
	{ true,  '%', el_expr_rem },
	{ false, -1,  NULL        }
};


/**
 * Parse a path.
 *   @unit: The code unit.
 *   @path: The path.
 *   &returns: Error.
 */
char *el_parse_path(struct el_unit_t *unit, const char *path)
{
	char *err;
	FILE *file;

	onerr( );

	file = fopen(path, "r");
	if(file == NULL)
		fail("Unable to open '%s' for reading. %s.", path, strerror(errno));

	err = el_parse_file(unit, path, file);
	fclose(file);

	return err;
}

/**
 * Parse a file.
 *   @unit: The code unit.
 *   @file: The file.
 *   @path: The path.
 *   &returns: Error.
 */
char *el_parse_file(struct el_unit_t *unit, const char *path, FILE *file)
{
	char *err;
	struct parse_t ctx;

	ctx.path = el_str_new(strdup(path));
	ctx.lex = lex_parse_new(file, strdup(path), 64, true);
	ctx.func = NULL;

	lex_rule_slashslash(ctx.lex, LEX_SKIP);
	lex_rule_slashstar(ctx.lex, LEX_SKIP);
	lex_rule_keyword(ctx.lex, sizeof(keywords) / sizeof(struct lex_map_t), keywords);
	lex_rule_symbol(ctx.lex, sizeof(symbols) / sizeof(struct lex_map_t), symbols);
	lex_rule_ident2(ctx.lex, LEX_ID);
	lex_rule_quote1(ctx.lex, TOK_QUOTE1);
	lex_rule_quote2(ctx.lex, TOK_QUOTE2);
	lex_rule_num(ctx.lex, LEX_NUM);

	err = parse_top(&ctx, unit);
	lex_parse_delete(ctx.lex);
	el_str_delete(ctx.path);

	return err;
}


/**
 * Parse all statements from the top-level.
 *   @parse: The parser.
 *   @unit: The code unit.
 *   &returns: Error.
 */
static char *parse_top(struct parse_t *parse, struct el_unit_t *unit)
{
	onerr( );

	while(true) {
		if(lex_peek(parse->lex)->id == LEX_EOF)
			break;
		else if(lex_try(parse->lex, TOK_TYDEF) != NULL)
			chkret(parse_tydef(parse, NULL, unit, unit->block));
		else if(lex_try(parse->lex, TOK_DEF) != NULL)
			chkret(parse_decl(parse, NULL, unit, unit->block, 0));
		else if(lex_try(parse->lex, TOK_LET) != NULL)
			chkret(parse_decl(parse, NULL, unit, unit->block, EL_AUTO));
		else if(lex_try(parse->lex, TOK_VAR) != NULL)
			chkret(parse_decl(parse, NULL, unit, unit->block, EL_AUTO | EL_MUT));
		else
			failmsg(lex_error(parse->lex, "Expected declaration (def, let, var, or tydef)."));
	}

	return NULL;
}

/**
 * Parse all statements in a block.
 *   @parse: The parser.
 *   @block: The block.
 *   &returns: Error.
 */
static char *parse_block(struct parse_t *parse, struct env_t *env, struct el_block_t *block)
{
	onerr( );

	while(true) {
		if(lex_peek(parse->lex)->id == LEX_EOF)
			break;
		else if(lex_peek(parse->lex)->id == '}')
			break;

		chkret(parse_stmt(parse, env, block));
	}

	return NULL;
}

/**
 * Parse a statement.
 *   @parse: The parser.
 *   @env: The environment.
 *   @block: The block.
 *   &returns: Error.
 */
static char *parse_stmt(struct parse_t *parse, struct env_t *env, struct el_block_t *block)
{
	onerr( );

	if(lex_try(parse->lex, TOK_TYDEF) != NULL)
		return parse_tydef(parse, env, NULL, block);
	else if(lex_try(parse->lex, TOK_DEF) != NULL)
		return parse_decl(parse, env, NULL, block, 0x00);
	else if(lex_try(parse->lex, TOK_LET) != NULL)
		return parse_decl(parse, env, NULL, block, EL_AUTO);
	else if(lex_try(parse->lex, TOK_VAR) != NULL)
		return parse_decl(parse, env, NULL, block, EL_AUTO | EL_MUT);
	else if(lex_try(parse->lex, TOK_RETURN) != NULL)
		return parse_return(parse, env, block);
	else if(lex_try(parse->lex, TOK_IF) != NULL)
		return parse_if(parse, env, block);
	else if(lex_try(parse->lex, TOK_FOR) != NULL)
		return parse_for(parse, env, block);
	else if(lex_try(parse->lex, TOK_WHILE) != NULL)
		return parse_while(parse, env, block);
	else if(lex_try(parse->lex, TOK_DO) != NULL)
		return parse_do(parse, env, block);
	/*
	else if(lex_try(parse->lex, TOK_LOOP) != NULL) {
		struct el_block_t *inner;
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		chkret(parse_single(parse, env, &inner, false));

		el_block_add(block, el_stmt_loop(el_loop_new(false, NULL, NULL, NULL, inner, tag)));
	}
	*/
	else if(lex_try(parse->lex, TOK_ONERR) != NULL) {
		struct el_block_t *inner;
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		chkret(parse_single(parse, env, &inner, false));

		el_block_add(block, el_stmt_onerr(inner, tag));
	}
	else if(lex_try(parse->lex, TOK_BRK) != NULL) {
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		el_block_add(block, el_stmt_brk(tag));

		if(!lex_try(parse->lex, ';'))
			return lex_error(parse->lex, "Expected ';'.");
	}
	else if(lex_try(parse->lex, TOK_CONT) != NULL) {
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		el_block_add(block, el_stmt_cont(tag));

		if(!lex_try(parse->lex, ';'))
			return lex_error(parse->lex, "Expected ';'.");
	}
	else if(lex_try(parse->lex, TOK_SWITCH) != NULL) {
		struct el_expr_t *expr;
		struct env_t nest;
		struct el_switch_t *sw;
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		if(!lex_try(parse->lex, '('))
			return lex_error(parse->lex, "Expected '('.");

		chkret(parse_expr(parse, &expr));

		if(!lex_try(parse->lex, ')'))
			return lex_error(parse->lex, "Expected ')'.");

		sw = el_switch_new(expr, tag);
		nest.vcase = &sw->vcase;

		chkret(parse_single(parse, &nest, &sw->block, false));

		el_block_add(block, el_stmt_switch(sw, el_tag_copy(tag)));
	}
	else if(lex_try(parse->lex, TOK_CASE) != NULL) {
		struct el_expr_t *expr;
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		chkret(parse_expr(parse, &expr));

		if(!lex_try(parse->lex, ':'))
			return lex_error(parse->lex, "Expected ':'.");

		struct el_label_t *label = el_label_new(NULL, tag);
		el_block_add(block, el_stmt_label(label, el_tag_copy(tag)));

		if(env == NULL)
			return lex_error(parse->lex, "Require switch around a case.");

		*env->vcase = el_case_new(expr, label, el_tag_copy(tag));
		env->vcase = &(*env->vcase)->next;
	}
	else if(lex_try(parse->lex, TOK_DEFAULT) != NULL) {
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		if(!lex_try(parse->lex, ':'))
			return lex_error(parse->lex, "Expected ':'124kjl.");

		struct el_label_t *label = el_label_new(NULL, tag);
		el_block_add(block, el_stmt_label(label, el_tag_copy(tag)));

		if(env == NULL)
			return lex_error(parse->lex, "Require switch around a case.");

		*env->vcase = el_case_new(NULL, label, el_tag_copy(tag));
		env->vcase = &(*env->vcase)->next;
	}
	else if(lex_try(parse->lex, TOK_ERET) != NULL) {
		struct el_expr_t *val;
		struct el_type_t type;
		struct el_init_t *init;
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		if(!lex_try(parse->lex, '('))
			return lex_error(parse->lex, "Expected '('.");

		chkret(parse_expr(parse, &val));

		if(!lex_try(parse->lex, ')'))
			return lex_error(parse->lex, "Expected ')'.");

		if(!lex_try(parse->lex, ';'))
			return lex_error(parse->lex, "Expected ';'.");

		type = el_type_copy(parse->func->ret);
		init = el_init_expr(val, strdup("err"));
		el_block_add(block, el_stmt_eret(el_expr_init(type, init, tag), el_tag_copy(tag)));
	}
	else if(lex_try(parse->lex, TOK_EFMT) != NULL) {
		struct el_expr_t *expr;
		struct el_type_t type;
		struct el_init_t *init;
		struct el_call_t *call = NULL;
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		if(!lex_try(parse->lex, '('))
			return lex_error(parse->lex, "Expected '('.");

		call = el_call_new(el_expr_ident(el_ident_unk(strdup("str:fmt")), tag), false);

		if(lex_try(parse->lex, ')') == NULL) {
			struct el_expr_t *tmp;
			struct el_arg_t **arg = &call->arg;

			while(true) {
				chkret(parse_expr(parse, &tmp));

				*arg = el_arg_new(tmp);
				arg = &(*arg)->next;
				call->cnt++;

				if(lex_try(parse->lex, ')') != NULL)
					break;
				else if(lex_try(parse->lex, ',') == NULL)
					failmsg(lex_error(parse->lex, "Expected ',' or ')'."));
			}
		}

		if(lex_try(parse->lex, ';') == NULL)
			return lex_error(parse->lex, "Expected ';'.");

		expr = el_expr_call(call, el_tag_copy(tag));
		type = el_type_copy(parse->func->ret);
		init = el_init_expr(expr, strdup("err"));
		el_block_add(block, el_stmt_eret(el_expr_init(type, init, el_tag_copy(tag)), el_tag_copy(tag)));
	}
	else if(lex_try(parse->lex, '{') != NULL) {
		struct el_block_t *inner;
		struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

		inner = el_block_new(false);
		chkret(parse_block(parse, env, inner));

		if(lex_try(parse->lex, '}') == NULL)
			return lex_error(parse->lex, "Expected '}' or statement.");

		el_block_add(block, el_stmt_block(inner, tag));
	}
	else {
		struct el_expr_t *expr;

		chkret(parse_expr(parse, &expr));
		el_block_add(block, el_stmt_expr(expr, el_tag_copy(expr->tag)));

		if(!lex_try(parse->lex, ';'))
			return lex_error(parse->lex, "Expected ';'.");
	}

	return NULL;
}

/**
 * Parse a single statement, always creating a block.
 *   @esc: The escapable flag.
 *   @parse: The parser.
 *   @block: Ref. The output block.
 */
static char *parse_single(struct parse_t *parse, struct env_t *env, struct el_block_t **block, bool esc)
{
	onerr( );

	*block = el_block_new(esc);

	if(lex_try(parse->lex, ';'))
		return NULL;

	if(lex_try(parse->lex, '{')) {
		chkret(parse_block(parse, env, *block));

		if(!lex_try(parse->lex, '}'))
			return lex_error(parse->lex, "Expected '}'.");
	}
	else
		chkret(parse_stmt(parse, env, *block));

	return NULL;
}

/**
 * Parse a type definition.
 *   @parse: The parser.
 *   @unit: The code unit.
 *   @block: The block.
 *   &returns: Error.
 */
static char *parse_tydef(struct parse_t *parse, struct env_t *env, struct el_unit_t *unit, struct el_block_t *block)
{
	int32_t kind;
	char *id = NULL, *tmp;
	struct el_attr_t *attr = NULL;
	struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

	onerr( if(id != NULL) free(id); el_tag_delete(tag); el_attr_clear(attr); );

	chkret(parse_attr(parse, &attr));

	if(lex_try(parse->lex, TOK_ST) != NULL)
		kind = TOK_ST;
	else if(lex_try(parse->lex, TOK_UN) != NULL)
		kind = TOK_UN;
	else if(lex_try(parse->lex, TOK_EN) != NULL)
		kind = TOK_EN;
	else if(lex_try(parse->lex, TOK_TY) != NULL)
		kind = TOK_TY;
	else if(lex_try(parse->lex, TOK_AR) != NULL)
		kind = TOK_AR;
	else
		failmsg(lex_error(parse->lex, "Expected type prefix."));

	if(lex_try(parse->lex, ':') == NULL)
		failmsg(lex_error(parse->lex, "Expected ':'."));

	chkret(parse_ident(parse, &tmp));
	id = mprintf("%s:%s", parse_prefix(kind), tmp);
	free(tmp);

	if(lex_try(parse->lex, '=') != NULL) {
		struct el_type_t type;

		chkret(parse_type(parse, &type, true));

		if(kind == TOK_TY) {
			struct el_tydecl_t *def = el_tydecl_new(id, (unit != NULL) ? EL_GLOB : 0, el_type_alias(el_alias_new(id, type)));
			def->attr = el_attr_clone(attr);
			el_attr_tydecl(def);

			if(unit != NULL)
				el_block_add(unit->block, el_stmt_tydecl(def, tag));
			else
				el_block_add(block, el_stmt_tydecl(def, tag));
		}
		else
			fatal("stub");
	}
	else if(lex_try(parse->lex, '{') != NULL) {
		if(kind == TOK_EN) {
			int64_t val = 0;
			struct el_repr_t *repr, **iter = &repr;

			if(!lex_try(parse->lex, '}')) {
				while(true) {
					char *id;
					struct lex_token_t *token;

					token = lex_try(parse->lex, TOK_QUOTE2);
					if(token != NULL)
						id = strdup(token->str);
					else
						chkret(parse_ident(parse, &id));

					if(lex_try(parse->lex, '=')) {
						char *endptr;

						if((token = lex_try(parse->lex, LEX_NUM)) == NULL)
							fail("Expected number.");

						errno = 0;
						val = strtol(token->str, &endptr, 0);
						if((errno != 0) || (*endptr != '\0'))
							fail("Invalid number '%s'.", token->str);
					}

					*iter = el_repr_new(id, val++);
					iter = &(*iter)->next;

					if(lex_try(parse->lex, ','))
						continue;
					else if(lex_try(parse->lex, '}'))
						break;

					failmsg(lex_error(parse->lex, "Expected ',' or '='."));
				}
			}

			struct el_tydecl_t *def = el_tydecl_new(id, (unit != NULL) ? EL_GLOB : 0, el_type_enum(el_enum_new(id, repr)));
			def->attr = el_attr_clone(attr);
			el_attr_tydecl(def);
			if(unit != NULL)
				el_block_add(unit->block, el_stmt_tydecl(def, tag));
			else
				el_block_add(block, el_stmt_tydecl(def, tag));
		}
		else {
			struct el_member_t *member;

			if((kind == TOK_TY) || (kind == TOK_EN) || (kind == TOK_AR))
				failmsg(lex_error(parse->lex, "Compound types only apply to structures or unions."));

			chkret(parse_struct(parse, env, &member));

			struct el_tydecl_t *def = el_tydecl_new(id, (unit != NULL) ? EL_GLOB : 0, el_type_struct(el_struct_new(id, member)));
			def->attr = el_attr_clone(attr);
			el_attr_tydecl(def);
			if(unit != NULL)
				el_block_add(unit->block, el_stmt_tydecl(def, tag));
			else
				el_block_add(block, el_stmt_tydecl(def, tag));
		}
	}
	else
		failmsg(lex_error(parse->lex, "Expected '=' or '{'."));

	if(lex_try(parse->lex, ';') == NULL)
		failmsg(lex_error(parse->lex, "Expected ';'."));

	el_attr_clear(attr);

	return NULL;
}

/**
 * Parse a structure.
 *   @parse: The parser.
 *   @member: Out. The member.
 *   &return: Error.
 */
static char *parse_struct(struct parse_t *parse, struct env_t *env, struct el_member_t **member)
{
	char *id;
	struct el_type_t type = el_type_void();
	struct el_member_t **head = member;

	onerr( el_member_clear(*head); el_type_delete(type); );

	*head = NULL;
	while(lex_try(parse->lex, '}') == NULL) {
		chkret(parse_type(parse, &type, true));

		while(true) {
			id = NULL;
			chkret(parse_ident(parse, &id));

			*member = el_member_new(id, el_type_copy(type));
			member = &(*member)->next;

			if(lex_try(parse->lex, ';') != NULL)
				break;
			else if(lex_try(parse->lex, ',') == NULL)
				failmsg(lex_error(parse->lex, "Expected ',' or ';'."));
		}

		el_type_delete(type);
	}

	return NULL;
}

/**
 * Parse a return statement.
 *   @block: The block.
 *   &returns: Error.
 */
static char *parse_return(struct parse_t *parse, struct env_t *env, struct el_block_t *block)
{
	struct el_expr_t *expr;
	struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

	onerr( el_tag_delete(tag); );

	if(!lex_try(parse->lex, ';')) {
		chkret(parse_expr(parse, &expr));
		if(!lex_try(parse->lex, ';'))
			return lex_error(parse->lex, "Expected ';'.");
	}
	else
		expr = NULL;

	el_block_add(block, el_stmt_return(expr, tag));

	return NULL;
}

/**
 * Parse an if statement.
 *   @block: The block.
 *   &returns: Error.
 */
static char *parse_if(struct parse_t *parse, struct env_t *env, struct el_block_t *block)
{
	struct el_expr_t *eval = NULL;
	struct el_block_t *ontrue = NULL, *onfalse = NULL;
	struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

	onerr(
		el_tag_delete(tag);
		if(eval != NULL) el_expr_delete(eval);
		if(ontrue != NULL) el_block_delete(ontrue);
	);

	if(!lex_try(parse->lex, '('))
		return lex_error(parse->lex, "Expected '('.");

	chkret(parse_expr(parse, &eval));

	if(!lex_try(parse->lex, ')'))
		return lex_error(parse->lex, "Expected ')'.");

	chkret(parse_single(parse, env, &ontrue, false));

	if(lex_try(parse->lex, TOK_ELSE))
		chkret(parse_single(parse, env, &onfalse, false));

	el_block_add(block, el_stmt_cond(el_cond_new(eval, ontrue, onfalse, tag)));
	return NULL;
}

/**
 * Parse a while statement.
 *   @block: The block.
 *   &returns: Error.
 */
static char *parse_while(struct parse_t *parse, struct env_t *env, struct el_block_t *block)
{
	struct el_block_t *body;
	struct el_expr_t *cond = NULL;
	struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

	onerr(
		el_tag_delete(tag);
		if(cond != NULL) el_expr_delete(cond);
	);

	if(!lex_try(parse->lex, '('))
		return lex_error(parse->lex, "Expected '('.");

	chkret(parse_expr(parse, &cond));

	if(!lex_try(parse->lex, ')'))
		failmsg(lex_error(parse->lex, "Expected ')'."));

	chkret(parse_single(parse, env, &body, true));
	el_block_add(block, el_stmt_loop(el_loop_new(false, NULL, cond, NULL, body, tag)));

	return NULL;
}

static char *parse_do(struct parse_t *parse, struct env_t *env, struct el_block_t *block)
{
	struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));
	struct el_expr_t *cond = NULL;
	struct el_block_t *body = NULL;

	onerr(
		el_tag_delete(tag);
		if(body != NULL) el_block_delete(body);
	);

	chkret(parse_single(parse, env, &body, true));

	if(!lex_try(parse->lex, TOK_WHILE))
		failmsg(lex_error(parse->lex, "Expected 'while'."));

	if(!lex_try(parse->lex, '('))
		failmsg(lex_error(parse->lex, "Expected '('."));

	chkret(parse_expr(parse, &cond));

	if(!lex_try(parse->lex, ')'))
		failmsg(lex_error(parse->lex, "Expected ')'."));

	el_block_add(block, el_stmt_loop(el_loop_new(true, NULL, cond, NULL, body, tag)));

	if(!lex_try(parse->lex, ';'))
		failmsg(lex_error(parse->lex, "Expected ';'."));

	return NULL;
}

/**
 * Parse a for statement.
 *   @block: The block.
 *   &returns: Error.
 */
static char *parse_for(struct parse_t *parse, struct env_t *env, struct el_block_t *block)
{
	struct el_block_t *body;
	struct el_expr_t *init = NULL, *cond = NULL, *inc = NULL;
	struct el_tag_t *tag = parse_tag(parse, lex_last(parse->lex));

	onerr(
		el_tag_delete(tag);
		if(init != NULL) el_expr_delete(init);
		if(cond != NULL) el_expr_delete(cond);
		if(inc != NULL) el_expr_delete(inc);
	);

	if(!lex_try(parse->lex, '('))
		return lex_error(parse->lex, "Expected '('.");

	if(!lex_try(parse->lex, ')')) {
		if(!lex_try(parse->lex, ';')) {
			chkret(parse_expr(parse, &init));
			if(!lex_try(parse->lex, ';'))
				failmsg(lex_error(parse->lex, "Expected ';'."));
		}

		if(!lex_try(parse->lex, ';')) {
			chkret(parse_expr(parse, &cond));
			if(!lex_try(parse->lex, ';'))
				failmsg(lex_error(parse->lex, "Expected ';'."));
		}

		if(!lex_try(parse->lex, ')')) {
			chkret(parse_expr(parse, &inc));
			if(!lex_try(parse->lex, ')'))
				failmsg(lex_error(parse->lex, "Expected ')'."));
		}
	}

	chkret(parse_single(parse, env, &body, true));
	el_block_add(block, el_stmt_loop(el_loop_new(false, init, cond, inc, body, tag)));

	return NULL;
}


/**
 * Parse a full identifier, including prefixes.
 *   @parse: The parser.
 *   @ident: Out. The identifier.
 */
static char *parse_ident(struct parse_t *parse, char **ident)
{
	//uint32_t len;
	struct lex_token_t *token;

	onerr( );

	if((token = lex_try(parse->lex, LEX_ID)) == NULL)
		failmsg(lex_error(parse->lex, "Expected identifier."));

	*ident = strdup(token->str);

	/*
	while(lex_try(parse->lex, ':')) {
		if((token = lex_try(parse->lex, LEX_ID)) == NULL)
			failmsg(lex_error(parse->lex, "Expected identifier."));

		len = strlen(*ident);
		*ident = realloc(*ident, len + strlen(token->str) + 2);
		(*ident)[len++] = ':';
		strcpy(*ident + len, token->str);
	}
	*/

	return NULL;
}

static bool try_id(struct lex_parse_t *lex, const char *id)
{
	struct lex_token_t *token = lex_peek(lex);

	if(token->id != LEX_ID)
		return false;

	if(strcmp(token->str, id) != 0)
		return false;

	lex_adv(lex);

	return true;
}

/**
 * Try to parse a type.
 *   @ctx: The context.
 *   @type: Out. The parsed type.
 *   @func: Whether to parse functions.
 *   &returns: Error.
 */
static char *parse_type(struct parse_t *ctx, struct el_type_t *type, bool ftype)
{
	struct lex_token_t *token;
	struct el_func_t *func = NULL;

	onerr( if(func != NULL) el_func_delete(func); );

	if((token = lex_try(ctx->lex, TOK_PT)) != NULL) {
		struct el_type_t sub;

		if(!lex_try(ctx->lex, ':'))
			failmsg(lex_error(ctx->lex, "Expected ':'."));

		if(lex_try(ctx->lex, '(') != NULL) {
			chkret(parse_type(ctx, &sub, true));
			if(lex_try(ctx->lex, ')') == NULL)
				failmsg(lex_error(ctx->lex, "Expected ')'."));
		}
		else
			chkret(parse_type(ctx, &sub, false));

		*type = el_type_ptr(el_ptr_new(0, sub));
	}
	else if((token = lex_try(ctx->lex, TOK_CPT)) != NULL) {
		struct el_type_t sub;

		if(!lex_try(ctx->lex, ':'))
			failmsg(lex_error(ctx->lex, "Expected ':'."));

		if(lex_try(ctx->lex, '(') != NULL) {
			chkret(parse_type(ctx, &sub, true));
			if(lex_try(ctx->lex, ')') == NULL)
				failmsg(lex_error(ctx->lex, "Expected ')'."));
		}
		else
			chkret(parse_type(ctx, &sub, false));

		*type = el_type_ptr(el_ptr_new(EL_CONST, sub));
	}
	else if((token = lex_try(ctx->lex, TOK_TY)) != NULL) {
		char *id, *name;

		if(!lex_try(ctx->lex, ':'))
			failmsg(lex_error(ctx->lex, "Expected ':'."));

		chkret(parse_ident(ctx, &id));
		name = mprintf("ty:%s", id);
		free(id);

		*type = el_type_alias(el_alias_new(name, el_type_void()));
	}
	else if((token = lex_try(ctx->lex, TOK_AR)) != NULL) {
		char *endptr;
		unsigned long len;
		struct el_type_t sub;
		struct lex_token_t *token;

		if(lex_try(ctx->lex, '[') == NULL)
			failmsg(lex_error(ctx->lex, "Expected '['."));

		if((token = lex_try(ctx->lex, LEX_NUM)) != NULL) {
			errno = 0;
			len = strtoul(token->str, &endptr, 0);
			if((errno != 0) || (*endptr != '\0'))
				fail("Invalid number '%s'.", token->str);
		}
		else
			len = 0;

		if(lex_try(ctx->lex, ']') == NULL)
			failmsg(lex_error(ctx->lex, "Expected number or ']'."));

		if(lex_try(ctx->lex, ':') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ':'."));

		chkret(parse_type(ctx, &sub, false));

		*type = el_type_arr(el_arr_new(sub, len));
	}
	else if((token = lex_try(ctx->lex, TOK_ST)) != NULL) {
		char *id, *name;

		if(!lex_try(ctx->lex, ':'))
			failmsg(lex_error(ctx->lex, "Expected ':'."));

		chkret(parse_ident(ctx, &id));
		name = mprintf("st:%s", id);
		free(id);

		*type = el_type_struct(el_struct_new(name, NULL));
	}
	else if((token = lex_try(ctx->lex, TOK_UN)) != NULL) {
		char *id, *name;

		if(!lex_try(ctx->lex, ':'))
			failmsg(lex_error(ctx->lex, "Expected ':'."));

		chkret(parse_ident(ctx, &id));
		name = mprintf("un:%s", id);
		free(id);

		*type = el_type_union(el_struct_new(name, NULL));
	}
	else if((token = lex_try(ctx->lex, TOK_EN)) != NULL) {
		char *id, *name;

		if(!lex_try(ctx->lex, ':'))
			failmsg(lex_error(ctx->lex, "Expected ':'."));

		chkret(parse_ident(ctx, &id));
		name = mprintf("un:%s", id);
		free(id);

		*type = el_type_enum(el_enum_new(name, NULL));
	}
	else if((token = lex_try(ctx->lex, TOK_VOID)) != NULL)
		*type = el_type_void();
	else if((token = lex_try(ctx->lex, TOK_I8)) != NULL)
		*type = el_type_i8();
	else if((token = lex_try(ctx->lex, TOK_U8)) != NULL)
		*type = el_type_u8();
	else if((token = lex_try(ctx->lex, TOK_I16)) != NULL)
		*type = el_type_i16();
	else if((token = lex_try(ctx->lex, TOK_U16)) != NULL)
		*type = el_type_u16();
	else if((token = lex_try(ctx->lex, TOK_I32)) != NULL)
		*type = el_type_i32();
	else if((token = lex_try(ctx->lex, TOK_U32)) != NULL)
		*type = el_type_u32();
	else if((token = lex_try(ctx->lex, TOK_I64)) != NULL)
		*type = el_type_i64();
	else if((token = lex_try(ctx->lex, TOK_U64)) != NULL)
		*type = el_type_u64();
	else if((token = lex_try(ctx->lex, TOK_CHAR)) != NULL)
		*type = el_type_u8();
	else if((token = lex_try(ctx->lex, TOK_BOOL)) != NULL)
		*type = el_type_bool();
	else if((token = lex_try(ctx->lex, TOK_ARGSTYPE)) != NULL)
		*type = el_type_args();
	else if(try_id(ctx->lex, "e:i32")) {
		struct el_struct_t *vstruct;

		if(!lex_try(ctx->lex, '['))
			return lex_error(ctx->lex, "Expected '['.");

		chkret(parse_type(ctx, type, false));
		if(!lex_try(ctx->lex, ']'))
			return lex_error(ctx->lex, "Expected ']'.");

		vstruct = el_struct_new(strdup("!e"), NULL);
		if(!el_type_is_void(*type)) {
			vstruct->member = el_member_new(strdup("val"), *type);
			vstruct->member->next = el_member_new(strdup("err"), el_type_i32());
		}
		else
			vstruct->member = el_member_new(strdup("err"), el_type_i32());

		*type = el_type_struct(vstruct);
	}
	else if(try_id(ctx->lex, "e:str")) {
		struct el_struct_t *vstruct;

		if(!lex_try(ctx->lex, '['))
			return lex_error(ctx->lex, "Expected '['.");

		chkret(parse_type(ctx, type, false));
		if(!lex_try(ctx->lex, ']'))
			return lex_error(ctx->lex, "Expected ']'.");

		vstruct = el_struct_new(strdup("!e"), NULL);
		if(!el_type_is_void(*type)) {
			vstruct->member = el_member_new(strdup("val"), *type);
			vstruct->member->next = el_member_new(strdup("err"), el_type_ptr(el_ptr_new(0, el_type_u8())));
		}
		else
			vstruct->member = el_member_new(strdup("err"), el_type_ptr(el_ptr_new(0, el_type_u8())));

		*type = el_type_struct(vstruct);
	}
	else
		return lex_error(ctx->lex, "Expected type.");

	if(!ftype)
		return NULL;

	if(!lex_try(ctx->lex, '('))
		return NULL;

	func = el_func_new(*type);

	if(!lex_try(ctx->lex, ')')) {
		while(true) {
			if(lex_try(ctx->lex, TOK_DOTS) != NULL) {
				func->var = true;

				if(lex_try(ctx->lex, ')') == NULL)
					failmsg(lex_error(ctx->lex, "Expected ')'."));

				break;
			}
			else {
				char *id;
				struct el_type_t type;

				chkret(parse_type(ctx, &type, true));

				if((token = lex_try(ctx->lex, LEX_ID)) != NULL)
					id = strdup(token->str);
				else
					id = NULL;

				el_func_append(func, type, id);

				if(lex_try(ctx->lex, ')') != NULL)
					break;
				else if(lex_try(ctx->lex, ',') == NULL)
					failmsg(lex_error(ctx->lex, "Expected ',' or ')'."));
			}
		}
	}

	*type = el_type_func(func);
	return NULL;
}

/**
 * Parse an attribute list.
 *   @ctx: The context.
 *   @attr: Out. The attribute list.
 *   &returns: Error.
 */
static char *parse_attr(struct parse_t *ctx, struct el_attr_t **attr)
{
	struct lex_token_t *token;

	onerr( );

	*attr = NULL;

	if(lex_try(ctx->lex, '[') == NULL)
		return NULL;

	while(true) {
		if(lex_try(ctx->lex, ']') != NULL)
			break;
		else if((token = lex_try(ctx->lex, LEX_ID)) != NULL)
			*attr = el_attr_new(strdup(token->str), NULL);
		//else if((token = lex_try(ctx->lex, LEX_STR)) != NULL)
			//fatal("stub");
		else
			failmsg(lex_error(ctx->lex, "Expected attribute or ']'."));

		if(lex_try(ctx->lex, ',') == NULL) {
			if(lex_try(ctx->lex, ']') != NULL)
				break;

			failmsg(lex_error(ctx->lex, "Expected ',' or ']'."));
		}

		attr = &(*attr)->next;
	}

	return NULL;
}


/**
 * Try to parse a declaration.
 *   @ctx: The context.
 *   @block: The block.
 *   @unit: The code unit.
 *   @flags: The declaration flags.
 *   &returns: Error.
 */
static char *parse_decl(struct parse_t *parse, struct env_t *env, struct el_unit_t *unit, struct el_block_t *block, uint32_t life)
{
	char *id  = NULL;
	struct el_tag_t *tag;
	struct el_decl_t *decl;
	struct el_expr_t *expr = NULL;
	struct el_attr_t *attr = NULL;
	struct el_type_t type = el_type_void();

	onerr( el_tag_delete(tag); el_attr_clear(attr); el_type_delete(type); if(id != NULL) free(id); );

	tag = parse_tag(parse, lex_last(parse->lex));

	chkret(parse_attr(parse, &attr));
	chkret(parse_type(parse, &type, true));

	if(unit != NULL)
		life |= EL_GLOB;

	while(true) {
		chkret(parse_ident(parse, &id));

		if(lex_try(parse->lex, '=') != NULL) {
			if(type.kind == el_func_v)
				chkret(parse_func(parse, env, type.data.func, &expr));
			else if(((type.kind == el_struct_v) || (type.kind == el_arr_v)) && (lex_peek(parse->lex)->id == '{')) {
				struct el_init_t *init;
				struct el_tag_t *tag2 = parse_tag(parse, lex_peek(parse->lex));

				chkret(parse_init(parse, &init));

				expr = el_expr_init(el_type_copy(type), init, tag2);

				if(type.kind == el_arr_v) {
					if(type.data.arr->len == 0)
						type.data.arr->len = el_init_len(init);
				}
			}
			else
				chkret(parse_expr(parse, &expr));
		}
		else
			expr = NULL;

		decl = el_decl_new(id, life, el_type_copy(type), expr, el_tag_copy(tag));
		decl->attr = el_attr_clone(attr);
		el_attr_decl(decl);
		el_block_add(block, el_stmt_decl(decl, el_tag_copy(tag)));

		if(lex_try(parse->lex, ';') != NULL)
			break;
		else if(lex_try(parse->lex, ',') == NULL)
			return lex_error(parse->lex, "Expected ',' or ';'.");
	}

	el_tag_delete(tag);
	el_attr_clear(attr);
	el_type_delete(type);

	return NULL;
}

/**
 * Parse a function body.
 *   @ctx: The context.
 *   @func: The function type.
 *   @expr: Out. The expression.
 *   &returns: Error.
 */
static char *parse_func(struct parse_t *parse, struct env_t *env, struct el_func_t *func, struct el_expr_t **expr)
{
	struct parse_t next;
	struct el_tag_t *tag;
	struct el_block_t *block;

	onerr( el_block_delete(block); );

	block = el_block_new(false);

	if(!lex_try(parse->lex, '{'))
		failmsg(lex_error(parse->lex, "Expected '{'."));

	tag = parse_tag(parse, lex_last(parse->lex));

	next.path = parse->path;
	next.lex = parse->lex;
	next.func = func;

	chkret(parse_block(&next, NULL, block));
	if(!lex_try(parse->lex, '}'))
		failmsg(lex_error(parse->lex, "Unexpected end-of-file. Expected '}'."));

	*expr = el_expr_body(el_body_new(el_func_copy(func), block), tag);
	//el_type_replace(&(*expr)->type, el_type_func(el_func_copy(func)));

	return NULL;
}


/**
 * Parse an expression.
 *   @parse: The parser.
 *   @expr: Out. The expression.
 *   &returns: Error.
 */
static char *parse_expr(struct parse_t *parse, struct el_expr_t **expr)
{
	return parse_assign(parse, expr);
}

/**
 * Parse assignments.
 *   @parse: The parser.
 *   @expr: Out. The expression.
 *   &returns: Error.
 */
static char *parse_assign(struct parse_t *parse, struct el_expr_t **expr)
{
	struct el_expr_t *rhs;
	struct lex_token_t *token;
	struct el_tag_t *tag = NULL;
	struct el_expr_t *(*func)(struct el_expr_t *, struct el_expr_t *, struct el_tag_t *);

	onerr( if(*expr != NULL) el_expr_delete(*expr); if(tag != NULL) el_tag_delete(tag); );

	*expr = NULL;
	//chkret(parse_bin(parse, expr, bins));
	chkret(parse_tern(parse, expr));

	while(true) {
		if((token = lex_try(parse->lex, '=')) != NULL)
			func = el_expr_assign;
		else if((token = lex_try(parse->lex, TOK_ADDEQ)) != NULL)
			func = el_expr_addeq;
		else if((token = lex_try(parse->lex, TOK_SUBEQ)) != NULL)
			func = el_expr_subeq;
		else if((token = lex_try(parse->lex, TOK_MULEQ)) != NULL)
			func = el_expr_muleq;
		else if((token = lex_try(parse->lex, TOK_DIVEQ)) != NULL)
			func = el_expr_diveq;
		else if((token = lex_try(parse->lex, TOK_REMEQ)) != NULL)
			func = el_expr_remeq;
		else if((token = lex_try(parse->lex, TOK_SHLEQ)) != NULL)
			func = el_expr_shleq;
		else if((token = lex_try(parse->lex, TOK_SHREQ)) != NULL)
			func = el_expr_shreq;
		else if((token = lex_try(parse->lex, TOK_ANDEQ)) != NULL)
			func = el_expr_andeq;
		else if((token = lex_try(parse->lex, TOK_OREQ)) != NULL)
			func = el_expr_oreq;
		else if((token = lex_try(parse->lex, TOK_XOREQ)) != NULL)
			func = el_expr_xoreq;
		else
			break;

		tag = parse_tag(parse, token);

		chkret(parse_assign(parse, &rhs));
		*expr = func(*expr, rhs, tag);
	}

	return NULL;
}

static char *parse_tern(struct parse_t *parse, struct el_expr_t **expr)
{
	struct el_expr_t *ontrue, *onfalse;

	onerr();

	chkret(parse_bin(parse, expr, bins));

	if(lex_try(parse->lex, '?') == NULL)
		return NULL;

	chkret(parse_bin(parse, &ontrue, bins));

	if(lex_try(parse->lex, ':') == NULL)
		failmsg(lex_error(parse->lex, "Expected ':'."));

	chkret(parse_bin(parse, &onfalse, bins));

	*expr = el_expr_tern(el_tern_new(*expr, ontrue, onfalse), el_tag_copy((*expr)->tag));
	return NULL;
}

/**
 * Parse a binary operator.
 *   @parse: The parser.
 *   @expr: Out. THe expression.
 *   @bin: The binary list.
 *   &returns: Error.
 */
static char *parse_bin(struct parse_t *parse, struct el_expr_t **expr, struct bin_t *bin)
{
	struct el_expr_t *rhs;
	struct lex_token_t *token;
	struct el_tag_t *tag = NULL;
	struct bin_t *iter, *inc;

	onerr( if(*expr != NULL) el_expr_delete(*expr); if(tag != NULL) el_tag_delete(tag); *expr = NULL; );

	if(bin->func == NULL)
		return parse_call(parse, expr);

	*expr = NULL;

	inc = bin;
	while(!inc->inc)
		inc++;

	inc++;

	chkret(parse_bin(parse, expr, inc));

	while(true) {
		iter = bin;
		while(true) {
			if((token = lex_try(parse->lex, iter->tok)) != NULL)
				break;
			else if(iter->inc)
				return NULL;

			iter++;
		}

		tag = parse_tag(parse, token);

		chkret(parse_bin(parse, &rhs, inc));
		*expr = iter->func(*expr, rhs, tag);
	}

	return NULL;
}

/**
 * Parse a call expression.
 *   @parse: The parser.
 *   @expr: Out. The expression.
 *   &returns: Error.
 */
static char *parse_call(struct parse_t *parse, struct el_expr_t **expr)
{
	return parse_pre(parse, expr);
	
	struct el_call_t *call = NULL;

	onerr( if(call != NULL) el_call_delete(call); *expr = NULL; );

	chkret(parse_pre(parse, expr));

	if(lex_try(parse->lex, '(') != NULL)
		call = el_call_new(*expr, false);
	else
		return NULL;

	if(lex_try(parse->lex, ')') == NULL) {
		struct el_expr_t *tmp;
		struct el_arg_t **arg = &call->arg;

		while(true) {
			chkret(parse_expr(parse, &tmp));

			*arg = el_arg_new(tmp);
			arg = &(*arg)->next;
			call->cnt++;

			if(lex_try(parse->lex, ')') != NULL)
				break;
			else if(lex_try(parse->lex, ',') == NULL)
				failmsg(lex_error(parse->lex, "Expected ',' or ')'."));
		}
	}

	*expr = el_expr_call(call, el_tag_copy((*expr)->tag));

	return NULL;
}

/**
 * Parse a prefix expression.
 *   @ctx: The context.
 *   @expr: Out. The expression.
 *   &returns: Error.
 */
static char *parse_pre(struct parse_t *ctx, struct el_expr_t **expr)
{
	struct el_tag_t *tag;
	struct lex_token_t *token;

	onerr( el_tag_delete(tag); );

	if((token = lex_try(ctx->lex, '+')) != NULL)
		return parse_pre(ctx, expr);
	else if((token = lex_try(ctx->lex, '-')) != NULL) {
		tag = parse_tag(ctx, token);

		chkret(parse_pre(ctx, expr));
		*expr = el_expr_neg(*expr, tag);
	}
	else if((token = lex_try(ctx->lex, '!')) != NULL) {
		tag = parse_tag(ctx, token);

		chkret(parse_pre(ctx, expr));
		*expr = el_expr_lnot(*expr, tag);
	}
	else if((token = lex_try(ctx->lex, '~')) != NULL) {
		tag = parse_tag(ctx, token);

		chkret(parse_pre(ctx, expr));
		*expr = el_expr_not(*expr, tag);
	}
	else if((token = lex_try(ctx->lex, '&')) != NULL) {
		tag = parse_tag(ctx, token);

		chkret(parse_pre(ctx, expr));
		*expr = el_expr_addr(*expr, tag);
	}
	else if((token = lex_try(ctx->lex, '*')) != NULL) {
		tag = parse_tag(ctx, token);

		chkret(parse_pre(ctx, expr));
		*expr = el_expr_deref(*expr, tag);
	}
	else
		return parse_post(ctx, expr);

	return NULL;
}

/**
 * Parse a sequence of postfix expression.
 *   @ctx: The context.
 *   @expr: Out. The expression.
 *   &returns: Error.
 */
static char *parse_post(struct parse_t *ctx, struct el_expr_t **expr)
{
	onerr( );

	chkret(parse_term(ctx, expr));

	while(true) {
		if(lex_try(ctx->lex, TOK_ARROW)) {
			char *id;

			chkret(parse_ident(ctx, &id));
			*expr = el_expr_elem(el_elem_new(true, *expr, id), el_tag_copy((*expr)->tag));
		}
		else if(lex_try(ctx->lex, '.')) {
			char *id;

			chkret(parse_ident(ctx, &id));
			*expr = el_expr_elem(el_elem_new(false, *expr, id), el_tag_copy((*expr)->tag));
		}
		else if(lex_try(ctx->lex, '(')) {
			struct el_expr_t *tmp;
			struct el_call_t *call;
			struct el_arg_t **arg;

			call = el_call_new(*expr, false);
			*expr = el_expr_call(call, el_tag_copy((*expr)->tag));
			arg = &call->arg;

			if(!lex_try(ctx->lex, ')')) {
				while(true) {
					chkret(parse_expr(ctx, &tmp));

					*arg = el_arg_new(tmp);
					arg = &(*arg)->next;
					call->cnt++;

					if(lex_try(ctx->lex, ')') != NULL)
						break;
					else if(lex_try(ctx->lex, ',') == NULL)
						failmsg(lex_error(ctx->lex, "Expected ',' or ')'."));
				}
			}
		}
		else if(lex_try(ctx->lex, TOK_CALL)) {
			struct el_expr_t *tmp;
			struct el_call_t *call;
			struct el_arg_t **arg;

			call = el_call_new(*expr, true);
			*expr = el_expr_call(call, el_tag_copy((*expr)->tag));
			arg = &call->arg;

			if(!lex_try(ctx->lex, ')')) {
				while(true) {
					chkret(parse_expr(ctx, &tmp));

					*arg = el_arg_new(tmp);
					arg = &(*arg)->next;
					call->cnt++;

					if(lex_try(ctx->lex, ')') != NULL)
						break;
					else if(lex_try(ctx->lex, ',') == NULL)
						failmsg(lex_error(ctx->lex, "Expected ',' or ')'."));
				}
			}
		}
		else if(lex_try(ctx->lex, '[')) {
			struct el_expr_t *off;

			chkret(parse_expr(ctx, &off));
			*expr = el_expr_index(el_index_new(false, *expr, off), el_tag_copy((*expr)->tag));

			if(!lex_try(ctx->lex, ']'))
				failmsg(lex_error(ctx->lex, "Expected ']'."));
		}
		else if(lex_try(ctx->lex, TOK_INC))
			*expr = el_expr_postinc(*expr, parse_tag(ctx, lex_last(ctx->lex)));
		else if(lex_try(ctx->lex, TOK_DEC))
			*expr = el_expr_postdec(*expr, parse_tag(ctx, lex_last(ctx->lex)));
		else
			return NULL;
	}
}


bool str_postfix(const char *str, const char *post)
{
	int32_t i = strlen(str) - 1, j = strlen(post) - 1;

	while((i >= 0) && (j >= 0)) {
		if(str[i--] != post[j--])
			return false;
	}

	return j < 0;
}

/**
 * Parse a terminal.
 *   @parse: The parser.
 *   @expr: Out. The expression.
 *   &returns: Error.
 */
static char *parse_term(struct parse_t *ctx, struct el_expr_t **expr)
{
	struct el_tag_t *tag;
	struct lex_token_t *token;

	onerr( el_tag_delete(tag); );

	tag = parse_tag(ctx, lex_peek(ctx->lex));

	if(lex_try(ctx->lex, '(') != NULL) {
		chkret(parse_expr(ctx, expr));

		if(lex_try(ctx->lex, ')') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ')'."));

		el_tag_delete(tag);
	}
	else if(peek_type(ctx)) {
		struct el_type_t type;
		struct el_init_t *head;

		type = el_type_void();
		chkret(parse_type(ctx, &type, true));

		chkret(parse_init(ctx, &head));
		*expr = el_expr_init(type, head, tag);
	}
	else if((token = lex_try(ctx->lex, LEX_NUM)) != NULL) {
		if(str_postfix(token->str, "i64")) {
			uint32_t len = strlen(token->str) - 3;
			char *tmp = malloc(len + 1);
			char *endptr;
			memcpy(tmp, token->str, len);
			tmp[len] = '\0';

			errno = 0;
			int64_t v = strtol(tmp, &endptr, 0);
			if(errno != 0)
				fail("Invalid number '%s'.", token->str);

			*expr = el_expr_const(el_const_i64(v), tag);
		}
		else{
			struct el_num_t *num;

			chkret(el_num_read(&num, token->str));
			*expr = el_expr_const(el_const_num(num), tag);
		}
	}
	else if(lex_try(ctx->lex, TOK_ARGSINIT) != NULL) {
		*expr = el_expr_initargs(tag);

		if(!lex_try(ctx->lex, '('))
			failmsg(lex_error(ctx->lex, "Expected '('."));

		if(!lex_try(ctx->lex, ')'))
			failmsg(lex_error(ctx->lex, "Expected ')'."));
	}
	else if(lex_try(ctx->lex, TOK_ARGSGET) != NULL) {
		struct el_type_t type;
		struct el_expr_t *args;

		if(!lex_try(ctx->lex, '('))
			failmsg(lex_error(ctx->lex, "Expected '('."));

		chkret(parse_expr(ctx, &args));

		if(!lex_try(ctx->lex, ','))
			failmsg(lex_error(ctx->lex, "Expected ','."));

		chkret(parse_type(ctx, &type, true));

		if(!lex_try(ctx->lex, ')'))
			failmsg(lex_error(ctx->lex, "Expected ')'."));

		*expr = el_expr_getarg(el_getarg_new(args, type), tag);
	}
	else if(lex_try(ctx->lex, TOK_EGET) != NULL) {
		struct el_expr_t *val;

		if(!lex_try(ctx->lex, '('))
			failmsg(lex_error(ctx->lex, "Expected '('."));

		chkret(parse_expr(ctx, &val));

		if(!lex_try(ctx->lex, ')'))
			failmsg(lex_error(ctx->lex, "Expected ')'."));

		*expr = el_expr_eget(val, tag);
	}
	else if(lex_try(ctx->lex, TOK_EREQ) != NULL) {
		struct el_expr_t *val;

		if(!lex_try(ctx->lex, '('))
			failmsg(lex_error(ctx->lex, "Expected '('."));

		chkret(parse_expr(ctx, &val));

		if(!lex_try(ctx->lex, ')'))
			failmsg(lex_error(ctx->lex, "Expected ')'."));

		*expr = el_expr_ereq(val, tag);
	}
	else if(lex_try(ctx->lex, TOK_MEMNEW) != NULL) {
		struct el_call_t *call;
		struct el_type_t type;

		if(!lex_try(ctx->lex, '('))
			failmsg(lex_error(ctx->lex, "Expected '('."));

		chkret(parse_type(ctx, &type, true));
		call = el_call_new(el_expr_ident(el_ident_unk(strdup("mem:alloc")), el_tag_copy(tag)), false);
		call->arg = el_arg_new(el_expr_sizeof(el_sizeof_new(type, NULL), el_tag_copy(tag)));

		*expr = el_expr_call(call, tag);

		if(!lex_try(ctx->lex, ')'))
			failmsg(lex_error(ctx->lex, "Expected ')'."));
	}
	else if(lex_try(ctx->lex, TOK_MEMMAKE) != NULL) {
		struct el_call_t *call;

		if(!lex_try(ctx->lex, '('))
			failmsg(lex_error(ctx->lex, "Expected '('."));

		chkret(parse_expr(ctx, expr));
		if((*expr)->kind != el_init_v)
			failmsg(lex_error(ctx->lex, "Call to `mem:make` requires an initializer."));

		call = el_call_new(el_expr_ident(el_ident_unk(strdup("mem:dup")), el_tag_copy(tag)), false);
		call->arg = el_arg_new(el_expr_addr(*expr, el_tag_copy(tag)));
		call->arg->next = el_arg_new(el_expr_sizeof(el_sizeof_new(el_type_copy((*expr)->type), NULL), el_tag_copy(tag)));

		*expr = el_expr_call(call, tag);

		if(!lex_try(ctx->lex, ')'))
			failmsg(lex_error(ctx->lex, "Expected ')'."));
	}
	else if(lex_try(ctx->lex, TOK_NULL) != NULL) {
		//*expr = el_expr_const(el_const_ptr(el_num_zero(), el_type_ptr(el_ptr_new(0, el_type_void()))), tag);
		*expr = el_expr_const(el_const_num(el_num_zero()), tag);
	}
	else if(lex_try(ctx->lex, TOK_TRUE) != NULL) {
		*expr = el_expr_const(el_const_bool(true), tag);
	}
	else if(lex_try(ctx->lex, TOK_FALSE) != NULL) {
		*expr = el_expr_const(el_const_bool(false), tag);
	}
	else if(lex_peek(ctx->lex)->id == LEX_ID) {
		char *id;

		chkret(parse_ident(ctx, &id));
		*expr = el_expr_ident(el_ident_unk(id), tag);
	}
	else if((token = lex_try(ctx->lex, TOK_QUOTE1)) != NULL) {
		if(strlen(token->str) > 1)
			failmsg(lex_error(ctx->lex, "Invalid character '%s'.", token->str));

		*expr = el_expr_const(el_const_num(el_num_i64(token->str[0])), tag);
	}
	else if((token = lex_try(ctx->lex, TOK_QUOTE2)) != NULL) {
		*expr = el_expr_const(el_const_str(strdup(token->str)), tag);
	}
	else if(lex_try(ctx->lex, TOK_SIZEOF) != NULL)
		return parse_sizeof(ctx, tag, expr);
	else if(lex_try(ctx->lex, TOK_OFFSETOF) != NULL) {
		char *id;
		struct el_type_t type;

		if(lex_try(ctx->lex, '(') == NULL)
			failmsg(lex_error(ctx->lex, "Expected '('."));

		chkret(parse_type(ctx, &type, true));

		if(lex_try(ctx->lex, ',') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ','."));

		chkret(parse_ident(ctx, &id));

		if(lex_try(ctx->lex, ')') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ')'."));

		*expr = el_expr_offsetof(el_offsetof_new(type, id), tag);
	}
	else if(lex_try(ctx->lex, TOK_GETPARENT) != NULL) {
		char *id;
		struct el_expr_t *sub;
		struct el_type_t type;

		if(lex_try(ctx->lex, '(') == NULL)
			failmsg(lex_error(ctx->lex, "Expected '('."));

		chkret(parse_expr(ctx, &sub));

		if(lex_try(ctx->lex, ',') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ','."));

		chkret(parse_type(ctx, &type, true));

		if(lex_try(ctx->lex, ',') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ','."));

		chkret(parse_ident(ctx, &id));

		if(lex_try(ctx->lex, ')') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ')'."));

		*expr = el_expr_getparent(el_getparent_new(sub, type, id), tag);
	}
	else if(lex_try(ctx->lex, TOK_ARRLEN) != NULL) {
		struct el_expr_t *inner;

		if(lex_try(ctx->lex, '(') == NULL)
			failmsg(lex_error(ctx->lex, "Expected '('."));

		chkret(parse_expr(ctx, &inner));

		if(lex_try(ctx->lex, ')') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ')'."));

		*expr = el_expr_arrlen(inner, tag);
	}
	else if(lex_try(ctx->lex, TOK_GETREF) != NULL) {
		struct el_expr_t *inner;

		if(lex_try(ctx->lex, '(') == NULL)
			failmsg(lex_error(ctx->lex, "Expected '('."));

		chkret(parse_expr(ctx, &inner));

		if(lex_try(ctx->lex, ')') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ')'."));

		*expr = el_expr_getref(inner, tag);
	}
	else if(lex_try(ctx->lex, TOK_EV) != NULL) {
		char *id, *en, *val, *colon;

		if(lex_try(ctx->lex, ':') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ':'."));

		chkret(parse_ident(ctx, &id));
		colon = strrchr(id, ':');
		if(colon == NULL)
			failmsg(lex_error(ctx->lex, "Enumerated value requires ':'."));

		en = mprintf("en:%.*s", (int)(colon - id), id);
		val = strdup(colon + 1);

		*expr = el_expr_enval(el_enval_new(en, val), tag);

		free(id);
	}
	else
		failmsg(lex_error(ctx->lex, "Expected expression."));

	return NULL;
}


/**
 * Parse a sizeof expression.
 *   @ctx: The context.
 *   @tag: Consumed. The tag.
 *   @expr: Out. The expression.
 *   &returns: Error.
 */
static char *parse_sizeof(struct parse_t *ctx, struct el_tag_t *tag, struct el_expr_t **expr)
{
	struct el_sizeof_t *size = NULL;

	onerr( el_tag_delete(tag); if(size != NULL) el_sizeof_delete(size); );

	if(lex_try(ctx->lex, '(') == NULL)
		failmsg(lex_error(ctx->lex, "Expected '('."));

	if(peek_type(ctx)) {
		struct el_type_t type;

		chkret(parse_type(ctx, &type, true));
		size = el_sizeof_new(type, NULL);
	}
	else {
		struct el_expr_t *expr;

		chkret(parse_expr(ctx, &expr));
		size = el_sizeof_new(el_type_void(), expr);
	}

	if(lex_try(ctx->lex, ')') == NULL)
		failmsg(lex_error(ctx->lex, "Expected ')'."));

	*expr = el_expr_sizeof(size, tag);

	return NULL;
}


/**
 * Parse a tag from a token.
 *   @parse: The parser.
 *   @token: The token.
 *   &returns: The tag.
 */
static struct el_tag_t *parse_tag(struct parse_t *parse, struct lex_token_t *token)
{
	return el_tag_new(el_str_copy(parse->path), token->line, token->col);
}


/**
 * Create the prefix asssociated with the type.
 *   @kind: The kind.
 *   &returns: The prefix.
 */
static const char *parse_prefix(int32_t kind)
{
	switch(kind) {
	case TOK_TY: return "ty";
	case TOK_ST: return "st";
	case TOK_UN: return "un";
	case TOK_EN: return "en";
	}

	unreachable();
}


/**
 * Peek to see if a type is next.
 *   @ctx: The context.
 *   &returns: True if a type.
 */
static bool peek_type(struct parse_t *ctx)
{
	uint32_t i;
	int32_t tok, toks[] = { TOK_AR, TOK_ST, TOK_UN, TOK_EN, TOK_PT, TOK_CPT, TOK_VOID, TOK_U8, TOK_I8, TOK_U16, TOK_I16, TOK_U32, TOK_I32, TOK_I64, TOK_U64, TOK_CHAR, TOK_BOOL, TOK_ARGSTYPE, 0 };

	tok = lex_peek(ctx->lex)->id;

	for(i = 0; toks[i] != 0; i++) {
		if(tok == toks[i])
			return true;
	}
	
	return false;
}


/**
 * Parse an initializer.
 *   @ctx: The context.
 *   @init: Out. The initializer.
 *   &returns: Error.
 */
static char *parse_init(struct parse_t *ctx, struct el_init_t **init)
{
	struct el_init_t **iter;

	onerr( el_init_clear(*init); );

	*init = NULL;
	iter = init;

	if(lex_try(ctx->lex, '{') == NULL)
		failmsg(lex_error(ctx->lex, "Expected '{'."));

	while(lex_try(ctx->lex, '}') == NULL) {
		char *id = NULL;

		if(lex_try(ctx->lex, '.')) {
			chkret(parse_ident(ctx, &id));

			if(!lex_try(ctx->lex, '='))
				failmsg(lex_error(ctx->lex, "Expected '='."));
		}

		if(lex_peek(ctx->lex)->id == '{') {
			struct el_init_t *nest;

			chkret(parse_init(ctx, &nest));

			*iter = el_init_new(true, (union el_init_u){ .nest = nest }, id);
			iter = &(*iter)->next;
		}
		else {
			struct el_expr_t *expr;

			chkret(parse_expr(ctx, &expr));

			*iter = el_init_expr(expr, id);
			iter = &(*iter)->next;
		}

		if(lex_try(ctx->lex, '}') != NULL)
			break;
		else if(lex_try(ctx->lex, ',') == NULL)
			failmsg(lex_error(ctx->lex, "Expected ',' or '}'."));
	}

	return NULL;
}
