#!/bin/sh

## 
# Hammer build system
#   version 1.0.0dev1
##

TMP=".hammer.c"
BIN="hammer.cache"
CC="gcc"

test $VERBOSE && echo "cc=$CC"

sed '0,/\#\#csrc\#\#/d' "$0" > "$TMP" || exit $?
"$CC" -Wall -O2 "$TMP" -o "$BIN" || exit $?

exit 0
##csrc##
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct ast_cmd_t;
struct ast_pipe_t;
struct cmd_t;
struct rt_ctx_t;
struct env_t;
struct imm_t;
struct list_t;
struct queue_t;
struct ns_t;
struct raw_t;
struct rd_t;
struct rule_t;
struct rt_obj_t;
struct rt_pipe_t;
struct seq_t;
struct set_t;
struct target_t;
struct target_list_t;
struct val_t;
struct loc_t;
struct tok_t;
struct val_t;
typedef int(*cmp_f)(const void *, const void *);
typedef void(*del_f)(void *);
typedef struct rt_obj_t(func_t)(struct rt_obj_t *args, uint32_t cnt, struct loc_t loc);
uint64_t hash64(uint64_t hash, const char *str);
void memswap(void *lhs, void *rhs, size_t len);
struct ast_block_t *ham_load(const char *path);
extern int64_t os_memcnt;
void print(const char *fmt, ...);
void printv(const char *fmt, va_list args);
void _fatal(const char *path, unsigned long line, const char *fmt, ...) __attribute__((noreturn));
void unreachable() __attribute__((noreturn));
#define fatal(...) _fatal(__FILE__, __LINE__, __VA_ARGS__)
void os_init(void);
int os_exec(struct cmd_t *cmd);
int os_wait(void);
int64_t os_mtime(const char *path);
void os_mkdir(const char *path);
void mk_eval(struct rt_ctx_t *ctx, const char *path, bool strict);
void mk_trim(FILE *file, int *ch);
char *mk_str(FILE *file, int *ch);
bool mk_space(int ch);
bool mk_ident(int ch);
void args_init(struct rt_obj_t **args, uint32_t *cnt);
void args_add(struct rt_obj_t **args, uint32_t *cnt, struct rt_obj_t obj);
void args_delete(struct rt_obj_t *args, uint32_t cnt);
struct list_t {
struct link_t *head, **tail;
void (*del)(void *);
};
struct link_t {
void *val;
struct link_t *next;
};
struct list_t *list_new(void(*del)(void *));
void list_delete(struct list_t *list);
void list_add(struct list_t *list, void *val);
struct opt_t {
bool force;
int jobs;
const char *dir;
};
struct rule_t {
char *id;
struct target_list_t *gens, *deps;
struct seq_t *seq;
bool add;
uint32_t edges;
};
struct rule_list_t {
struct rule_inst_t *inst;
};
struct rule_inst_t {
struct rule_t *rule;
struct rule_inst_t *next;
};
struct rule_iter_t {
struct rule_inst_t *inst;
};
struct rule_t *rule_new(char *id, struct target_list_t *gens, struct target_list_t *deps, struct seq_t *seq);
void rule_delete(struct rule_t *rule);
struct rule_iter_t rule_iter(struct rule_list_t *list);
struct rule_t *rule_next(struct rule_iter_t *iter);
struct rule_list_t *rule_list_new(void);
void rule_list_delete(struct rule_list_t *list);
void rule_list_add(struct rule_list_t *list, struct rule_t *rule);
struct queue_t *queue_new(void);
void queue_delete(struct queue_t *queue);
void queue_recur(struct queue_t *queue, struct rule_t *rule);
void queue_add(struct queue_t *queue, struct rule_t *rule);
struct rule_t *queue_rem(struct queue_t *queue);
struct target_t {
char *path;
uint32_t flags;
int64_t mtime;
struct rule_t *rule;
struct edge_t *edge;
};
struct target_list_t {
struct target_inst_t *inst;
};
struct target_inst_t {
struct target_t *target;
struct target_inst_t *next;
};
struct target_iter_t {
struct target_inst_t *inst;
};
struct edge_t {
struct rule_t *rule;
struct edge_t *next;
};
#define FLAG_BUILD (1 << 0)
#define FLAG_SPEC (1 << 1)
struct target_t *rt_ref_new(bool spec, char *path);
void rt_ref_delete(struct target_t *ref);
int64_t target_mtime(struct target_t *target);
void target_conn(struct target_t *target, struct rule_t *rule);
bool target_equal(const struct target_t *lhs, const struct target_t *rhs);
struct target_iter_t target_iter(struct target_list_t *list);
struct target_t *target_next(struct target_iter_t *iter);
struct target_list_t *target_list_new(void);
void target_list_delete(struct target_list_t *list);
uint32_t target_list_len(struct target_list_t *list);
bool target_list_contains(struct target_list_t *list, struct target_t *target);
void target_list_add(struct target_list_t *list, struct target_t *target);
struct target_t *target_list_find(struct target_list_t *list, bool spec, const char *path);
struct map_t {
struct ent_t *ent;
};
struct ent_t {
struct target_t *target;
struct ent_t *next;
};
struct map_t *map_new(void);
void map_delete(struct map_t *map);
struct target_t *map_get(struct map_t *map, bool spec, const char *path);
void map_add(struct map_t *map, struct target_t *target);
struct seq_t {
struct cmd_t *head, **tail;
};
struct cmd_t {
struct rt_pipe_t *pipe;
char *in, *out;
bool append;
struct cmd_t *next;
};
struct seq_t *seq_new(void);
void seq_delete(struct seq_t *seq);
void seq_add(struct seq_t *seq, struct rt_pipe_t *pipe, char *in, char *out, bool append);
enum rt_obj_e { rt_null_v, rt_val_v, rt_env_v, rt_func_v };
union rt_obj_u { struct val_t *val; struct env_t *env; func_t *func; };
struct rt_obj_t {
enum rt_obj_e tag;
union rt_obj_u data;
};
struct rt_ctx_t {
const struct opt_t *opt;
struct map_t *map;
struct rule_list_t *rules;
struct rule_t *cur;
struct target_list_t *gens, *deps;
};
struct rt_ctx_t *ctx_new(const struct opt_t *opt);
void ctx_delete(struct rt_ctx_t *ctx);
void ctx_run(struct rt_ctx_t *ctx, const char **builds);
struct target_t *ctx_target(struct rt_ctx_t *ctx, bool spec, const char *path);
struct rule_t *ctx_rule(struct rt_ctx_t *ctx, const char *id, struct target_list_t *gens, struct target_list_t *deps);
struct val_t *rt_eval_val(struct imm_t *imm, struct rt_ctx_t *ctx, struct env_t *env, struct loc_t loc);
char *rt_eval_str(struct raw_t *raw, struct rt_ctx_t *ctx, struct env_t *env, struct loc_t loc);
struct rt_obj_t rt_obj_new(enum rt_obj_e tag, union rt_obj_u data);
struct rt_obj_t rt_obj_dup(struct rt_obj_t obj);
void rt_obj_delete(struct rt_obj_t obj);
void rt_obj_set(struct rt_obj_t *dst, struct rt_obj_t src);
struct rt_obj_t rt_obj_null(void);
struct rt_obj_t rt_obj_val(struct val_t *val);
struct rt_obj_t rt_obj_env(struct env_t *env);
struct rt_obj_t rt_obj_func(func_t *func);
void rt_obj_add(struct rt_obj_t dst, struct rt_obj_t src, struct loc_t loc);
struct rt_pipe_t {
struct val_t *cmd;
struct rt_pipe_t *next;
};
struct rt_pipe_t *rt_pipe_new(struct val_t *cmd);
void rt_pipe_clear(struct rt_pipe_t *pipe);
struct node_t {
char *path;
uint64_t hash;
uint32_t flags;
uint32_t deps;
struct set_t *edge;
};
#define NODE_SPEC (1 << 0)
struct node_t *node_new(bool spec, char *path);
void node_delete(struct node_t *node);
struct loc_t {
const char *path;
uint32_t lin, col;
};
extern char *cli_app;
void cli_proc(char **args);
void cli_err(const char *fmt, ...) __attribute__((noreturn));
struct loc_t loc_off(struct loc_t loc, uint32_t off);
void loc_err(struct loc_t loc, const char *fmt, ...) __attribute__((noreturn));
struct val_t {
bool spec;
char *str;
struct val_t *next;
};
struct val_t *val_new(bool spec, char *str);
struct val_t *val_dup(const struct val_t *val);
struct val_t *rt_val_copy(const struct val_t *val);
void val_delete(struct val_t *val);
void val_clear(struct val_t *val);
char *val_id(struct val_t *val, struct loc_t loc);
char *val_str(struct val_t *val, struct loc_t loc);
uint32_t val_len(struct val_t *val);
struct val_t **val_tail(struct val_t **val);
struct val_t *rt_val_get(struct val_t *val, const char *str);
struct bind_t {
char *id;
struct rt_obj_t obj;
struct loc_t loc;
struct bind_t *next;
};
struct bind_t *bind_new(char *id, struct rt_obj_t obj, struct loc_t loc);
void bind_delete(struct bind_t *bind);
void bind_erase(struct bind_t *bind);
void bind_set(struct bind_t **dst, struct bind_t *src);
void bind_reset(struct bind_t *bind, struct rt_obj_t obj, struct loc_t loc);
char *get_str(const char **str, struct loc_t loc);
void get_var(const char **str, char *var, struct loc_t loc);
void str_trim(const char **str);
char *str_fmt(const char *pat, ...);
void str_set(char **dst, char *src);
void str_final(char **str, const char *dir);
struct buf_t {
char *str;
uint32_t len, max;
};
struct buf_t buf_new(uint32_t init);
void buf_delete(struct buf_t *buf);
char *buf_done(struct buf_t *buf);
void buf_ch(struct buf_t *buf, char ch);
void buf_mem(struct buf_t *buf, const char *mem, uint32_t len);
void buf_str(struct buf_t *buf, const char *str);
bool ch_space(int ch);
bool ch_alpha(int ch);
bool ch_num(int ch);
bool ch_alnum(int ch);
bool ch_var(int ch);
bool ch_str(int ch);
bool ch_id(int ch);
struct ns_t {
char *id;
struct ns_t *up, *next;
struct bind_t *bind;
};
struct ns_t *ns_new(char *id, struct ns_t *up);
void ns_delete(struct ns_t *ns);
struct bind_t *ns_get(struct ns_t *ns, const char *id);
void ns_add(struct ns_t *ns, struct bind_t *bind);
struct bind_t *ns_find(struct ns_t *ns, const char *id);
struct bind_t **ns_lookup(struct ns_t *ns, const char *id);
struct ast_inc_t {
bool nest, opt;
struct imm_t *imm;
};
struct ast_inc_t *ast_inc_new(bool nest, bool opt, struct imm_t *imm);
void ast_inc_delete(struct ast_inc_t *inc);
void ast_inc_eval(struct ast_inc_t *inc, struct rt_ctx_t *ctx, struct env_t *env, struct loc_t loc);
struct ast_rule_t {
struct imm_t *gen, *dep;
struct list_t *cmd;
struct loc_t loc;
};
struct ast_rule_t *ast_rule_new(struct imm_t *gen, struct imm_t *dep, struct loc_t loc);
void ast_rule_delete(struct ast_rule_t *syn);
void ast_rule_add(struct ast_rule_t *syn, struct imm_t *cmd);
enum ast_bind_e { ast_val_v, ast_func_v, ast_block_v };
union ast_bind_u { struct imm_t *val; void *func; struct ast_block_t *block; };
struct ast_bind_t {
struct raw_t *id;
enum ast_bind_e tag;
union ast_bind_u data;
bool add;
};
struct ast_bind_t *ast_bind_new(struct raw_t *id, enum ast_bind_e tag, union ast_bind_u data, bool add);
void ast_bind_delete(struct ast_bind_t *bind);
struct ast_bind_t *ast_bind_val(struct raw_t *id, struct imm_t *val, bool add);
struct ast_bind_t *ast_bind_block(struct raw_t *id, struct ast_block_t *block, bool add);
struct ast_block_t {
struct ast_stmt_t *stmt;
};
struct ast_block_t *ast_block_new(void);
void ast_block_delete(struct ast_block_t *block);
struct ast_cmd_t {
struct ast_pipe_t *pipe;
struct raw_t *in, *out;
bool append;
};
struct ast_cmd_t *ast_cmd_new(struct ast_pipe_t *pipe);
void ast_cmd_delete(struct ast_cmd_t *cmd);
struct ast_mkdep_t {
struct imm_t *path;
struct loc_t loc;
};
struct ast_mkdep_t *ast_mkdep_new(struct imm_t *path, struct loc_t loc);
void ast_mkdep_delete(struct ast_mkdep_t *dep);
void ast_mkdep_eval(struct ast_mkdep_t *dep, struct rt_ctx_t *ctx, struct env_t *env);
struct ast_pipe_t {
struct imm_t *imm;
struct ast_pipe_t *next;
};
struct ast_pipe_t *ast_pipe_new(struct imm_t *imm);
void ast_pipe_clear(struct ast_pipe_t *bar);
struct cond_t {
};
struct loop_t {
char *id;
struct imm_t *imm;
struct ast_stmt_t *body;
struct loc_t loc;
};
struct loop_t *loop_new(char *id, struct imm_t *imm, struct ast_stmt_t *body, struct loc_t loc);
void loop_delete(struct loop_t *loop);
enum stmt_e { ast_bind_v, syn_v, loop_v, print_v, ast_mkdep_v, block_v, ast_inc_v };
union stmt_u { struct ast_bind_t *bind; struct ast_rule_t *syn; struct cond_t *conf; struct loop_t *loop; struct print_t *print; struct ast_mkdep_t *mkdep; struct ast_block_t *block; struct ast_inc_t *inc; };
struct ast_stmt_t {
enum stmt_e tag;
union stmt_u data;
struct loc_t loc;
struct ast_stmt_t *next;
};
struct ast_stmt_t *stmt_new(enum stmt_e tag, union stmt_u data, struct loc_t loc);
void stmt_delete(struct ast_stmt_t *stmt);
void stmt_clear(struct ast_stmt_t *stmt);
struct ast_stmt_t *ast_stmt_mkdep(struct ast_mkdep_t *mkdep, struct loc_t loc);
struct ast_stmt_t *ast_stmt_inc(struct ast_inc_t *inc, struct loc_t loc);
struct print_t {
struct imm_t *imm;
};
struct print_t *print_new(struct imm_t *imm);
void print_delete(struct print_t *print);
struct imm_t {
struct raw_t *raw;
};
struct imm_t *imm_new(void);
void imm_delete(struct imm_t *imm);
uint32_t imm_len(struct imm_t *imm);
struct raw_t {
bool spec, var;
char *str;
struct loc_t loc;
struct raw_t *next;
};
struct raw_t *raw_new(bool spec, bool var, char *str, struct loc_t loc);
struct raw_t *raw_dup(const struct raw_t *raw);
void raw_delete(struct raw_t *raw);
void raw_clear(struct raw_t *raw);
struct env_t {
uint32_t nrefs;
struct map0_t *map;
struct env_t *next;
};
void rt_eval_top(struct ast_block_t *block, struct rt_ctx_t *ctx, struct env_t *env);
void eval_block(struct ast_block_t *block, struct rt_ctx_t *ctx, struct env_t *env);
void eval_stmt(struct ast_stmt_t *stmt, struct rt_ctx_t *ctx, struct env_t *env);
struct rt_obj_t eval_imm(struct imm_t *imm, struct rt_ctx_t *ctx, struct env_t *env, struct loc_t loc);
struct rt_obj_t eval_raw(struct raw_t *raw, struct rt_ctx_t *ctx, struct env_t *env);
struct rt_obj_t eval_var(const char **str, struct loc_t loc, struct rt_ctx_t *ctx, struct env_t *env);
struct env_t *rt_env_new(struct env_t *next);
struct env_t *rt_env_dup(struct env_t *env);
void rt_env_delete(struct env_t *env);
void rt_env_clear(struct env_t *env);
struct bind_t *rt_env_lookup(struct env_t *env, const char *id);
struct bind_t *env_get(struct env_t *env, const char *id);
void env_put(struct env_t *env, struct bind_t *bind);
struct env_t **rt_env_tail(struct env_t **env);
struct job_t {
int pid;
struct rule_t *rule;
struct cmd_t *cmd;
};
struct ctrl_t {
struct queue_t *queue;
struct job_t *job;
uint32_t cnt;
};
struct ctrl_t *ctrl_new(struct queue_t *queue, uint32_t n);
void ctrl_delete(struct ctrl_t *ctrl);
void ctrl_add(struct ctrl_t *ctrl, struct rule_t *rule);
bool ctrl_avail(struct ctrl_t *ctrl);
bool ctrl_busy(struct ctrl_t *ctrl);
void ctrl_wait(struct ctrl_t *ctrl);
void ctrl_done(struct ctrl_t *ctrl, struct rule_t *rule);
int ctrl_exec(struct cmd_t *cmd);
struct rt_obj_t fn_sub(struct rt_obj_t *args, uint32_t cnt, struct loc_t loc);
struct rt_obj_t fn_pat(struct rt_obj_t *args, uint32_t cnt, struct loc_t loc);
struct rt_obj_t rt_fun_uniq(struct rt_obj_t *args, uint32_t cnt, struct loc_t loc);
struct map1_t {
struct ent1_t *ent;
void (*cmp)(const void *, const void *);
void (*del)(void *);
};
struct ent1_t {
const char *id;
void *ref;
struct ent1_t *next;
};
struct entry0_t {
const void *key;
void *val;
struct entry0_t *next;
};
struct map0_t {
cmp_f cmp;
del_f del;
struct entry0_t *entry;
};
struct map0_t *map0_new(cmp_f cmp, del_f del);
void map0_delete(struct map0_t *map);
void map0_add(struct map0_t *map, const void *key, void *val);
void *map0_get(struct map0_t *map, const void *key);
void *map_rem(struct map0_t *map, const void *key);
int main(int argc, char **argv)
{
cli_app = "hammer";
os_init();
cli_proc(argv + 1);
return 0;
}
uint64_t hash64(uint64_t hash, const char *str)
{
while(*str != '\0') {
hash += (uint64_t)*str++ ^ 0x687ebde0fd11d4ae;
hash *= 0xe278b9fdc665d444;
hash ^= (hash >> 53);
hash *= 0x19ba967de10aacc0;
hash ^= (hash >> 48);
hash *= 0x1b2b216668c9e998;
hash ^= (hash >> 43);
hash *= 0x73c3d43eb467f8df;
hash ^= (hash >> 37);
}
return hash;
}
void memswap(void *lhs, void *rhs, size_t len)
{
size_t i;
uint8_t t, *x = lhs, *y = rhs;
for(i = 0; i < len; i++) {
t = x[i];
x[i] = y[i];
y[i] = t;
}
}
struct loc_t loc_off(struct loc_t loc, uint32_t off)
{
return (struct loc_t){ loc.path, loc.lin, loc.col + off };
}
void loc_err(struct loc_t loc, const char *fmt, ...)
{
va_list args;
va_start(args, fmt);
fprintf(stderr, "%s:%u:%u: ", loc.path, loc.lin, loc.col);
vfprintf(stderr, fmt, args);
fprintf(stderr, "\n");
va_end(args);
exit(1);
}
struct loop_t *loop_new(char *id, struct imm_t *imm, struct ast_stmt_t *body, struct loc_t loc)
{
struct loop_t *loop;
loop = malloc(sizeof(struct loop_t));
*loop = (struct loop_t){ id, imm, body, loc };
return loop;
}
void loop_delete(struct loop_t *loop)
{
imm_delete(loop->imm);
stmt_delete(loop->body);
free(loop->id);
free(loop);
}
struct map0_t *map0_new(cmp_f cmp, del_f del)
{
struct map0_t *map;
map = malloc(sizeof(struct map0_t));
map->cmp = cmp;
map->del = del;
map->entry = NULL;
return map;
}
void map0_delete(struct map0_t *map)
{
struct entry0_t *entry;
while(map->entry != NULL) {
map->entry = (entry = map->entry)->next;
map->del(entry->val);
free(entry);
}
free(map);
}
void map0_add(struct map0_t *map, const void *key, void *val)
{
struct entry0_t *entry;
entry = malloc(sizeof(struct entry0_t));
entry->key = key;
entry->val = val;
entry->next = map->entry;
map->entry = entry;
}
void *map0_get(struct map0_t *map, const void *key)
{
struct entry0_t *entry;
for(entry = map->entry; entry != NULL; entry = entry->next) {
if(map->cmp(entry->key, key) == 0)
return entry->val;
}
return NULL;
}
void *map_rem(struct map0_t *map, const void *key)
{
void *val;
struct entry0_t **entry, *tmp;
for(entry = &map->entry; *entry != NULL; entry = &(*entry)->next) {
if(map->cmp((*entry)->key, key) != 0)
continue;
*entry = (tmp = *entry)->next;
val = tmp->val;
free(tmp);
return val;
}
return NULL;
}
struct print_t *print_new(struct imm_t *imm)
{
struct print_t *print;
print = malloc(sizeof(struct print_t));
print->imm = imm;
return print;
}
void print_delete(struct print_t *print)
{
imm_delete(print->imm);
free(print);
}
struct imm_t *imm_new(void)
{
struct imm_t *list;
list = malloc(sizeof(struct imm_t));
list->raw = NULL;
return list;
}
void imm_delete(struct imm_t *imm)
{
raw_clear(imm->raw);
free(imm);
}
uint32_t imm_len(struct imm_t *imm)
{
uint32_t n;
struct raw_t *raw;
n = 0;
for(raw = imm->raw; raw != NULL; raw = raw->next)
n++;
return n;
}
struct raw_t *raw_new(bool spec, bool var, char *str, struct loc_t loc)
{
struct raw_t *raw;
raw = malloc(sizeof(struct raw_t));
raw->spec = spec;
raw->var = var;
raw->str = str;
raw->loc = loc;
raw->next = NULL;
return raw;
}
struct raw_t *raw_dup(const struct raw_t *raw)
{
return raw_new(raw->spec, raw->var, strdup(raw->str), raw->loc);
}
void raw_delete(struct raw_t *raw)
{
free(raw->str);
free(raw);
}
void raw_clear(struct raw_t *raw)
{
struct raw_t *tmp;
while(raw != NULL) {
raw = (tmp = raw)->next;
raw_delete(tmp);
}
}
struct rd_t {
FILE *file;
int ch;
struct loc_t loc, tloc;
int tok;
char *str;
uint32_t len, max;
};
int rd_ch(struct rd_t *rd);
int rd_tok(struct rd_t *rd);
void rd_reset(struct rd_t *rd);
int rd_push(struct rd_t *rd, char ch);
int rd_buf(struct rd_t *rd);
struct ast_block_t *rd_top(struct rd_t *rd);
struct ast_stmt_t *rd_stmt(struct rd_t *rd);
struct imm_t *rd_imm(struct rd_t *rd);
struct raw_t *rd_raw(struct rd_t *rd);
struct ast_block_t *ham_load(const char *path)
{
struct rd_t rd;
struct ast_block_t *block;
rd.file = fopen(path, "r");
if(rd.file == NULL)
return NULL;
rd.loc.path = path;
rd.loc.lin = 1;
rd.loc.col = 0;
rd.str = malloc(64);
rd.len = 0;
rd.max = 64;
rd_ch(&rd);
rd.tok = rd_tok(&rd);
block = rd_top(&rd);
fclose(rd.file);
free(rd.str);
return block;
}
int rd_ch(struct rd_t *rd)
{
rd->ch = fgetc(rd->file);
if(rd->ch == '\n') {
rd->loc.lin++;
rd->loc.col = 0;
}
else
rd->loc.col++;
return rd->ch;
}
void rd_reset(struct rd_t *rd)
{
rd->len = 0;
}
int rd_push(struct rd_t *rd, char ch)
{
if(rd->len >= rd->max)
rd->str = realloc(rd->str, rd->max *= 2);
rd->str[rd->len++] = ch;
return rd->ch;
}
int rd_buf(struct rd_t *rd)
{
rd_push(rd, rd->ch);
return rd_ch(rd);
}
struct sym_t {
int tok;
const char *str;
};
#define TOK_STR 0x1000
#define TOK_VAR 0x1001
#define TOK_SPEC 0x1002
#define TOK_DIR 0x2000
#define TOK_FOR 0x2001
#define TOK_IF 0x2002
#define TOK_ELIF 0x2003
#define TOK_ELSE 0x2004
#define TOK_PRINT 0x2005
#define TOK_DEF 0x2006
#define TOK_MKDEP 0x2007
#define TOK_INCLUDE 0x2008
#define TOK_IMPORT 0x2009
#define TOK_SHR 0x3000
#define TOK_SHL 0x3001
#define TOK_ADDEQ 0x4000
#define TOK_EOF 0x7FFF
struct sym_t syms[] = {
{ TOK_SHR, ">>" },
{ TOK_SHL, "<<" },
{ TOK_ADDEQ, "+=" },
{ '{', "{" },
{ '}', "}" },
{ ':', ":" },
{ ';', ";" },
{ '=', "=" },
{ '<', "<" },
{ '>', ">" },
{ '|', "|" },
{ '?', "?" },
{ 0, NULL }
};
struct sym_t keys[] = {
{ TOK_DIR, "dir" },
{ TOK_FOR, "for" },
{ TOK_IF, "if" },
{ TOK_ELIF, "elif" },
{ TOK_ELSE, "else" },
{ TOK_PRINT, "print" },
{ TOK_DEF , "default" },
{ TOK_MKDEP, "makedep" },
{ TOK_INCLUDE, "include" },
{ TOK_IMPORT, "import" },
{ 0, NULL }
};
bool rd_var(struct rd_t *rd);
void rd_str(struct rd_t *rd);
bool rd_quote1(struct rd_t *rd);
bool rd_quote2(struct rd_t *rd);
bool rd_escape(struct rd_t *rd);
bool rd_escape(struct rd_t *rd)
{
if(rd->ch != '\\')
return false;
if(strchr("tn'\" ,$", rd_ch(rd)) == NULL)
loc_err(rd->loc, "Invalid escape character '\\%c'.", rd->ch);
rd_push(rd, '\\');
rd_buf(rd);
return true;
}
void rd_str(struct rd_t *rd)
{
for(;;) {
if(rd->ch == '$')
rd_var(rd);
else if(rd->ch == '\'')
rd_quote1(rd);
else if(rd->ch == '"')
rd_quote2(rd);
else if(rd->ch == '\\')
rd_escape(rd);
else if(ch_str(rd->ch))
rd_buf(rd);
else
break;
}
}
bool rd_var(struct rd_t *rd)
{
if(rd->ch != '$')
return false;
rd_buf(rd);
if(rd->ch == '{') {
rd_buf(rd);
for(;;) {
if(rd->ch == '\\')
rd_escape(rd);
else if(rd->ch == '}')
break;
else
rd_buf(rd);
}
rd_buf(rd);
}
else if(ch_var(rd->ch)) {
do
rd_buf(rd);
while(ch_var(rd->ch));
}
else if(strchr("@^<*", rd->ch) != NULL)
rd_buf(rd);
else
loc_err(rd->loc, "Invalid variable name.");
return true;
}
bool rd_quote1(struct rd_t *rd)
{
if(rd->ch != '\'')
return false;
rd_buf(rd);
for(;;) {
if(rd->ch == '\\')
rd_escape(rd);
else if(rd->ch == '\'')
break;
else if((rd->ch == '\n') || (rd->ch < 0))
loc_err(rd->loc, "Unterminated quote.", rd->ch);
else
rd_buf(rd);
}
rd_buf(rd);
return true;
}
bool rd_quote2(struct rd_t *rd)
{
if(rd->ch != '"')
return false;
rd_buf(rd);
for(;;) {
if(rd->ch == '\\')
rd_escape(rd);
else if(rd->ch == '$')
rd_var(rd);
else if(rd->ch == '"')
break;
else if((rd->ch == '\n') || (rd->ch < 0))
loc_err(rd->loc, "Unterminated quote.", rd->ch);
else
rd_buf(rd);
}
rd_buf(rd);
return true;
}
int rd_tok(struct rd_t *rd)
{
char ch, peek;
struct sym_t *sym;
for(;;) {
while(ch_space(rd->ch))
rd_ch(rd);
if(rd->ch != '#')
break;
do
rd_ch(rd);
while((rd->ch >= 0) && (rd->ch != '\n'));
}
rd_reset(rd);
rd->tloc = rd->loc;
ch = rd->ch;
peek = '\0';
for(sym = syms; sym->tok != 0; sym++) {
if(ch != sym->str[0])
continue;
if(peek == '\0')
peek = rd_ch(rd);
if(sym->str[1] == '\0')
return rd->tok = sym->tok;
else if(sym->str[1] != peek)
continue;
rd_ch(rd);
return rd->tok = sym->tok;
}
if(ch_str(rd->ch) || (rd->ch == '$') || (rd->ch == '"') || (rd->ch == '\'')) {
rd_str(rd);
rd_push(rd, '\0');
for(sym = keys; sym->tok != 0; sym++) {
if(strcmp(rd->str, sym->str) == 0)
return rd->tok = sym->tok;
}
return rd->tok = TOK_STR;
}
else if(rd->ch < 0)
return rd->tok = TOK_EOF;
else
fatal("FIXME boo %c", rd->ch);
return rd->tok;
}
struct ast_block_t *rd_block(struct rd_t *rd)
{
struct ast_block_t *block;
struct ast_stmt_t **stmt;
if(rd->tok != '{')
loc_err(rd->tloc, "Expected ';' or '{'.");
rd_tok(rd);
block = ast_block_new();
stmt = &block->stmt;
while(rd->tok != '}') {
*stmt = rd_stmt(rd);
stmt = &(*stmt)->next;
}
rd_tok(rd);
return block;
}
struct ast_block_t *rd_top(struct rd_t *rd)
{
struct ast_block_t *block;
struct ast_stmt_t **stmt;
block = ast_block_new();
stmt = &block->stmt;
while(rd->tok != TOK_EOF) {
*stmt = rd_stmt(rd);
stmt = &(*stmt)->next;
}
return block;
}
struct ast_stmt_t *rd_stmt(struct rd_t *rd)
{
struct loc_t loc;
enum stmt_e tag;
union stmt_u data;
loc = rd->loc;
if((rd->tok == TOK_STR) || (rd->tok == TOK_SPEC) || (rd->tok == TOK_VAR)) {
struct imm_t *lhs, *rhs;
lhs = rd_imm(rd);
if((rd->tok == '=') || (rd->tok == TOK_ADDEQ)) {
struct raw_t *id;
struct ast_bind_t *bind;
bool add = (rd->tok == TOK_ADDEQ);
if(imm_len(lhs) == 0)
loc_err(rd->tloc, "Missing variable name.");
else if(imm_len(lhs) >= 2)
loc_err(rd->tloc, "Invalid variable name.");
id = raw_dup(lhs->raw);
imm_delete(lhs);
rd_tok(rd);
if((rd->tok == TOK_STR) && (strcmp(rd->str, "env") == 0)) {
struct ast_block_t *block;
rd_tok(rd);
block = rd_block(rd);
bind = ast_bind_block(id, block, add);
}
else {
rhs = rd_imm(rd);
bind = ast_bind_val(id, rhs, add);
if(rd->tok != ';')
loc_err(rd->tloc, "Expected ';'.");
rd_tok(rd);
}
tag = ast_bind_v;
data.bind = bind;
}
else if(rd->tok == ':') {
loc = rd->tloc;
rd_tok(rd);
rhs = rd_imm(rd);
tag = syn_v;
data.syn = ast_rule_new(lhs, rhs, loc);
if(rd->tok == '{') {
data.syn->cmd = list_new((del_f)ast_cmd_delete);
rd_tok(rd);
while(rd->tok != '}') {
struct ast_cmd_t *proc;
struct ast_pipe_t *pipe, **ipipe;
pipe = NULL;
ipipe = &pipe;
*ipipe = ast_pipe_new(rd_imm(rd));
ipipe = &(*ipipe)->next;
while(rd->tok == '|') {
rd_tok(rd);
*ipipe = ast_pipe_new(rd_imm(rd));
ipipe = &(*ipipe)->next;
}
proc = ast_cmd_new(pipe);
list_add(data.syn->cmd, proc);
while(rd->tok != ';') {
if((rd->tok == '>') || (rd->tok == TOK_SHR)) {
if(proc->out != NULL)
loc_err(rd->tloc, "Output direct already given.");
if(rd->tok == TOK_SHR)
proc->append = true;
rd_tok(rd);
proc->out = rd_raw(rd);
if(proc->out == NULL)
loc_err(rd->tloc, "Missing output file path.");
}
else
loc_err(rd->tloc, "Expected ';'.");
}
rd_tok(rd);
}
rd_tok(rd);
}
else if(rd->tok == ';')
rd_tok(rd);
else
loc_err(rd->tloc, "Expected ';' or '{'.");
}
else
loc_err(rd->tloc, "Expected assignment or rule.");
}
else if(rd->tok == TOK_FOR) {
char *id;
struct imm_t *imm;
struct ast_stmt_t *body;
struct loc_t loc = rd->tloc;
if(rd_tok(rd) != TOK_STR)
loc_err(rd->tloc, "Expected variable name.");
id = strdup(rd->str);
if(rd_tok(rd) != ':')
loc_err(rd->tloc, "Expected ':'.");
rd_tok(rd);
imm = rd_imm(rd);
body = rd_stmt(rd);
tag = loop_v;
data.loop = loop_new(id, imm, body, loc);
}
else if(rd->tok == TOK_PRINT) {
tag = print_v;
rd_tok(rd);
data.print = print_new(rd_imm(rd));
if(rd->tok != ';')
loc_err(rd->tloc, "Expected ';'.");
rd_tok(rd);
}
else if(rd->tok == TOK_MKDEP) {
struct imm_t *imm;
rd_tok(rd);
imm = rd_imm(rd);
if(rd->tok != ';')
loc_err(rd->tloc, "Expected ';'.");
rd_tok(rd);
return ast_stmt_mkdep(ast_mkdep_new(imm, loc), loc);
}
else if((rd->tok == TOK_INCLUDE) || (rd->tok == TOK_IMPORT)) {
bool nest, opt;
struct imm_t *imm;
nest = (rd->tok == TOK_IMPORT);
rd_tok(rd);
if((opt = (rd->tok == '?')) == true) rd_tok(rd);
imm = rd_imm(rd);
if(rd->tok != ';')
loc_err(rd->tloc, "Expected ';'.");
rd_tok(rd);
return ast_stmt_inc(ast_inc_new(nest, opt, imm), loc);
}
else if(rd->tok == '{') {
tag = block_v;
data.block = rd_block(rd);
}
else
loc_err(rd->tloc, "Expected statement.", rd->tok, rd->tok);
return stmt_new(tag, data, loc);
}
struct imm_t *rd_imm(struct rd_t *rd)
{
struct imm_t *imm;
struct raw_t **raw;
imm = imm_new();
raw = &imm->raw;
while((*raw = rd_raw(rd)) != NULL)
raw = &(*raw)->next;
return imm;
}
struct raw_t *rd_raw(struct rd_t *rd)
{
struct raw_t *raw;
if((rd->tok != TOK_STR) && (rd->tok != TOK_SPEC) && (rd->tok != TOK_VAR))
return NULL;
raw = raw_new(rd->tok == TOK_SPEC, rd->tok == TOK_VAR, strdup(rd->str), rd->tloc);
rd_tok(rd);
return raw;
}
struct bind_t *bind_new(char *id, struct rt_obj_t obj, struct loc_t loc)
{
struct bind_t *bind;
bind = malloc(sizeof(struct bind_t));
*bind = (struct bind_t){ id, obj, loc, NULL };
return bind;
}
void bind_delete(struct bind_t *bind)
{
rt_obj_delete(bind->obj);
free(bind->id);
free(bind);
}
void bind_erase(struct bind_t *bind)
{
if(bind != NULL)
bind_delete(bind);
}
void bind_set(struct bind_t **dst, struct bind_t *src)
{
if(*dst != NULL)
bind_delete(*dst);
*dst = src;
}
void bind_reset(struct bind_t *bind, struct rt_obj_t obj, struct loc_t loc)
{
rt_obj_delete(bind->obj);
bind->obj = obj;
}
struct rt_obj_t rt_obj_new(enum rt_obj_e tag, union rt_obj_u data)
{
return (struct rt_obj_t){ tag, data };
}
struct rt_obj_t rt_obj_dup(struct rt_obj_t obj)
{
switch(obj.tag) {
case rt_null_v: return rt_obj_null(); break;
case rt_val_v: return rt_obj_val(val_dup(obj.data.val)); break;
case rt_env_v: return rt_obj_env(rt_env_dup(obj.data.env)); break;
case rt_func_v: return rt_obj_func(obj.data.func); break;
}
unreachable();
}
void rt_obj_delete(struct rt_obj_t obj)
{
switch(obj.tag) {
case rt_null_v: break;
case rt_val_v: val_clear(obj.data.val); break;
case rt_env_v: rt_env_clear(obj.data.env); break;
case rt_func_v: break;
}
}
void rt_obj_set(struct rt_obj_t *dst, struct rt_obj_t src)
{
rt_obj_delete(*dst);
*dst = src;
}
struct rt_obj_t rt_obj_null(void)
{
return rt_obj_new(rt_null_v, (union rt_obj_u){ });
}
struct rt_obj_t rt_obj_val(struct val_t *val)
{
return rt_obj_new(rt_val_v, (union rt_obj_u){ .val = val });
}
struct rt_obj_t rt_obj_env(struct env_t *env)
{
return rt_obj_new(rt_env_v, (union rt_obj_u){ .env = env });
}
struct rt_obj_t rt_obj_func(func_t *func)
{
return rt_obj_new(rt_func_v, (union rt_obj_u){ .func = func });
}
void rt_obj_add(struct rt_obj_t dst, struct rt_obj_t src, struct loc_t loc)
{
switch(dst.tag) {
case rt_null_v:
fatal("FIXME stub rt_obj_add null");
break;
case rt_val_v:
if(src.tag != dst.tag)
loc_err(loc, "Cannot add non-string value to a string value.");
*val_tail(&dst.data.val) = src.data.val;
break;
case rt_env_v:
if(src.tag != dst.tag)
loc_err(loc, "Cannot add non-environment value to an environment value.");
*rt_env_tail(&dst.data.env) = src.data.env;
break;
case rt_func_v:
loc_err(loc, "Cannot add function to variable.");
break;
}
}
struct val_t *val_new(bool spec, char *str)
{
struct val_t *val;
val = malloc(sizeof(struct val_t));
val->spec = spec;
val->str = str;
val->next = NULL;
return val;
}
struct val_t *val_dup(const struct val_t *val)
{
struct val_t *ret, **iter;
iter = &ret;
while(val != NULL) {
*iter = malloc(sizeof(struct val_t));
(*iter)->spec = val->spec;
(*iter)->str = strdup(val->str);
iter = &(*iter)->next;
val = val->next;
}
*iter = NULL;
return ret;
}
struct val_t *rt_val_copy(const struct val_t *val)
{
return val_new(val->spec, strdup(val->str));
}
void val_delete(struct val_t *val)
{
free(val->str);
free(val);
}
void val_clear(struct val_t *val)
{
struct val_t *tmp;
while(val != NULL) {
val = (tmp = val)->next;
val_delete(tmp);
}
}
char *val_id(struct val_t *val, struct loc_t loc)
{
char *id;
if((val == NULL) || (val_len(val) >= 2))
loc_err(loc, "Invalid variable name.");
id = val->str;
free(val);
return id;
}
char *val_str(struct val_t *val, struct loc_t loc)
{
char *id;
if((val == NULL) || (val_len(val) >= 2))
loc_err(loc, "Must be a single string.");
id = val->str;
free(val);
return id;
}
uint32_t val_len(struct val_t *val)
{
uint32_t n = 0;
while(val != NULL) {
n++;
val = val->next;
}
return n;
}
struct val_t **val_tail(struct val_t **val)
{
while(*val != NULL)
val = &(*val)->next;
return val;
}
struct val_t *rt_val_get(struct val_t *val, const char *str)
{
while(val != NULL) {
if(strcmp(val->str, str) == 0)
return val;
val = val->next;
}
return NULL;
}
struct env_t *rt_env_new(struct env_t *up)
{
struct env_t *env;
env = malloc(sizeof(struct env_t));
env->nrefs = 1;
env->map = map0_new((cmp_f)strcmp, (del_f)bind_delete);
env->next = up;
return env;
}
struct env_t *rt_env_dup(struct env_t *env)
{
struct env_t *iter;
for(iter = env; iter != NULL; iter = iter->next)
iter->nrefs++;
return env;
}
void rt_env_delete(struct env_t *env)
{
if(env->nrefs-- >= 2)
return;
map0_delete(env->map);
free(env);
}
void rt_env_clear(struct env_t *env)
{
struct env_t *tmp;
while(env != NULL) {
env = (tmp = env)->next;
rt_env_delete(tmp);
}
}
struct bind_t *rt_env_lookup(struct env_t *env, const char *id)
{
return map0_get(env->map, id);
}
struct bind_t *env_get(struct env_t *env, const char *id)
{
struct bind_t *bind;
while(env != NULL) {
bind = map0_get(env->map, id);
if(bind != NULL)
return bind;
env = env->next;
}
return NULL;
}
void env_put(struct env_t *env, struct bind_t *bind)
{
struct bind_t *cur;
cur = map_rem(env->map, bind->id);
if(cur != NULL)
bind_delete(cur);
map0_add(env->map, bind->id, bind);
}
struct env_t **rt_env_tail(struct env_t **env)
{
while(*env != NULL)
env = &(*env)->next;
return env;
}
char *cli_app = NULL;
void arr_init(const char ***arr, uint32_t *cnt) {
*arr = malloc(0);
*cnt = 0;
}
void arr_delete(const char **arr, uint32_t cnt) {
free(arr);
}
void arr_add(const char ***arr, uint32_t *cnt, const char *str)
{
*arr = realloc(*arr, (*cnt + 1) * sizeof(const char *));
(*arr)[(*cnt)++] = str;
}
void cli_proc(char **args)
{
struct env_t *env;
struct ast_block_t *top;
struct opt_t opt;
struct rt_ctx_t *ctx;
const char **arr;
uint32_t i, k, cnt;
opt.force = false;
opt.jobs = -1;
opt.dir = NULL;
env = rt_env_new(NULL);
arr_init(&arr, &cnt);
for(i = 0; args[i] != NULL; i++) {
if(args[i][0] == '-') {
if(args[i][1] == '-') {
fatal("FIXME stub");
}
else {
k = 1;
while(args[i][k] != '\0') {
bool end = false;
switch(args[i][k]) {
case 'B': opt.force = true; break;
case 'd':
if(opt.dir != NULL)
cli_err("Directory already given.");
else if(args[i][k + 1] == '\0') {
if((opt.dir = args[++i]) == NULL)
cli_err("Missing directory (-d).");
}
else
opt.dir = args[i] + k + 1;
end = true;
break;
case 'j': {
char *endptr;
const char *str;
unsigned long val;
if(opt.jobs >= 0)
cli_err("Jobs (-j) already given.");
else if(args[i][k + 1] == '\0') {
if((str = args[++i]) == NULL)
cli_err("Missing job count (-j).");
}
else
str = args[i] + k + 1;
errno = 0;
val = strtol(str, &endptr, 0);
if((errno != 0) || (*endptr != '\0') || (val == 0))
cli_err("Invalid job count (-j).");
opt.jobs = (val > 1024) ? 1024 : val;
k = strlen(args[i]);
end = true;
} break;
default: cli_err("Unknown option '-%c'.", args[i][k]); }
if(end)
break;
k++;
}
}
}
else {
char *key, *val, *find;
find = strchr(args[i], '=');
if(find != NULL) {
key = strndup(args[i], find - args[i]);
val = strdup(find + 1);
env_put(env, bind_new(key, rt_obj_val(val_new(false, val)), (struct loc_t){}));
}
else
arr_add(&arr, &cnt, args[i]);
}
}
arr_add(&arr, &cnt, NULL);
top = ham_load("Hammer");
if(top == NULL)
cli_err("Cannot open '%s'.", "Hammer");
ctx = ctx_new(&opt);
rt_eval_top(top, ctx, env);
ctx_run(ctx, arr);
rt_env_delete(env);
ast_block_delete(top);
ctx_delete(ctx);
arr_delete(arr, cnt);
}
void cli_err(const char *fmt, ...)
{
va_list args;
va_start(args, fmt);
fprintf(stderr, "%s: ", cli_app);
vfprintf(stderr, fmt, args);
fprintf(stderr, "\n");
exit(1);
}
struct seq_t *seq_new(void)
{
struct seq_t *seq;
seq = malloc(sizeof(struct seq_t));
*seq = (struct seq_t){ NULL, &seq->head };
return seq;
}
void seq_delete(struct seq_t *seq)
{
struct cmd_t *cmd, *tmp;
cmd = seq->head;
while(cmd != NULL) {
cmd = (tmp = cmd)->next;
rt_pipe_clear(tmp->pipe);
if(tmp->in != NULL)
free(tmp->in);
if(tmp->out != NULL)
free(tmp->out);
free(tmp);
}
free(seq);
}
void seq_add(struct seq_t *seq, struct rt_pipe_t *pipe, char *in, char *out, bool append)
{
struct cmd_t *cmd;
cmd = malloc(sizeof(struct cmd_t));
*cmd = (struct cmd_t){ pipe, in, out, append, NULL };
*seq->tail = cmd;
seq->tail = &cmd->next;
}
struct rt_pipe_t *rt_pipe_new(struct val_t *cmd)
{
struct rt_pipe_t *pipe;
pipe = malloc(sizeof(struct rt_pipe_t));
pipe->cmd = cmd;
pipe->next = NULL;
return pipe;
}
void rt_pipe_clear(struct rt_pipe_t *pipe)
{
struct rt_pipe_t *tmp;
while(pipe != NULL) {
pipe = (tmp = pipe)->next;
val_clear(tmp->cmd);
free(tmp);
}
}
struct rt_ctx_t *ctx_new(const struct opt_t *opt)
{
struct rt_ctx_t *ctx;
ctx = malloc(sizeof(struct rt_ctx_t));
ctx->opt = opt;
ctx->map = map_new();
ctx->rules = rule_list_new();
ctx->cur = NULL;
ctx->gens = ctx->deps = NULL;
return ctx;
}
void ctx_delete(struct rt_ctx_t *ctx)
{
map_delete(ctx->map);
rule_list_delete(ctx->rules);
free(ctx);
}
void ctx_run(struct rt_ctx_t *ctx, const char **builds)
{
struct ctrl_t *ctrl;
struct rule_t *rule;
struct rule_iter_t irule;
struct queue_t *queue;
queue = queue_new();
ctrl = ctrl_new(queue, 4);
irule = rule_iter(ctx->rules);
while((rule = rule_next(&irule)) != NULL) {
uint32_t i;
struct target_t *target;
struct target_iter_t iter;
iter = target_iter(rule->gens);
while((target = target_next(&iter)) != NULL) {
for(i = 0; builds[i] != NULL; i++) {
if(strcmp(target->path, builds[i]) == 0)
queue_recur(queue, rule);
}
if((target->flags & (FLAG_BUILD | FLAG_SPEC)) == 0)
target->flags |= FLAG_BUILD;
}
}
for(;;) {
while(!ctrl_avail(ctrl))
ctrl_wait(ctrl);
rule = queue_rem(queue);
if(rule == NULL) {
if(!ctrl_busy(ctrl))
break;
ctrl_wait(ctrl);
continue;
}
struct target_t *target;
struct target_iter_t iter;
int64_t min = INT64_MAX - 1, max = INT64_MIN + 1;
iter = target_iter(rule->gens);
while((target = target_next(&iter)) != NULL) {
if(target->flags & FLAG_SPEC)
min = INT64_MIN, max = INT64_MAX;
if(target_mtime(target) < min)
min = target_mtime(target);
}
iter = target_iter(rule->deps);
while((target = target_next(&iter)) != NULL) {
if(target->flags & FLAG_SPEC)
continue;
if(target_mtime(target) > max)
max = target_mtime(target);
}
iter = target_iter(rule->gens);
if((max > min) || ctx->opt->force) {
iter = target_iter(rule->gens);
while((target = target_next(&iter)) != NULL) {
char *path, *iter;
if(target->flags & FLAG_SPEC)
continue;
path = strdup(target->path);
iter = path;
while((iter = strchr(iter, '/')) != NULL) {
*iter = '\0';
os_mkdir(path);
*iter = '/';
iter++;
}
free(path);
}
ctrl_add(ctrl, rule);
}
else
ctrl_done(ctrl, rule);
}
while(ctrl_busy(ctrl))
ctrl_wait(ctrl);
queue_delete(queue);
ctrl_delete(ctrl);
}
struct target_t *ctx_target(struct rt_ctx_t *ctx, bool spec, const char *path)
{
struct target_t *target;
target = map_get(ctx->map, spec, path);
if(target == NULL) {
target = rt_ref_new(spec, strdup(path));
map_add(ctx->map, target);
}
return target;
}
struct rule_t *ctx_rule(struct rt_ctx_t *ctx, const char *id, struct target_list_t *gens, struct target_list_t *deps)
{
struct rule_t *rule;
if(gens->inst == NULL)
fatal("FIXME location All rules must have at least one targets.");
if(id != NULL) {
fatal("FIXME stub rule w/ id");
}
else {
struct target_t *target;
struct target_inst_t **inst;
struct target_iter_t iter;
iter = target_iter(gens);
rule = target_next(&iter)->rule;
if(rule != NULL) {
uint32_t len;
if(rule->seq != NULL)
fatal("Cannot add targets to a rule with a recipe.");
len = target_list_len(gens);
if(target_list_len(rule->gens) != len)
fatal("Partial rules must have matching target lists.");
iter = target_iter(gens);
while((target = target_next(&iter)) != NULL) {
if(!target_list_contains(rule->gens, target))
fatal("Partial rules must have matching target lists.");
}
inst = &rule->deps->inst;
while(*inst != NULL)
inst = &(*inst)->next;
*inst = deps->inst;
deps->inst = NULL;
iter = target_iter(deps);
while((target = target_next(&iter)) != NULL)
target_conn(target, rule);
target_list_delete(gens);
target_list_delete(deps);
}
else {
rule = rule_new(id ? strdup(id) : NULL, gens, deps, NULL);
rule_list_add(ctx->rules, rule);
iter = target_iter(gens);
while((target = target_next(&iter)) != NULL) {
if(target->rule != NULL)
fatal("FIXME target already had rule, better error");
target->rule = rule;
}
iter = target_iter(deps);
while((target = target_next(&iter)) != NULL)
target_conn(target, rule);
}
}
return rule;
}
struct val_t *func_sub(struct rt_ctx_t *ctx, struct val_t **args, uint32_t cnt)
{
return NULL;
}
struct rt_obj_t rt_fun_uniq(struct rt_obj_t *args, uint32_t cnt, struct loc_t loc)
{
struct val_t *val, *ret, **iter;
if(cnt != 1)
loc_err(loc, "Function `.uniq` requires no arguments.");
ret = NULL;
iter = &ret;
for(val = args[0].data.val; val != NULL; val = val->next) {
if(rt_val_get(ret, val->str) == NULL) {
*iter = rt_val_copy(val);
iter = &(*iter)->next;
}
}
return rt_obj_val(ret);
}
void rt_eval_top(struct ast_block_t *block, struct rt_ctx_t *ctx, struct env_t *env)
{
struct env_t *nest;
struct ast_stmt_t *stmt;
nest = rt_env_new(env);
env_put(nest, bind_new(strdup(".sub"), rt_obj_func(fn_sub), (struct loc_t){ }));
env_put(nest, bind_new(strdup(".pat"), rt_obj_func(fn_pat), (struct loc_t){ }));
env_put(nest, bind_new(strdup(".uniq"), rt_obj_func(rt_fun_uniq), (struct loc_t){ }));
for(stmt = block->stmt; stmt != NULL; stmt = stmt->next)
eval_stmt(stmt, ctx, nest);
rt_env_delete(nest);
}
void eval_block(struct ast_block_t *block, struct rt_ctx_t *ctx, struct env_t *env)
{
struct ast_stmt_t *stmt;
for(stmt = block->stmt; stmt != NULL; stmt = stmt->next)
eval_stmt(stmt, ctx, env);
}
void eval_stmt(struct ast_stmt_t *stmt, struct rt_ctx_t *ctx, struct env_t *env)
{
switch(stmt->tag) {
case ast_bind_v: {
char *id;
struct bind_t *get = NULL;
struct rt_obj_t obj;
struct ast_bind_t *bind = stmt->data.bind;
id = rt_eval_str(bind->id, ctx, env, stmt->loc);
switch(bind->tag) {
case ast_val_v:
obj = eval_imm(bind->data.val, ctx, env, stmt->loc);
break;
case ast_func_v:
fatal("FIXME stub");
break;
case ast_block_v: {
struct env_t *nest;
nest = rt_env_new(env);
eval_block(bind->data.block, ctx, nest);
nest->next = NULL;
obj = rt_obj_env(nest);
} break;
}
get = rt_env_lookup(env, id);
if(get != NULL) {
if(bind->add)
rt_obj_add(get->obj, obj, stmt->loc);
else
rt_obj_set(&get->obj, obj);
free(id);
}
else
env_put(env, bind_new(id, obj, stmt->loc));
} break;
case syn_v: {
struct ast_rule_t *syn = stmt->data.syn;
struct target_list_t *gens, *deps;
struct val_t *gen, *dep, *iter;
struct link_t *link;
struct ast_cmd_t *proc;
struct rule_t *rule;
gens = target_list_new();
deps = target_list_new();
gen = rt_eval_val(syn->gen, ctx, env, stmt->loc);
for(iter = gen; iter != NULL; iter = iter->next)
target_list_add(gens, ctx_target(ctx, iter->spec, iter->str));
dep = rt_eval_val(syn->dep, ctx, env, stmt->loc);
for(iter = dep; iter != NULL; iter = iter->next)
target_list_add(deps, ctx_target(ctx, iter->spec, iter->str));
rule = ctx_rule(ctx, NULL, gens, deps);
val_clear(gen);
val_clear(dep);
if(syn->cmd != NULL) {
rule->seq = seq_new();
ctx->cur = rule;
for(link = syn->cmd->head; link != NULL; link = link->next) {
char *out, *in;
struct ast_pipe_t *iter;
struct rt_pipe_t *pipe = NULL, **ipipe = &pipe;
proc = link->val;
in = proc->in ? rt_eval_str(proc->in, ctx, env, syn->loc) : NULL;
out = proc->out ? rt_eval_str(proc->out, ctx, env, syn->loc) : NULL;
for(iter = proc->pipe; iter != NULL; iter = iter->next) {
*ipipe = rt_pipe_new(rt_eval_val(iter->imm, ctx, env, stmt->loc));
ipipe = &(*ipipe)->next;
}
seq_add(rule->seq, pipe, in, out, proc->append);
}
}
} break;
case loop_v: {
struct env_t *nest;
struct rt_obj_t obj;
struct loop_t *loop = stmt->data.loop;
obj = eval_imm(loop->imm, ctx, env, stmt->loc);
switch(obj.tag) {
case rt_val_v: {
struct val_t *iter;
for(iter = obj.data.val; iter != NULL; iter = iter->next) {
nest = rt_env_new(env);
env_put(nest, bind_new(strdup(loop->id), rt_obj_val(val_new(iter->spec, strdup(iter->str))), stmt->loc));
eval_stmt(loop->body, ctx, nest);
rt_env_delete(nest);
}
} break;
case rt_env_v: {
struct env_t *iter;
for(iter = obj.data.env; iter != NULL; iter = iter->next) {
nest = rt_env_new(env);
env_put(nest, bind_new(strdup(loop->id), rt_obj_env(rt_env_dup(iter)), stmt->loc));
eval_stmt(loop->body, ctx, nest);
rt_env_delete(nest);
}
} break;
default:
loc_err(stmt->loc, "Can only iterate over strings and environments.");
}
rt_obj_delete(obj);
} break;
case print_v: {
struct val_t *val, *iter;
val = rt_eval_val(stmt->data.print->imm, ctx, env, stmt->loc);
for(iter = val; iter != NULL; iter = iter->next)
print("%s%s", iter->str, iter->next ? " " : "");
val_clear(val);
} break;
case ast_mkdep_v:
ast_mkdep_eval(stmt->data.mkdep, ctx, env);
break;
case block_v: {
struct env_t *nest;
nest = rt_env_new(env);
eval_block(stmt->data.block, ctx, nest);
rt_env_delete(nest);
} break;
case ast_inc_v:
ast_inc_eval(stmt->data.inc, ctx, env, stmt->loc);
break;
}
}
struct rt_obj_t eval_imm(struct imm_t *imm, struct rt_ctx_t *ctx, struct env_t *env, struct loc_t loc)
{
struct raw_t *raw;
struct rt_obj_t obj;
if(imm->raw == NULL)
return rt_obj_null();
obj = eval_raw(imm->raw, ctx, env);
for(raw = imm->raw->next; raw != NULL; raw = raw->next)
rt_obj_add(obj, eval_raw(raw, ctx, env), loc);
return obj;
}
struct exp_t {
struct buf_t buf;
const char *orig, *str;
struct rt_ctx_t *ctx;
struct env_t *env;
struct loc_t loc;
};
struct rt_obj_t exp_get(struct exp_t *exp);
char exp_ch(struct exp_t *exp);
char exp_adv(struct exp_t *exp);
char exp_buf(struct exp_t *exp);
char exp_trim(struct exp_t *exp);
void exp_str(struct exp_t *exp);
void exp_escape(struct exp_t *exp);
void exp_quote1(struct exp_t *exp);
void exp_quote2(struct exp_t *exp);
struct rt_obj_t exp_var(struct exp_t *exp);
struct rt_obj_t exp_bind(struct exp_t *exp);
void exp_flat(struct exp_t *exp, struct rt_obj_t obj);
struct loc_t exp_loc(struct exp_t *exp);
__attribute__((noreturn)) void exp_err(struct exp_t *exp, const char *fmt, ...);
struct rt_obj_t exp_get(struct exp_t *exp)
{
struct rt_obj_t obj;
struct buf_t tmp;
tmp = exp->buf;
exp->buf = buf_new(32);
if(*exp->str == '$') {
obj = exp_var(exp);
if(*exp->str != '\0') {
exp_flat(exp, obj);
exp_str(exp);
obj = rt_obj_val(val_new(false, strdup(buf_done(&exp->buf))));
}
}
else if(*exp->str == '.') {
exp_buf(exp);
while(ch_var(exp_ch(exp)))
exp_buf(exp);
if(*exp->str != '\0') {
exp_str(exp);
obj = rt_obj_val(val_new(false, strdup(buf_done(&exp->buf))));
}
else
obj = rt_obj_val(val_new(true, strdup(buf_done(&exp->buf))));
}
else {
exp_str(exp);
obj = rt_obj_val(val_new(false, strdup(buf_done(&exp->buf))));
}
buf_delete(&exp->buf);
exp->buf = tmp;
return obj;
}
char exp_ch(struct exp_t *exp)
{
return *exp->str;
}
char exp_adv(struct exp_t *exp)
{
exp->str++;
return *exp->str;
}
char exp_buf(struct exp_t *exp)
{
buf_ch(&exp->buf, *exp->str);
return exp_adv(exp);
}
char exp_trim(struct exp_t *exp)
{
const char *str;
str = exp->str;
while((*str == ' ') || (*str == '\t') || (*str == '\n'))
str++;
exp->str = str;
return *str;
}
void exp_str(struct exp_t *exp)
{
char ch;
for(;;) {
ch = exp_ch(exp);
if(ch == '\0')
break;
else if(ch == '\\')
exp_escape(exp);
else if(ch == '\'')
exp_quote1(exp);
else if(ch == '"')
exp_quote2(exp);
else if(ch == '$')
exp_flat(exp, exp_var(exp));
else if(ch_str(ch))
exp_buf(exp);
else
break;
}
}
void exp_escape(struct exp_t *exp)
{
char ch;
switch(exp_adv(exp)) {
case '\\': ch = '\\'; break;
case '\'': ch = '\''; break;
case '\"': ch = '\"'; break;
case 't': ch = '\t'; break;
case 'n': ch = '\n'; break;
case '$': ch = '$'; break;
case ' ': ch = ' '; break;
case ',': ch = ','; break;
default: exp_err(exp, "Invalid escape sequence '\\%c'.", exp_ch(exp));
}
buf_ch(&exp->buf, ch);
exp_adv(exp);
}
void exp_quote1(struct exp_t *exp)
{
char ch;
exp_adv(exp);
for(;;) {
ch = exp_ch(exp);
if(ch == '\'')
break;
else if(ch == '\\')
exp_escape(exp);
else
exp_buf(exp);
}
exp_adv(exp);
}
void exp_quote2(struct exp_t *exp)
{
char ch;
exp_adv(exp);
for(;;) {
ch = exp_ch(exp);
if(ch == '"')
break;
else if(ch == '\\')
exp_escape(exp);
else if(ch == '$')
exp_flat(exp, exp_var(exp));
else
exp_buf(exp);
}
exp_adv(exp);
}
struct rt_obj_t exp_var(struct exp_t *exp)
{
char ch;
struct rt_obj_t obj;
ch = exp_adv(exp);
if(ch == '{') {
exp_adv(exp);
obj = exp_bind(exp);
while((ch = exp_trim(exp)) != '}') {
const char *id;
struct buf_t buf;
if(ch != '.')
exp_err(exp, "Expected '.' or '}'.");
buf = buf_new(32);
buf_ch(&buf, ch);
exp_adv(exp);
ch = exp_trim(exp);
if(!ch_var(ch))
exp_err(exp, "Expected function/member name.");
do
buf_ch(&buf, ch);
while(ch_var(ch = exp_adv(exp)));
id = buf_done(&buf);
switch(obj.tag) {
case rt_null_v:
fatal("FIXME exp_var null call");
case rt_val_v: {
uint32_t cnt;
struct bind_t *bind;
struct loc_t loc;
struct rt_obj_t *args;
bind = env_get(exp->env, id);
if(bind == NULL)
exp_err(exp, "Unknown function '%s'.", id);
else if(bind->obj.tag != rt_func_v)
exp_err(exp, "Variable '%s' is not a function.", id);
loc = exp_loc(exp);
args_init(&args, &cnt);
args_add(&args, &cnt, obj);
if(exp_trim(exp) != '(')
exp_err(exp, "Expected '('.");
exp_adv(exp);
if(exp_trim(exp) != ')') {
for(;;) {
args_add(&args, &cnt, exp_get(exp));
ch = exp_trim(exp);
if(ch == ')')
break;
else if(ch != ',')
exp_err(exp, "Expected ',' or '('.");
exp_adv(exp);
exp_trim(exp);
}
}
exp_adv(exp);
switch(bind->obj.tag) {
case rt_func_v:
obj = bind->obj.data.func(args, cnt, loc);
break;
default:
unreachable();
}
args_delete(args, cnt);
} break;
case rt_env_v: {
struct bind_t *bind;
bind = env_get(obj.data.env, id + 1);
if(bind == NULL)
exp_err(exp, "Unknown member '%s'.", id + 1);
rt_obj_delete(obj);
obj = rt_obj_dup(bind->obj);
} break;
case rt_func_v:
fatal("FIXME exp_var func call");
}
buf_delete(&buf);
}
exp_adv(exp);
return obj;
}
else
return exp_bind(exp);
}
void exp_flat(struct exp_t *exp, struct rt_obj_t obj)
{
struct val_t *val, *orig;
if(obj.tag != rt_val_v)
exp_err(exp, "Cannot convert non-value to a string.");
val = orig = obj.data.val;
while(val != NULL) {
buf_str(&exp->buf, val->str);
if(val->next != NULL)
buf_ch(&exp->buf, ' ');
val = val->next;
}
val_clear(orig);
}
struct rt_obj_t exp_bind(struct exp_t *exp)
{
char *id;
const char *str;
struct buf_t buf;
struct bind_t *bind;
str = exp->str;
if(*str == '@') {
struct target_inst_t *inst;
struct val_t *val = NULL, **iter = &val;
if(exp->ctx->cur == NULL)
exp_err(exp, "Variable '$@' can only be used in recipes.");
for(inst = exp->ctx->cur->gens->inst; inst != NULL; inst = inst->next) {
*iter = val_new(false, strdup(inst->target->path));
iter = &(*iter)->next;
}
exp_adv(exp);
return rt_obj_val(val);
}
else if(*str == '^') {
struct target_inst_t *inst;
struct val_t *val = NULL, **iter = &val;
if(exp->ctx->cur == NULL)
exp_err(exp, "Variable '$^' can only be used in recipes.");
for(inst = exp->ctx->cur->deps->inst; inst != NULL; inst = inst->next) {
*iter = val_new(false, strdup(inst->target->path));
iter = &(*iter)->next;
}
exp_adv(exp);
return rt_obj_val(val);
}
else if(*str == '<') {
struct target_inst_t *inst;
if(exp->ctx->cur == NULL)
exp_err(exp, "Variable '$<' can only be used in recipes.");
inst = exp->ctx->cur->deps->inst;
if(inst == NULL)
return rt_obj_null();
exp_adv(exp);
return rt_obj_val(val_new(inst->target->flags & FLAG_SPEC, strdup(inst->target->path)));
}
else if(*str == '*') {
struct val_t *val, **iter;
struct rule_inst_t *inst;
val = NULL;
iter = &val;
for(inst = exp->ctx->rules->inst; inst != NULL; inst = inst->next) {
struct target_inst_t *ref;
for(ref = inst->rule->gens->inst; ref != NULL; ref = ref->next) {
if(ref->target->flags & FLAG_SPEC)
continue;
*iter = val_new(false, strdup(ref->target->path));
iter = &(*iter)->next;
}
}
*iter = NULL;
exp_adv(exp);
return rt_obj_val(val);
}
buf = buf_new(32);
do
buf_ch(&buf, *str++);
while(ch_var(*str));
id = buf_done(&buf);
bind = env_get(exp->env, id);
if(bind == NULL)
exp_err(exp, "Unknown variable '%s'.", id);
exp->str = str;
free(id);
return rt_obj_dup(bind->obj);
}
struct loc_t exp_loc(struct exp_t *exp)
{
return loc_off(exp->loc, exp->str - exp->orig);
}
void exp_err(struct exp_t *exp, const char *fmt, ...)
{
va_list args;
va_start(args, fmt);
fprintf(stderr, "%s:%u:%lu: ", exp->loc.path, exp->loc.lin, exp->loc.col + (exp->str - exp->orig));
vfprintf(stderr, fmt, args);
fprintf(stderr, "\n");
va_end(args);
exit(1);
}
struct rt_obj_t eval_raw(struct raw_t *raw, struct rt_ctx_t *ctx, struct env_t *env)
{
struct exp_t exp;
exp.orig = exp.str = raw->str;
exp.loc = raw->loc;
exp.ctx = ctx;
exp.env = env;
return exp_get(&exp);
}
struct val_t *rt_eval_val(struct imm_t *imm, struct rt_ctx_t *ctx, struct env_t *env, struct loc_t loc)
{
struct rt_obj_t obj;
obj = eval_imm(imm, ctx, env, loc);
if(obj.tag == rt_null_v)
return NULL;
else if(obj.tag != rt_val_v)
loc_err(loc, "String values required.");
return obj.data.val;
}
char *rt_eval_str(struct raw_t *raw, struct rt_ctx_t *ctx, struct env_t *env, struct loc_t loc)
{
char *str;
struct val_t *val;
struct rt_obj_t obj;
obj = eval_raw(raw, ctx, env);
if(obj.tag != rt_val_v)
loc_err(loc, "String required.");
val = obj.data.val;
if((val == NULL) || (val->next != NULL))
loc_err(loc, "String required.");
str = val->str;
free(val);
return str;
}
void args_init(struct rt_obj_t **args, uint32_t *cnt)
{
*args = malloc(0);
*cnt = 0;
}
void args_add(struct rt_obj_t **args, uint32_t *cnt, struct rt_obj_t obj)
{
*args = realloc(*args, (*cnt + 1) * sizeof(struct rt_obj_t));
(*args)[(*cnt)++] = obj;
}
void args_delete(struct rt_obj_t *args, uint32_t cnt)
{
uint32_t i;
for(i = 0; i < cnt; i++)
rt_obj_delete(args[i]);
free(args);
}
struct rt_obj_t fn_sub(struct rt_obj_t *args, uint32_t cnt, struct loc_t loc)
{
struct buf_t buf;
struct val_t *val, *ret, **iter;
const char *get, *put, *str, *find;
if(cnt != 3)
loc_err(loc, "Function `.sub` requires 2 arguments.");
else if((args[0].tag != rt_val_v) || (args[1].tag != rt_val_v) || (args[2].tag != rt_val_v))
loc_err(loc, "Function `.sub` requires string values as arguments.");
else if((val_len(args[1].data.val) != 1) || (val_len(args[2].data.val) != 1))
loc_err(loc, "Function `.sub` requires string values as arguments.");
iter = &ret;
get = args[1].data.val->str;
put = args[2].data.val->str;
for(val = args[0].data.val; val != NULL; val = val->next) {
str = val->str;
buf = buf_new(strlen(val->str) + 1);
find = strstr(str, get);
while(find != NULL) {
buf_mem(&buf, str, find - str);
buf_str(&buf, put);
str = find + strlen(get);
find = strstr(str, get);
}
buf_str(&buf, str);
*iter = val_new(val->spec, buf_done(&buf));
iter = &(*iter)->next;
}
*iter = NULL;
return rt_obj_val(ret);
}
bool pat_len(const char *str, uint32_t *pre, uint32_t *post)
{
const char *find;
find = strchr(str, '%');
if((find == NULL) || (strrchr(str, '%') != find))
return false;
*pre = find - str;
*post = strlen(str) - *pre - 1;
return true;
}
struct rt_obj_t fn_pat(struct rt_obj_t *args, uint32_t cnt, struct loc_t loc)
{
uint32_t len, min, spre, spost, slen, rpre, rpost, rlen;
struct buf_t buf;
struct val_t *val, *ret, **iter;
char *pat, *repl;
if(cnt != 3)
loc_err(loc, "Function `.pat` requires 2 arguments.");
else if((args[0].tag != rt_val_v) || (args[1].tag != rt_val_v) || (args[2].tag != rt_val_v))
loc_err(loc, "Function `.pat` requires string values as arguments.");
else if((val_len(args[1].data.val) != 1) || (val_len(args[2].data.val) != 1))
loc_err(loc, "Function `.pat` requires string values as arguments.");
pat = args[1].data.val->str;
repl = args[2].data.val->str;
slen = strlen(pat);
rlen = strlen(repl);
if(!pat_len(pat, &spre, &spost) || !pat_len(repl, &rpre, &rpost))
loc_err(loc, "Function `.pat` requires patterns as arguments (must contain a single '%').");
iter = &ret;
min = spre + spost;
for(val = args[0].data.val; val != NULL; val = val->next) {
len = strlen(val->str);
if((len > min) && (memcmp(val->str, pat, spre) == 0) && (strcmp(val->str + len - spost, pat + slen - spost) == 0)) {
buf = buf_new(32);
buf_mem(&buf, repl, rpre);
buf_mem(&buf, val->str + spre, len - spost - spre);
buf_str(&buf, repl + rlen - rpost);
*iter = val_new(val->spec, buf_done(&buf));
}
else
*iter = val_new(val->spec, strdup(val->str));
iter = &(*iter)->next;
}
*iter = NULL;
return rt_obj_val(ret);
}
struct ctrl_t *ctrl_new(struct queue_t *queue, uint32_t n)
{
uint32_t i;
struct ctrl_t *ctrl;
ctrl = malloc(sizeof(struct ctrl_t));
ctrl->queue = queue;
ctrl->cnt = n;
ctrl->job = malloc(n * sizeof(struct job_t));
for(i = 0; i < n; i++)
ctrl->job[i].pid = -1;
return ctrl;
}
void ctrl_delete(struct ctrl_t *ctrl)
{
free(ctrl->job);
free(ctrl);
}
void ctrl_add(struct ctrl_t *ctrl, struct rule_t *rule)
{
uint32_t i;
if((rule->seq == NULL) || (rule->seq->head == NULL))
return ctrl_done(ctrl, rule);
for(i = 0; i < ctrl->cnt; i++) {
if(ctrl->job[i].pid < 0)
break;
}
if(i >= ctrl->cnt)
fatal("Failed to start job.");
ctrl->job[i].pid = ctrl_exec(rule->seq->head);
ctrl->job[i].rule = rule;
ctrl->job[i].cmd = rule->seq->head->next;
}
bool ctrl_avail(struct ctrl_t *ctrl)
{
uint32_t i;
for(i = 0; i < ctrl->cnt; i++) {
if(ctrl->job[i].pid < 0)
return true;
}
return false;
}
bool ctrl_busy(struct ctrl_t *ctrl)
{
uint32_t i;
for(i = 0; i < ctrl->cnt; i++) {
if(ctrl->job[i].pid >= 0)
return true;
}
return false;
}
void ctrl_wait(struct ctrl_t *ctrl)
{
uint32_t i;
int pid;
pid = os_wait();
for(i = 0; i < ctrl->cnt; i++) {
if(ctrl->job[i].pid == pid)
break;
}
if(i >= ctrl->cnt)
return;
if(ctrl->job[i].cmd != NULL) {
ctrl->job[i].pid = ctrl_exec(ctrl->job[i].cmd);
ctrl->job[i].cmd = ctrl->job[i].cmd->next;
}
else {
ctrl->job[i].pid = -1;
ctrl_done(ctrl, ctrl->job[i].rule);
}
}
void ctrl_done(struct ctrl_t *ctrl, struct rule_t *rule)
{
struct edge_t *edge;
struct target_t *target;
struct target_iter_t iter;
iter = target_iter(rule->gens);
while((target = target_next(&iter)) != NULL) {
target->mtime = -1;
for(edge = target->edge; edge != NULL; edge = edge->next) {
edge->rule->edges--;
if(edge->rule->edges == 0)
queue_add(ctrl->queue, edge->rule);
}
}
}
int ctrl_exec(struct cmd_t *cmd)
{
struct val_t *val;
struct rt_pipe_t *pipe;
for(pipe = cmd->pipe; pipe != NULL; pipe = pipe->next) {
for(val = pipe->cmd; val != NULL; val = val->next)
print("%s%s", val->str, (val->next != NULL) ? " " : "");
if(pipe->next != NULL)
print(" | ");
}
if(cmd->in != NULL)
print(" < %s", cmd->in);
if(cmd->out != NULL)
print(" >%s %s", cmd->append ? ">" : "", cmd->out);
print("\n");
return os_exec(cmd);
}
struct list_t *list_new(void(*del)(void *))
{
struct list_t *list;
list = malloc(sizeof(struct list_t));
list->head = NULL;
list->tail = &list->head;
list->del = del;
return list;
}
void list_delete(struct list_t *list)
{
struct link_t *link, *tmp;
link = list->head;
while(link != NULL) {
link = (tmp = link)->next;
if(list->del != NULL)
list->del(tmp->val);
free(tmp);
}
free(list);
}
void list_add(struct list_t *list, void *val)
{
struct link_t *link;
link = malloc(sizeof(struct link_t));
link->val = val;
link->next = NULL;
*list->tail = link;
list->tail = &link->next;
}
struct ns_t *ns_new(char *id, struct ns_t *up)
{
struct ns_t *ns;
ns = malloc(sizeof(struct ns_t));
ns->id = id;
ns->up = up;
ns->next = NULL;
ns->bind = NULL;
return ns;
}
void ns_delete(struct ns_t *ns)
{
struct bind_t *bind;
while(ns->bind != NULL) {
ns->bind = (bind = ns->bind)->next;
bind_delete(bind);
}
if(ns->id != NULL)
free(ns->id);
free(ns);
}
void ns_add(struct ns_t *ns, struct bind_t *bind)
{
if(bind->id == NULL) {
bind->next = ns->bind;
ns->bind = bind;
}
else {
cli_err("FIXME STUB SLKFJSDLFJ");
}
}
struct bind_t *ns_find(struct ns_t *ns, const char *id)
{
return *ns_lookup(ns, id);
}
struct bind_t **ns_lookup(struct ns_t *ns, const char *id)
{
struct bind_t **bind;
bind = &ns->bind;
while(*bind != NULL) {
if(strcmp((*bind)->id, id) == 0)
break;
bind = &(*bind)->next;
}
return bind;
}
struct rule_t *rule_new(char *id, struct target_list_t *gens, struct target_list_t *deps, struct seq_t *seq)
{
struct rule_t *rule;
rule = malloc(sizeof(struct rule_t));
*rule = (struct rule_t){ id, gens, deps, seq, false, 0 };
return rule;
}
void rule_delete(struct rule_t *rule)
{
if(rule->seq != NULL)
seq_delete(rule->seq);
target_list_delete(rule->gens);
target_list_delete(rule->deps);
free(rule);
}
struct rule_iter_t rule_iter(struct rule_list_t *list)
{
return (struct rule_iter_t){ list->inst };
}
struct rule_t *rule_next(struct rule_iter_t *iter)
{
struct rule_t *rule;
if(iter->inst == NULL)
return NULL;
rule = iter->inst->rule;
iter->inst = iter->inst->next;
return rule;
}
struct rule_list_t *rule_list_new(void)
{
struct rule_list_t *list;
list = malloc(sizeof(struct rule_list_t));
*list = (struct rule_list_t){ NULL };
return list;
}
void rule_list_delete(struct rule_list_t *list)
{
struct rule_inst_t *inst, *tmp;
inst = list->inst;
while(inst != NULL) {
inst = (tmp = inst)->next;
rule_delete(tmp->rule);
free(tmp);
}
free(list);
}
void rule_list_add(struct rule_list_t *list, struct rule_t *rule)
{
struct rule_inst_t *inst;
inst = malloc(sizeof(struct rule_inst_t));
inst->rule = rule;
inst->next = list->inst;
list->inst = inst;
}
struct queue_t {
struct item_t *head, **tail;
};
struct item_t {
struct rule_t *rule;
struct item_t *next;
};
struct queue_t *queue_new(void)
{
struct queue_t *queue;
queue = malloc(sizeof(struct queue_t));
*queue = (struct queue_t){ NULL, &queue->head };
return queue;
}
void queue_delete(struct queue_t *queue)
{
free(queue);
}
void queue_recur(struct queue_t *queue, struct rule_t *rule)
{
uint32_t cnt = 0;
struct target_t *target;
struct target_iter_t iter;
if(rule->add)
return;
rule->add = true;
iter = target_iter(rule->deps);
while((target = target_next(&iter)) != NULL) {
if(target->rule != NULL) {
cnt++;
queue_recur(queue, target->rule);
}
}
if(cnt == 0)
queue_add(queue, rule);
else
rule->edges = cnt;
}
void queue_add(struct queue_t *queue, struct rule_t *rule)
{
struct item_t *item;
item = malloc(sizeof(struct item_t));
item->rule = rule;
item->next = NULL;
*queue->tail = item;
queue->tail = &item->next;
}
struct rule_t *queue_rem(struct queue_t *queue)
{
struct item_t *item;
struct rule_t *rule;
item = queue->head;
if(item == NULL)
return NULL;
queue->head = item->next;
if(queue->head == NULL)
queue->tail = &queue->head;
rule = item->rule;
free(item);
return rule;
}
char *get_str(const char **str, struct loc_t loc)
{
struct buf_t ret;
ret = buf_new(32);
if(**str == '\'') {
fatal("stub");
}
else if(**str == '"') {
fatal("stub");
}
else if(ch_str(**str)) {
do
buf_ch(&ret, *(*str)++);
while(ch_str(**str));
}
else
loc_err(loc, "Expected value.");
return buf_done(&ret);
}
void get_var(const char **str, char *var, struct loc_t loc)
{
const char *orig = *str;
if((**str == '@') || (**str == '^') || (**str == '<') || (**str == '~')) {
*var++ = *(*str)++;
*var = '\0';
}
else {
if(!ch_var(**str))
loc_err(loc, "Invalid variable name.");
do {
if((*str - orig) >= 255)
loc_err(loc, "Variable name too long.");
*var++ = *(*str)++;
} while(ch_var(**str));
*var = '\0';
}
}
void str_trim(const char **str)
{
while(ch_space(**str))
(*str)++;
}
char *str_fmt(const char *pat, ...)
{
char *ret;
va_list args;
va_start(args, pat);
ret = malloc(vsnprintf(NULL, 0, pat, args) + 1);
va_end(args);
va_start(args, pat);
vsprintf(ret, pat, args);
va_end(args);
return ret;
}
void str_set(char **dst, char *src)
{
if(*dst != NULL)
free(*dst);
*dst = src;
}
void str_final(char **str, const char *dir)
{
char *ptr, *find;
struct buf_t buf;
find = strchr(*str, '$');
if(find == NULL)
return;
ptr = *str;
buf = buf_new(2 * strlen(*str));
do {
buf_mem(&buf, ptr, find - ptr);
find++;
if(ch_num(find[0]))
fatal("stub");
else if(find[0] == '~')
buf_str(&buf, dir);
else if(find[0] == '$')
buf_ch(&buf, '$');
ptr = find + 1;
find = strchr(ptr, '$');
} while(find != NULL);
buf_str(&buf, ptr);
str_set(str, buf_done(&buf));
}
struct buf_t buf_new(uint32_t init)
{
return (struct buf_t){ malloc(init), 0, init };
}
void buf_delete(struct buf_t *buf)
{
free(buf->str);
}
char *buf_done(struct buf_t *buf)
{
buf_ch(buf, '\0');
return buf->str;
}
void buf_ch(struct buf_t *buf, char ch)
{
if(buf->len >= buf->max)
buf->str = realloc(buf->str, buf->max *= 2);
buf->str[buf->len++] = ch;
}
void buf_mem(struct buf_t *buf, const char *mem, uint32_t len)
{
while(len-- > 0)
buf_ch(buf, *mem++);
}
void buf_str(struct buf_t *buf, const char *str)
{
while(*str != '\0')
buf_ch(buf, *str++);
}
bool ch_space(int ch)
{
return (ch == ' ') || (ch == '\r') || (ch == '\t') || (ch == '\n') || (ch == '\v');
}
bool ch_alpha(int ch)
{
return ((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z'));
}
bool ch_num(int ch)
{
return (ch >= '0') && (ch <= '9');
}
bool ch_alnum(int ch)
{
return ch_alpha(ch) || ch_num(ch);
}
bool ch_var(int ch)
{
return ch_alnum(ch) || ch == '_';
}
bool ch_str(int ch)
{
return ch_alnum(ch) || (strchr("~/._-+=%", ch) != NULL);
}
bool ch_id(int ch)
{
return !ch_space(ch) && (ch != '{') && (ch != '}') && (ch != ':') && (ch != ';') && (ch != '=');
}
int64_t target_mtime(struct target_t *target)
{
if(target->mtime < 0)
target->mtime = os_mtime(target->path);
return target->mtime;
}
void target_conn(struct target_t *target, struct rule_t *rule)
{
struct edge_t *edge;
edge = malloc(sizeof(struct edge_t));
edge->rule = rule;
edge->next = target->edge;
target->edge = edge;
}
struct target_iter_t target_iter(struct target_list_t *list)
{
return (struct target_iter_t){ list->inst };
}
struct target_t *target_next(struct target_iter_t *iter)
{
struct target_t *target;
if(iter->inst == NULL)
return NULL;
target = iter->inst->target;
iter->inst = iter->inst->next;
return target;
}
struct target_list_t *target_list_new(void)
{
struct target_list_t *list;
list = malloc(sizeof(struct target_list_t));
*list = (struct target_list_t){ NULL };
return list;
}
void target_list_delete(struct target_list_t *list)
{
struct target_inst_t *inst, *tmp;
inst = list->inst;
while(inst != NULL) {
inst = (tmp = inst)->next;
free(tmp);
}
free(list);
}
uint32_t target_list_len(struct target_list_t *list)
{
uint32_t n = 0;
struct target_inst_t *inst;
for(inst = list->inst; inst != NULL; inst = inst->next)
n++;
return n;
}
bool target_list_contains(struct target_list_t *list, struct target_t *target)
{
struct target_inst_t *inst;
for(inst = list->inst; inst != NULL; inst = inst->next) {
if(inst->target == target)
return true;
}
return false;
}
void target_list_add(struct target_list_t *list, struct target_t *target)
{
struct target_inst_t **inst;
inst = &list->inst;
while(*inst != NULL)
inst = &(*inst)->next;
*inst = malloc(sizeof(struct target_inst_t));
(*inst)->target = target;
(*inst)->next = NULL;
}
struct target_t *target_list_find(struct target_list_t *list, bool spec, const char *path)
{
struct target_inst_t *inst;
for(inst = list->inst; inst != NULL; inst = inst->next) {
if((strcmp(inst->target->path, path) == 0))
return inst->target;
}
return NULL;
}
struct map_t *map_new(void)
{
struct map_t *map;
map = malloc(sizeof(struct map_t));
map->ent = NULL;
return map;
}
void map_delete(struct map_t *map)
{
struct ent_t *ent, *tmp;
ent = map->ent;
while(ent != NULL) {
ent = (tmp = ent)->next;
rt_ref_delete(tmp->target);
free(tmp);
}
free(map);
}
struct target_t *map_get(struct map_t *map, bool spec, const char *path)
{
struct ent_t *ent;
for(ent = map->ent; ent != NULL; ent = ent->next) {
if(strcmp(ent->target->path, path) == 0)
return ent->target;
}
return NULL;
}
void map_add(struct map_t *map, struct target_t *target)
{
struct ent_t *ent;
ent = malloc(sizeof(struct ent_t));
ent->target = target;
ent->next = map->ent;
map->ent = ent;
}
struct ast_bind_t *ast_bind_new(struct raw_t *id, enum ast_bind_e tag, union ast_bind_u data, bool add)
{
struct ast_bind_t *bind;
bind = malloc(sizeof(struct ast_bind_t));
bind->id = id;
bind->tag = tag;
bind->data = data;
bind->add = add;
return bind;
}
void ast_bind_delete(struct ast_bind_t *bind)
{
switch(bind->tag) {
case ast_val_v: imm_delete(bind->data.val); break;
case ast_func_v: fatal("FIXME stub");
case ast_block_v: ast_block_delete(bind->data.block); break;
}
raw_delete(bind->id);
free(bind);
}
struct ast_bind_t *ast_bind_val(struct raw_t *id, struct imm_t *val, bool add)
{
return ast_bind_new(id, ast_val_v, (union ast_bind_u){ .val = val }, add);
}
struct ast_bind_t *ast_bind_block(struct raw_t *id, struct ast_block_t *block, bool add)
{
return ast_bind_new(id, ast_block_v, (union ast_bind_u){ .block = block }, add);
}
struct ast_block_t *ast_block_new(void)
{
struct ast_block_t *block;
block = malloc(sizeof(struct ast_block_t));
block->stmt = NULL;
return block;
}
void ast_block_delete(struct ast_block_t *block)
{
stmt_clear(block->stmt);
free(block);
}
struct ast_env_t {
struct block_t *block;
};
struct ast_cmd_t *ast_cmd_new(struct ast_pipe_t *pipe)
{
struct ast_cmd_t *proc;
proc = malloc(sizeof(struct ast_cmd_t));
proc->pipe = pipe;
proc->in = proc->out = NULL;
proc->append = false;
return proc;
}
void ast_cmd_delete(struct ast_cmd_t *cmd)
{
if(cmd->in != NULL)
raw_delete(cmd->in);
if(cmd->out != NULL)
raw_delete(cmd->out);
ast_pipe_clear(cmd->pipe);
free(cmd);
}
struct ast_pipe_t *ast_pipe_new(struct imm_t *imm)
{
struct ast_pipe_t *pipe;
pipe = malloc(sizeof(struct ast_pipe_t));
pipe->imm = imm;
pipe->next = NULL;
return pipe;
}
void ast_pipe_clear(struct ast_pipe_t *pipe)
{
struct ast_pipe_t *tmp;
while(pipe != NULL) {
pipe = (tmp = pipe)->next;
imm_delete(tmp->imm);
free(tmp);
}
}
struct ast_rule_t *ast_rule_new(struct imm_t *gen, struct imm_t *dep, struct loc_t loc)
{
struct ast_rule_t *syn;
syn = malloc(sizeof(struct ast_rule_t));
syn->gen = gen;
syn->dep = dep;
syn->loc = loc;
syn->cmd = NULL;
return syn;
}
void ast_rule_delete(struct ast_rule_t *syn)
{
if(syn->cmd != NULL)
list_delete(syn->cmd);
imm_delete(syn->gen);
imm_delete(syn->dep);
free(syn);
}
void ast_rule_add(struct ast_rule_t *syn, struct imm_t *cmd)
{
list_add(syn->cmd, cmd);
}
struct ast_stmt_t *stmt_new(enum stmt_e tag, union stmt_u data, struct loc_t loc)
{
struct ast_stmt_t *stmt;
stmt = malloc(sizeof(struct ast_stmt_t));
stmt->tag = tag;
stmt->data = data;
stmt->loc = loc;
stmt->next = NULL;
return stmt;
}
void stmt_delete(struct ast_stmt_t *stmt)
{
switch(stmt->tag) {
case ast_bind_v: ast_bind_delete(stmt->data.bind); break;
case syn_v: ast_rule_delete(stmt->data.syn); break;
case loop_v: loop_delete(stmt->data.loop); break;
case print_v: print_delete(stmt->data.print); break;
case ast_mkdep_v: ast_mkdep_delete(stmt->data.mkdep); break;
case block_v: ast_block_delete(stmt->data.block); break;
case ast_inc_v: ast_inc_delete(stmt->data.inc); break;
}
free(stmt);
}
void stmt_clear(struct ast_stmt_t *stmt)
{
struct ast_stmt_t *tmp;
while(stmt != NULL) {
stmt = (tmp = stmt)->next;
stmt_delete(tmp);
}
}
struct ast_stmt_t *ast_stmt_mkdep(struct ast_mkdep_t *mkdep, struct loc_t loc)
{
return stmt_new(ast_mkdep_v, (union stmt_u){ .mkdep = mkdep }, loc);
}
struct ast_stmt_t *ast_stmt_inc(struct ast_inc_t *inc, struct loc_t loc)
{
return stmt_new(ast_inc_v, (union stmt_u){ .inc = inc }, loc);
}
struct ast_mkdep_t *ast_mkdep_new(struct imm_t *path, struct loc_t loc)
{
struct ast_mkdep_t *dep;
dep = malloc(sizeof(struct ast_mkdep_t));
*dep = (struct ast_mkdep_t){ path, loc };
return dep;
}
void ast_mkdep_delete(struct ast_mkdep_t *dep)
{
imm_delete(dep->path);
free(dep);
}
void ast_mkdep_eval(struct ast_mkdep_t *dep, struct rt_ctx_t *ctx, struct env_t *env)
{
struct val_t *val;
struct rt_obj_t obj;
obj = eval_imm(dep->path, ctx, env, dep->loc);
if(obj.tag == rt_null_v)
return;
if(obj.tag != rt_val_v)
loc_err(dep->loc, "Command `makedep` requires a string value.");
for(val = obj.data.val; val != NULL; val = val->next)
mk_eval(ctx, val->str, false);
rt_obj_delete(obj);
}
struct ast_inc_t *ast_inc_new(bool nest, bool opt, struct imm_t *imm)
{
struct ast_inc_t *inc;
inc = malloc(sizeof(struct ast_inc_t));
inc->nest = nest;
inc->opt = opt;
inc->imm = imm;
return inc;
}
void ast_inc_delete(struct ast_inc_t *inc)
{
imm_delete(inc->imm);
free(inc);
}
void ast_inc_eval(struct ast_inc_t *inc, struct rt_ctx_t *ctx, struct env_t *env, struct loc_t loc)
{
struct val_t *val;
struct rt_obj_t obj;
struct env_t *nest;
struct ast_block_t *top;
obj = eval_imm(inc->imm, ctx, env, loc);
if(obj.tag == rt_null_v)
return;
else if(obj.tag != rt_val_v)
loc_err(loc, "%s require string values.", inc->nest ? "Import" : "Include");
for(val = obj.data.val; val != NULL; val = val->next) {
top = ham_load(val->str);
if(top == NULL) {
if(inc->opt)
continue;
loc_err(loc, "Cannot open '%s'.", val->str);
}
if(inc->nest) {
nest = rt_env_new(env);
eval_block(top, ctx, nest);
rt_env_delete(nest);
}
else
eval_block(top, ctx, env);
ast_block_delete(top);
rt_obj_delete(obj);
}
}
void mk_eval(struct rt_ctx_t *ctx, const char *path, bool strict)
{
int ch;
char *str;
FILE *file;
struct target_list_t *gens, *deps;
file = fopen(path, "r");
if(file == NULL) {
if(!strict)
return;
fatal("Cannot open '%s'.", path);
}
gens = target_list_new();
deps = target_list_new();
ch = fgetc(file);
for(;;) {
while(mk_space(ch) || (ch == '\n'))
ch = fgetc(file);
if(ch == EOF)
break;
while((str = mk_str(file, &ch)) != NULL) {
target_list_add(gens, ctx_target(ctx, false, str));
free(str);
}
mk_trim(file, &ch);
if(ch != ':') {
fprintf(stderr, "%s: Invalid makedep file.\n", path);
if(strict)
exit(1);
goto clean;
}
ch = fgetc(file);
mk_trim(file, &ch);
while((str = mk_str(file, &ch)) != NULL) {
target_list_add(deps, ctx_target(ctx, false, str));
free(str);
}
ctx_rule(ctx, NULL, gens, deps);
gens = target_list_new();
deps = target_list_new();
}
clean:
target_list_delete(gens);
target_list_delete(deps);
fclose(file);
}
void mk_trim(FILE *file, int *ch)
{
for(;;) {
while(mk_space(*ch))
*ch = fgetc(file);
if(*ch != '\\')
break;
if((*ch = fgetc(file)) != '\n')
break;
*ch = fgetc(file);
}
}
char *mk_str(FILE *file, int *ch)
{
struct buf_t buf;
mk_trim(file, ch);
if(!mk_ident(*ch))
return NULL;
buf = buf_new(32);
do
buf_ch(&buf, *ch);
while(mk_ident(*ch = fgetc(file)));
return buf_done(&buf);
}
bool mk_space(int ch)
{
return (ch == ' ') || (ch == '\t') || (ch == '\r');
}
bool mk_ident(int ch)
{
return (ch > 0) && !mk_space(ch) && (ch != ':') && (ch != '\n');
}
struct target_t *rt_ref_new(bool spec, char *path)
{
struct target_t *ref;
ref = malloc(sizeof(struct target_t));
*ref = (struct target_t){ path, spec ? FLAG_SPEC : 0, -1, NULL, NULL };
return ref;
}
void rt_ref_delete(struct target_t *ref)
{
struct edge_t *edge, *tmp;
edge = ref->edge;
while(edge != NULL) {
edge = (tmp = edge)->next;
free(tmp);
}
free(ref->path);
free(ref);
}
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
int64_t os_memcnt = 0;
void print(const char *fmt, ...)
{
va_list args;
va_start(args, fmt);
vprintf(fmt, args);
va_end(args);
}
void printv(const char *fmt, va_list args)
{
vprintf(fmt, args);
}
void _fatal(const char *file, unsigned long line, const char *fmt, ...)
{
va_list args;
va_start(args, fmt);
vfprintf(stderr, fmt, args);
fprintf(stderr, "\n");
abort(); }
void unreachable(void)
{
fprintf(stderr, "fatal error: unreachable code\n");
abort();
}
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
void os_init(void)
{
setlinebuf(stdout);
}
struct os_job_t {
int *pid;
uint32_t len;
};
int os_exec(struct cmd_t *cmd)
{
uint32_t i, n;
char **args;
struct val_t *val;
struct rt_pipe_t *iter;
pid_t pid = 0;
int in = -1, out = -1, pair[2];
for(iter = cmd->pipe; iter != NULL; iter = iter->next) {
val = iter->cmd;
n = val_len(val);
args = malloc(sizeof(void *) * (n + 1));
for(i = 0; i < n; i++) {
args[i] = strdup(val->str);
val = val->next;
}
args[n] = NULL;
if(cmd->in && (iter == cmd->pipe)) {
in = open(cmd->in, O_RDONLY);
if(in < 0)
fatal("Cannot open '%s' for reading. %s.", in, strerror(errno));
}
else if(iter != cmd->pipe)
in = pair[0];
else
in = -1;
if(cmd->out && (iter->next == NULL)) {
out = open(cmd->out, O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC), 0644);
if(out < 0)
fatal("Cannot open '%s' for writing . %s.", out, strerror(errno));
}
else if(iter->next != NULL) {
if(pipe(pair) < 0)
fatal("Cannot create pipe. %s.", strerror(errno));
out = pair[1];
}
else
out = -1;
pid = vfork();
if(pid == 0) {
if(in >= 0) {
dup2(in, STDIN_FILENO);
close(in);
}
if(out >= 0) {
dup2(out, STDOUT_FILENO);
close(out);
}
execvp(args[0], args);
}
if(in >= 0)
close(in);
if(out >= 0)
close(out);
for(i = 0; i < n; i++)
free(args[i]);
free(args);
}
return pid;
}
int os_wait(void)
{
int pid, stat;
for(;;) {
pid = wait(&stat);
if(WIFSIGNALED(stat))
fatal("Command failed with signal '%d'.", WTERMSIG(stat));
if(pid >= 0)
break;
if(errno != EINTR)
fatal("Failed to wait. %s.", strerror(errno));
}
stat = WEXITSTATUS(stat);
if(stat != 0)
fatal("Command terminated with status %d.", stat);
return pid;
}
int64_t os_mtime(const char *path)
{
struct stat info;
if(stat(path, &info) < 0)
return INT64_MIN;
return 1000000 * info.st_mtim.tv_sec + info.st_mtim.tv_nsec / 1000;
}
void os_mkdir(const char *path)
{
mkdir(path, 0777);
}
