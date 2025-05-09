#pragma once

/*
 * type declarations
 */
struct el_type_t el_type_new(enum el_type_e kind, union el_type_u data);
struct el_type_t el_type_copy(struct el_type_t type);
void el_type_delete(struct el_type_t type);

void el_type_replace(struct el_type_t *dst, struct el_type_t src);

struct el_type_t el_type_void(void);
struct el_type_t el_type_i8(void);
struct el_type_t el_type_u8(void);
struct el_type_t el_type_i16(void);
struct el_type_t el_type_u16(void);
struct el_type_t el_type_i32(void);
struct el_type_t el_type_u32(void);
struct el_type_t el_type_i64(void);
struct el_type_t el_type_u64(void);
struct el_type_t el_type_bool(void);
struct el_type_t el_type_num(void);
struct el_type_t el_type_str(void);
struct el_type_t el_type_ptr(struct el_ptr_t *ptr);
struct el_type_t el_type_arr(struct el_arr_t *arr);
struct el_type_t el_type_func(struct el_func_t *func);
struct el_type_t el_type_enum(struct el_enum_t *venum);
struct el_type_t el_type_struct(struct el_struct_t *vstruct);
struct el_type_t el_type_union(struct el_struct_t *vstruct);
struct el_type_t el_type_alias(struct el_alias_t *alias);
struct el_type_t el_type_args(void);

struct el_type_t el_type_err(struct el_type_t val, struct el_type_t err);

struct el_type_t el_type_root(struct el_type_t type);

int32_t el_type_size(struct el_type_t type);
static inline int32_t el_type_nbytes(struct el_type_t type) { return (el_type_size(type) + 7) / 8; }
bool el_type_isint(struct el_type_t type);
bool el_type_is_int(struct el_type_t type);
bool el_type_is_num(struct el_type_t type);
bool el_type_is_bool(struct el_type_t type);
bool el_type_is_enum(struct el_type_t type);
bool el_type_is_ptr(struct el_type_t type);
bool el_type_is_arr(struct el_type_t type);
bool el_type_is_comp(struct el_type_t type);
bool el_type_is_void(struct el_type_t type);
bool el_type_is_signed(struct el_type_t type);
bool el_type_is_err(struct el_type_t type);
bool el_type_is_struct(struct el_type_t type);
struct el_type_t el_err_get_val(struct el_type_t type);
struct el_type_t el_err_get_err(struct el_type_t type);

int el_type_cmp(struct el_type_t lhs, const struct el_type_t rhs);
int el_type_ptrcmp(const void *lhs, const void *rhs);

void el_type_tostr(struct el_type_t type, char *str, uint32_t size);

/*
 * alias declarations
 */
struct el_alias_t *el_alias_new(char *id, struct el_type_t type);
struct el_alias_t *el_alias_copy(const struct el_alias_t *alias);
void el_alias_delete(struct el_alias_t *alias);
struct el_type_t el_alias_base(struct el_alias_t *alias);

/*
 * pointer declarations
 */
struct el_ptr_t *el_ptr_new(uint32_t flags, struct el_type_t sub);
struct el_ptr_t *el_ptr_copy(const struct el_ptr_t *ptr);
void el_ptr_delete(struct el_ptr_t *ptr);

/*
 * function type declarations
 */
struct el_func_t *el_func_new(struct el_type_t ret);
struct el_func_t *el_func_copy(const struct el_func_t *func);
void el_func_delete(struct el_func_t *func);

int32_t el_func_find(struct el_func_t *func, const char *id);
struct el_param_t *el_func_get(struct el_func_t *func, uint32_t idx);
void el_func_append(struct el_func_t *func, struct el_type_t type, char *id);

/*
 * structure type declarations
 */
struct el_struct_t *el_struct_new(char *id, struct el_member_t *member);
struct el_struct_t *el_struct_copy(struct el_struct_t *vstruct);
void el_struct_delete(struct el_struct_t *vstruct);

uint32_t el_struct_cnt(struct el_struct_t *vstruct);
int32_t el_struct_size(struct el_struct_t *vstruct);
int32_t el_struct_max(struct el_struct_t *vstruct);
int32_t el_struct_offset(struct el_struct_t *vstruct, uint32_t idx);
struct el_member_t *el_struct_get(struct el_struct_t *vstruct, uint32_t idx);
int32_t el_struct_find(struct el_struct_t *vstruct, const char *id);

struct el_member_t *el_member_new(char *id, struct el_type_t type);
struct el_member_t *el_member_copy(struct el_member_t *member);
struct el_member_t *el_member_clone(struct el_member_t *member);
void el_member_delete(struct el_member_t *member);
void el_member_clear(struct el_member_t *member);

/*
 * enumerator declarations
 */
struct el_enum_t *el_enum_new(char *id, struct el_repr_t *repr);
struct el_enum_t *el_enum_copy(const struct el_enum_t *venum);
void el_enum_delete(struct el_enum_t *venum);
struct el_repr_t *el_enum_find(struct el_enum_t *venum, const char *id);

struct el_repr_t *el_repr_new(char *id, int64_t val);
struct el_repr_t *el_repr_copy(const struct el_repr_t *repr);
struct el_repr_t *el_repr_clone(const struct el_repr_t *repr);
void el_repr_delete(struct el_repr_t *repr);

/*
 * error type declarations
 */
struct el_etype_t *el_etype_new(struct el_type_t val, struct el_type_t err);
void el_etype_delete(struct el_etype_t *etype);

struct el_arr_t *el_arr_new(struct el_type_t type, uint32_t len);
void el_arr_delete(struct el_arr_t *arr);
