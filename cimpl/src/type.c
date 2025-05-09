#include "common.h"


/**
 * Create a type.
 *   @kind: The kind.
 *   @data: Consumed. The data.
 *   &returns: The type.
 */
struct el_type_t el_type_new(enum el_type_e kind, union el_type_u data)
{
	return (struct el_type_t){ kind, data };
}

/**
 * Copy a type.
 *   @type: The original type.
 *   &returns: The copied type.
 */
struct el_type_t el_type_copy(struct el_type_t type)
{
	switch(type.kind) {
	case el_void_v: return el_type_void();
	case el_i8_v: return el_type_i8();
	case el_u8_v: return el_type_u8();
	case el_i16_v: return el_type_i16();
	case el_u16_v: return el_type_u16();
	case el_i32_v: return el_type_i32();
	case el_u32_v: return el_type_u32();
	case el_i64_v: return el_type_i64();
	case el_u64_v: return el_type_u64();
	case el_bool_v: return el_type_bool();
	case el_num_v: return el_type_num();
	case el_str_v: return el_type_str();
	case el_ptr_v: return el_type_ptr(el_ptr_copy(type.data.ptr));
	case el_arr_v: return el_type_arr(el_arr_new(el_type_copy(type.data.arr->type), type.data.arr->len));
	case el_alias_v: return el_type_alias(el_alias_copy(type.data.alias));
	case el_func_v: return el_type_func(el_func_copy(type.data.func));
	case el_struct_v: return el_type_struct(el_struct_copy(type.data.vstruct));
	case el_union_v: return el_type_union(el_struct_copy(type.data.vstruct));
	case el_enum_v: return el_type_enum(el_enum_copy(type.data.venum));
	case el_args_v: return el_type_args();
	}

	unreachable();
}

/**
 * Delete a type.
 *   @type: The type.
 */
void el_type_delete(struct el_type_t type)
{
	switch(type.kind) {
	case el_ptr_v:
		el_ptr_delete(type.data.ptr);
		break;

	case el_func_v:
		el_func_delete(type.data.func);
		break;

	case el_struct_v:
	case el_union_v:
		el_struct_delete(type.data.vstruct);
		break;

	case el_enum_v:
		el_enum_delete(type.data.venum);
		break;

	case el_alias_v:
		el_alias_delete(type.data.alias);
		break;

	case el_arr_v:
		el_arr_delete(type.data.arr);
		break;

	default:
		break;
	}
}


/**
 * Replace a destination type.
 *   @dst: The destination.
 *   @src: The source.
 */
void el_type_replace(struct el_type_t *dst, struct el_type_t src)
{
	el_type_delete(*dst);
	*dst = src;
}


/**
 * Create a void type.
 *   &returns: The type.
 */
struct el_type_t el_type_void(void)
{
	return el_type_new(el_void_v, (union el_type_u){ });
}

/**
 * Create a 8-bit signed integer type.
 *   &returns: The type.
 */
struct el_type_t el_type_i8(void)
{
	return el_type_new(el_i8_v, (union el_type_u){ });
}

/**
 * Create a 8-bit unsigned integer type.
 *   &returns: The type.
 */
struct el_type_t el_type_u8(void)
{
	return el_type_new(el_u8_v, (union el_type_u){ });
}

/**
 * Create a 16-bit signed integer type.
 *   &returns: The type.
 */
struct el_type_t el_type_i16(void)
{
	return el_type_new(el_i16_v, (union el_type_u){ });
}

/**
 * Create a 16-bit unsigned integer type.
 *   &returns: The type.
 */
struct el_type_t el_type_u16(void)
{
	return el_type_new(el_u16_v, (union el_type_u){ });
}

/**
 * Create a 32-bit signed integer type.
 *   &returns: The type.
 */
struct el_type_t el_type_i32(void)
{
	return el_type_new(el_i32_v, (union el_type_u){ });
}

/**
 * Create a 32-bit unsigned integer type.
 *   &returns: The type.
 */
struct el_type_t el_type_u32(void)
{
	return el_type_new(el_u32_v, (union el_type_u){ });
}

/**
 * Create a 64-bit signed integer type.
 *   &returns: The type.
 */
struct el_type_t el_type_i64(void)
{
	return el_type_new(el_i64_v, (union el_type_u){ });
}

/**
 * Create a 64-bit unsigned integer type.
 *   &returns: The type.
 */
struct el_type_t el_type_u64(void)
{
	return el_type_new(el_u64_v, (union el_type_u){ });
}

/**
 * Create a boolean type.
 *   &returns: The type.
 */
struct el_type_t el_type_bool(void)
{
	return el_type_new(el_bool_v, (union el_type_u){ });
}

/**
 * Create a number type.
 *   &returns: The type.
 */
struct el_type_t el_type_num(void)
{
	return el_type_new(el_num_v, (union el_type_u){ });
}

/**
 * Create a string type.
 *   &returns: The type.
 */
struct el_type_t el_type_str(void)
{
	return el_type_new(el_str_v, (union el_type_u){ });
}

/**
 * Create a pointer type.
 *   @ptr: Consumed. The pointer type.
 *   &returns: The type.
 */
struct el_type_t el_type_ptr(struct el_ptr_t *ptr)
{
	return el_type_new(el_ptr_v, (union el_type_u){ .ptr = ptr });
}

/**
 * Create an array type.
 *   @arr: Consumed. The pointer type.
 *   &returns: The type.
 */
struct el_type_t el_type_arr(struct el_arr_t *arr)
{
	return el_type_new(el_arr_v, (union el_type_u){ .arr = arr });
}

/**
 * Create a function type.
 *   @func: Consumed. The function type.
 *   &returns: The type.
 */
struct el_type_t el_type_func(struct el_func_t *func)
{
	return el_type_new(el_func_v, (union el_type_u){ .func = func });
}

/**
 * Create a structure type.
 *   @vstruct: Consumed. The structure.
 *   &returns: The type.
 */
struct el_type_t el_type_struct(struct el_struct_t *vstruct)
{
	return el_type_new(el_struct_v, (union el_type_u){ .vstruct = vstruct });
}

/**
 * Create a enum type.
 *   @venum: Consumed. The enumerator.
 *   &returns: The type.
 */
struct el_type_t el_type_enum(struct el_enum_t *venum)
{
	return el_type_new(el_enum_v, (union el_type_u){ .venum = venum });
}

/**
 * Create a union type.
 *   @vstruct: Consumed. The structure.
 *   &returns: The type.
 */
struct el_type_t el_type_union(struct el_struct_t *vstruct)
{
	return el_type_new(el_union_v, (union el_type_u){ .vstruct = vstruct });
}

/**
 * Create an alias type.
 *   @alias: Consumed. The alias.
 *   &returns: The type.
 */
struct el_type_t el_type_alias(struct el_alias_t *alias)
{
	return el_type_new(el_alias_v, (union el_type_u){ .alias = alias });
}

/**
 * Create an arguments.
 *   &returns: The type.
 */
struct el_type_t el_type_args(void)
{
	return el_type_new(el_args_v, (union el_type_u){ });
}


/**
 * Retrieve the root type.
 *   @type: The type.
 *   &returns: The root type.
 */
struct el_type_t el_type_root(struct el_type_t type)
{
	while(type.kind == el_alias_v)
		type = type.data.alias->type;

	return type;
}


/**
 * Retrieve the size of a type.
 *   @type: The type.
 *   &returns: The size. Negative indicates an invalid type size.
 */
int32_t el_type_size(struct el_type_t type)
{
	switch(type.kind) {
	case el_void_v: return -1;
	case el_i8_v: return 8;
	case el_u8_v: return 8;
	case el_i16_v: return 16;
	case el_u16_v: return 16;
	case el_i32_v: return 32;
	case el_u32_v: return 32;
	case el_i64_v: return 64;
	case el_u64_v: return 64;
	case el_bool_v: return 8;
	case el_num_v: return -1;
	case el_arr_v: return type.data.arr->len * el_type_size(type.data.arr->type);
	case el_alias_v: return el_type_size(el_alias_base(type.data.alias));
	case el_ptr_v: return 64;
	case el_str_v: return 64;
	case el_func_v: return -1;
	case el_struct_v: return el_struct_size(type.data.vstruct);
	case el_union_v: return el_struct_max(type.data.vstruct) ?: 1;
	case el_enum_v: return 32;
	case el_args_v: return -1;
	}

	unreachable();
}

/**
 * Determine if a type is a non-pointer integer.
 *   @type: The type.
 *   &returns: True if integer.
 */
bool el_type_is_int(struct el_type_t type)
{
	switch(type.kind) {
	case el_void_v: return false;
	case el_i8_v: return true;
	case el_u8_v: return true;
	case el_i16_v: return true;
	case el_u16_v: return true;
	case el_i32_v: return true;
	case el_u32_v: return true;
	case el_i64_v: return true;
	case el_u64_v: return true;
	case el_bool_v: return true;
	case el_num_v: return false;
	case el_ptr_v: return false;
	case el_str_v: return false;
	case el_arr_v: return false;
	case el_alias_v: return el_type_isint(el_alias_base(type.data.alias));
	case el_func_v: return false;
	case el_struct_v: return false;
	case el_union_v: return false;
	case el_enum_v: return false;
	case el_args_v: return false;
	}

	unreachable();
}

/**
 * Determine if a type is a number.
 *   @type: The type.
 *   &returns: True if integer.
 */
bool el_type_is_num(struct el_type_t type)
{
	switch(type.kind) {
	case el_void_v: return false;
	case el_i8_v: return true;
	case el_u8_v: return true;
	case el_i16_v: return true;
	case el_u16_v: return true;
	case el_i32_v: return true;
	case el_u32_v: return true;
	case el_i64_v: return true;
	case el_u64_v: return true;
	case el_bool_v: return true;
	case el_num_v: return true;
	case el_ptr_v: return false;
	case el_str_v: return false;
	case el_arr_v: return false;
	case el_alias_v: return el_type_isint(el_alias_base(type.data.alias));
	case el_func_v: return false;
	case el_struct_v: return false;
	case el_union_v: return false;
	case el_enum_v: return false;
	case el_args_v: return false;
	}

	unreachable();
}

bool el_type_is_bool(struct el_type_t type)
{
	return el_type_root(type).kind == el_bool_v;
}

bool el_type_is_enum(struct el_type_t type)
{
	return el_type_root(type).kind == el_enum_v;
}

/**
 * Determine if a type is a pointer.
 *   @type: The type.
 *   &returns: True if pointer.
 */
bool el_type_is_ptr(struct el_type_t type)
{
	return el_type_root(type).kind == el_ptr_v;
}

/**
 * Determine if a type is a array.
 *   @type: The type.
 *   &returns: True if array.
 */
bool el_type_is_arr(struct el_type_t type)
{
	return el_type_root(type).kind == el_arr_v;
}

/**
 * Determine if a type is an integer.
 *   @type: The type.
 *   &returns: True if integer.
 */
bool el_type_isint(struct el_type_t type)
{
	switch(type.kind) {
	case el_void_v: return false;
	case el_i8_v: return true;
	case el_u8_v: return true;
	case el_i16_v: return true;
	case el_u16_v: return true;
	case el_i32_v: return true;
	case el_u32_v: return true;
	case el_i64_v: return true;
	case el_u64_v: return true;
	case el_bool_v: return true;
	case el_num_v: return false;
	case el_ptr_v: return true;
	case el_str_v: return true;
	case el_arr_v: return true;
	case el_alias_v: return el_type_isint(el_alias_base(type.data.alias));
	case el_func_v: return false;
	case el_struct_v: return false;
	case el_union_v: return false;
	case el_enum_v: return false;
	case el_args_v: return false;
	}

	unreachable();
}

bool el_type_is_comp(struct el_type_t type)
{
	return (el_type_root(type).kind == el_struct_v);
}

bool el_type_is_void(struct el_type_t type)
{
	return el_type_root(type).kind == el_void_v;
}

bool el_type_is_signed(struct el_type_t type)
{
	switch(type.kind) {
	case el_void_v: return false;
	case el_i8_v: return true;
	case el_u8_v: return false;
	case el_i16_v: return true;
	case el_u16_v: return false;
	case el_i32_v: return true;
	case el_u32_v: return false;
	case el_i64_v: return true;
	case el_u64_v: return false;
	case el_bool_v: return false;
	case el_num_v: return false;
	case el_ptr_v: return false;
	case el_str_v: return false;
	case el_arr_v: return false;
	case el_alias_v: return el_type_is_signed(el_alias_base(type.data.alias));
	case el_func_v: return false;
	case el_struct_v: return false;
	case el_union_v: return false;
	case el_enum_v: return false;
	case el_args_v: return false;
	}

	unreachable();
}

bool el_type_is_err(struct el_type_t type)
{
	if(type.kind != el_struct_v)
		return false;

	return strcmp(type.data.vstruct->id, "!e") == 0;
}

bool el_type_is_struct(struct el_type_t type)
{
	return el_type_root(type).kind == el_struct_v;
}

struct el_type_t el_err_get_val(struct el_type_t type)
{
	int32_t idx;

	if(type.kind != el_struct_v)
		fatal("invalid err");

	idx = el_struct_find(type.data.vstruct, "val");
	if(idx < 0)
		return el_type_void();

	return el_struct_get(type.data.vstruct, idx)->type;
}

struct el_type_t el_err_get_err(struct el_type_t type)
{
	if(type.kind != el_struct_v)
		fatal("invalid err");

	return el_struct_get(type.data.vstruct, el_struct_find(type.data.vstruct, "err"))->type;
}



/**
 * Compare two named types for order.
 *   @lhs: The left-hand side.
 *   @rhs: The right-hand side.
 *   &returns: Their order.
 */
int el_type_cmp(struct el_type_t lhs, const struct el_type_t rhs)
{
	if(lhs.kind < rhs.kind)
		return -1;
	else if(lhs.kind > rhs.kind)
		return 1;

	switch(lhs.kind) {
	case el_void_v:
	case el_i8_v:
	case el_u8_v:
	case el_i16_v:
	case el_u16_v:
	case el_i32_v:
	case el_u32_v:
	case el_i64_v:
	case el_u64_v:
	case el_num_v:
	case el_ptr_v:
	case el_func_v:
	case el_str_v:
	case el_bool_v:
	case el_args_v:
	case el_arr_v:
		unreachable();

	case el_alias_v:
	case el_struct_v:
		return avl_cmp_str(lhs.data.vstruct->id, lhs.data.vstruct->id);

	case el_union_v:
	case el_enum_v:
		fatal("stub");
	}

	unreachable();
}

/**
 pointers of * Compare two pointers of named types for order.
 *   @lhs: The left-hand side.
 *   @rhs: The right-hand side.
 *   &returns: Their order.
 */
int el_type_ptrcmp(const void *lhs, const void *rhs)
{
	return el_type_cmp(*(const struct el_type_t *)lhs, *(const struct el_type_t *)rhs);
}


/**
 * Create an alias.
 *   @id: Consumed. The identifier.
 *   @type: Consumed. The type.
 *   &returns: The alias.
 */
struct el_alias_t *el_alias_new(char *id, struct el_type_t type)
{
	struct el_alias_t *alias;

	alias = malloc(sizeof(struct el_alias_t));
	alias->id = id;
	alias->type = type;

	return alias;
}

/**
 * Copy a type alias.
 *   @alias: The original alias.
 *   &returns: The copy.
 */
struct el_alias_t *el_alias_copy(const struct el_alias_t *alias)
{
	return el_alias_new(strdup(alias->id), el_type_copy(alias->type));
}

/**
 * Delete an alias.
 *   @alias: The alias.
 */
void el_alias_delete(struct el_alias_t *alias)
{
	el_type_delete(alias->type);
	free(alias->id);
	free(alias);
}

/**
 * Retrieve the base type of an alias chain.
 *   @alias: The alias.
 *   &returns: The type.
 */
struct el_type_t el_alias_base(struct el_alias_t *alias)
{
	while(alias->type.kind == el_alias_v)
		alias = alias->type.data.alias;

	return alias->type;
}


/**
 * Create a pointer type.
 *   @flags: The flags.
 *   @sub: Consumed. The subtype.
 *   &returns: The pointer.
 */
struct el_ptr_t *el_ptr_new(uint32_t flags, struct el_type_t sub)
{
	struct el_ptr_t *ptr;

	ptr = malloc(sizeof(struct el_ptr_t));
	ptr->flags = flags;
	ptr->sub = sub;

	return ptr;
}

/**
 * Copy a pointer type.
 *   @ptr: The orignal pointer
 *   &returns: The pointer.
 */
struct el_ptr_t *el_ptr_copy(const struct el_ptr_t *ptr)
{
	return el_ptr_new(ptr->flags, el_type_copy(ptr->sub));
}

/**
 * Delete a pointer.
 *   @ptr: The pointer.
 */
void el_ptr_delete(struct el_ptr_t *ptr)
{
	el_type_delete(ptr->sub);
	free(ptr);
}


/**
 * Convert a type to a string.
 *   @type: The type.
 *   @str: The string.
 *   @size: The size.
 */
void el_type_tostr(struct el_type_t type, char *str, uint32_t size)
{
#define adj() do { size -= strlen(str); str += strlen(str); } while(0)
	switch(type.kind) {
	case el_void_v: snprintf(str, size, "void"); break;
	case el_i8_v: snprintf(str, size, "i8"); break;
	case el_u8_v: snprintf(str, size, "u8"); break;
	case el_i16_v: snprintf(str, size, "i16"); break;
	case el_u16_v: snprintf(str, size, "u16"); break;
	case el_i32_v: snprintf(str, size, "i32"); break;
	case el_u32_v: snprintf(str, size, "u32"); break;
	case el_i64_v: snprintf(str, size, "i64"); break;
	case el_u64_v: snprintf(str, size, "u64"); break;
	case el_bool_v: snprintf(str, size, "bool"); break;
	case el_num_v: snprintf(str, size, "num"); break;
	case el_alias_v:
		snprintf(str, size, "alias:%s:", type.data.alias->id);
		size -= strlen(str); str += strlen(str);

		el_type_tostr(type.data.alias->type, str, size);
		break;

	case el_arr_v:
		snprintf(str, size, "arr[%u]:", type.data.arr->len);
		size -= strlen(str); str += strlen(str);

		el_type_tostr(type.data.arr->type, str, size);
		break;

	case el_ptr_v: {
		struct el_ptr_t *ptr = type.data.ptr;

		if(ptr->flags & EL_CONST)
			snprintf(str, size, "cpt:");
		else
			snprintf(str, size, "pt:");

		adj();
		el_type_tostr(type.data.ptr->sub, str, size);
	} break;

	case el_str_v: snprintf(str, size, "str"); break;

	case el_func_v: {
		uint32_t i;
		struct el_func_t *func = type.data.func;

		el_type_tostr(func->ret, str, size); adj();
		snprintf(str, size, "("); adj();

		for(i = 0; i < func->cnt; i++) {
			if(i > 0) {
				snprintf(str, size, ", "); adj();
			}

			el_type_tostr(func->param[i].type, str, size); adj();
			snprintf(str, size, " %s", func->param[i].id);
		}

		snprintf(str, size, ")"); adj();
	} break;

	case el_struct_v:
		snprintf(str, size, "st:%s{", type.data.vstruct->id);
		size -= strlen(str); str += strlen(str);

		for(struct el_member_t *member = type.data.vstruct->member; member != NULL; member = member->next) {
			el_type_tostr(member->type, str, size);
			size -= strlen(str); str += strlen(str);
			snprintf(str, size, ";");
			size -= strlen(str); str += strlen(str);
		}

		snprintf(str, size, "}");
		size -= strlen(str); str += strlen(str);

		break;

	case el_args_v:
		snprintf(str, size, "args");
		size -= strlen(str); str += strlen(str);
		break;

	case el_enum_v:
		snprintf(str, size, "%s", type.data.venum->id);
		break;

	case el_union_v:
		//fatal("stub");
		snprintf(str, size, "un:");
		size -= strlen(str); str += strlen(str);
		break;
	}
}


/**
 * Function type structure.
 *   @ret: The return type.
 *   &returns: The function type.
 */
struct el_func_t *el_func_new(struct el_type_t ret)
{
	struct el_func_t *func;

	func = malloc(sizeof(struct el_func_t));
	func->ret = ret;
	func->cnt = 0;
	func->var = false;
	func->param = malloc(0);
	func->check = el_check_none();

	return func;
}

/**
 * Copy a function type.
 *   @func: The function.
 *   &return: The copy.
 */
struct el_func_t *el_func_copy(const struct el_func_t *func)
{
	struct el_func_t *copy;
	uint32_t i;

	copy = malloc(sizeof(struct el_func_t));
	copy->ret = el_type_copy(func->ret);
	copy->cnt = func->cnt;
	copy->var = func->var;
	copy->param = malloc(func->cnt * sizeof(struct el_param_t));
	copy->check = el_check_copy(func->check);

	for(i = 0; i < func->cnt; i++) {
		copy->param[i].id = func->param[i].id ? strdup(func->param[i].id) : NULL;
		copy->param[i].type = el_type_copy(func->param[i].type);
	}

	return copy;
}

/**
 * Delete a function type.
 *   @func: The function type.
 */
void el_func_delete(struct el_func_t *func)
{
	uint32_t i;
	
	for(i = 0; i < func->cnt; i++) {
		el_type_delete(func->param[i].type);

		if(func->param[i].id != NULL)
			free(func->param[i].id);
	}

	el_check_delete(func->check);
	el_type_delete(func->ret);
	free(func->param);
	free(func);
}


/**
 * Find a parameter given a name.
 *   @func: The function type.
 *   @id: The identifier.
 *   &returns: The index if found, `-1` if not found.
 */
int32_t el_func_find(struct el_func_t *func, const char *id)
{
	uint32_t i;

	for(i = 0; i < func->cnt; i++) {
		if(func->param[i].id != NULL) {
			if(strcmp(func->param[i].id, id) == 0)
				return i;
		}
	}

	return -1;
}

/**
 * Get a parameter given an index.
 *   @func: The function.
 *   @idx: The index.
 *   &returns: The parameter.
 */
struct el_param_t *el_func_get(struct el_func_t *func, uint32_t idx)
{
	assert(idx < func->cnt);

	return &func->param[idx];
}

/**
 * Append a parameter to the function type.
 *   @func: The function.
 *   @type: Consumed. The type.
 *   @id: Consumed. Optional. The identifier.
 *   &returns: The function.
 */
void el_func_append(struct el_func_t *func, struct el_type_t type, char *id)
{
	func->param = realloc(func->param, (func->cnt + 1) * sizeof(struct el_param_t));
	func->param[func->cnt++] = (struct el_param_t){ id, type };
}


/**
 * Create a structure type.
 *   @id: Consumed. Optional. The identifier.
 *   @member: Consumed. Optional. The member list.
 *   &returns: The structure.
 */
struct el_struct_t *el_struct_new(char *id, struct el_member_t *member)
{
	struct el_struct_t *vstruct;

	vstruct = malloc(sizeof(struct el_struct_t));
	vstruct->id = id;
	vstruct->member = member;

	return vstruct;
}

/**
 * Copy a structure.
 *   @vstruct: The structure.
 *   &returns: The copy.
 */
struct el_struct_t *el_struct_copy(struct el_struct_t *vstruct)
{
	struct el_struct_t *copy;

	copy = malloc(sizeof(struct el_struct_t));
	copy->id = strdup(vstruct->id);
	copy->member = el_member_clone(vstruct->member);

	return copy;
}

/**
 * Delete a structure type.
 *   @vstruct: The structure.
 */
void el_struct_delete(struct el_struct_t *vstruct)
{
	if(vstruct->id != NULL)
		free(vstruct->id);

	el_member_clear(vstruct->member);
	free(vstruct);
}


/**
 * Count the number of members in the structure.
 *   @vstruct: The structure.
 *   &returns: The count.
 */
uint32_t el_struct_cnt(struct el_struct_t *vstruct)
{
	uint32_t n = 0;
	struct el_member_t *member;

	for(member = vstruct->member; member != NULL; member = member->next)
		n++;

	return n;
}

/**
 * Compute the size of a structure.
 *   @vstruct: The structure.
 *   &returns: The size. Negative indicates an incomplete structure.
 */
int32_t el_struct_size(struct el_struct_t *vstruct)
{
	int32_t tmp, size = 0;
	struct el_member_t *member;

	for(member = vstruct->member; member != NULL; member = member->next) {
		tmp = el_type_size(member->type);
		if(tmp < 0)
			return tmp;

		size += tmp;
	}

	return size;
}

int32_t el_struct_max(struct el_struct_t *vstruct)
{
	int32_t tmp, size = 0;
	struct el_member_t *member;

	for(member = vstruct->member; member != NULL; member = member->next) {
		tmp = el_type_size(member->type);
		if(tmp < 0)
			return tmp;

		if(tmp > size)
			size = tmp;
	}

	return size;
}

/**
 * Compute the byte offset of a member index.
 *   @vstruct: The structure.
 *   @idx: The index.
 *   &returns: The offset. Negative indicates an incomplete structure.
 */
int32_t el_struct_offset(struct el_struct_t *vstruct, uint32_t idx)
{
	uint32_t tmp, size = 0;
	struct el_member_t *member;

	for(member = vstruct->member; idx-- > 0; member = member->next) {
		if(member == NULL)
			return -1;

		tmp = el_type_size(member->type);
		if(tmp < 0)
			return -1;

		size += tmp;
	}

	return size;
}

/**
 * Retrieve the member given an index.
 *   @vstruct: The structure.
 *   @idx: The index.
 *   &returns: The member or null.
 */
struct el_member_t *el_struct_get(struct el_struct_t *vstruct, uint32_t idx)
{
	struct el_member_t *member;

	member = vstruct->member;
	while(idx-- > 0) {
		if(member == NULL)
			return NULL;

		member = member->next;
	}

	return member;
}

/**
 * Find the index of a structure member.
 *   @vstruct: The structure.
 *   @id: The identifier.
 *   &returns: The index, or negative if not found.
 */
int32_t el_struct_find(struct el_struct_t *vstruct, const char *id)
{
	int32_t idx = 0;
	struct el_member_t *memb;

	for(memb = vstruct->member; memb != NULL; memb = memb->next) {
		if(strcmp(memb->id, id) == 0)
			return idx;

		idx++;
	}

	return -1;
}


/**
 * Create a member.
 *   @id: The identifier.
 *   @type: The type.
 *   &returns: The member.
 */
struct el_member_t *el_member_new(char *id, struct el_type_t type)
{
	struct el_member_t *member;

	member = malloc(sizeof(struct el_member_t));
	member->id = id;
	member->type = type;
	member->next = NULL;

	return member;
}

/**
 * Copy a member.
 *   @member: The meember.
 *   &returns: The copy.
 */
struct el_member_t *el_member_copy(struct el_member_t *member)
{
	return el_member_new(strdup(member->id), el_type_copy(member->type));
}

/**
 * Close a list of members.
 *   @member: The member list.
 *   &returns: The copied list.
 */
struct el_member_t *el_member_clone(struct el_member_t *member)
{
	struct el_member_t *head, **iter;

	iter = &head;
	while(member != NULL) {
		*iter = el_member_copy(member);
		iter = &(*iter)->next;
		member = member->next;
	}
	*iter = NULL;

	return head;
}

/**
 * Delete a member.
 *   @member: The member.
 */
void el_member_delete(struct el_member_t *member)
{
	el_type_delete(member->type);
	free(member->id);
	free(member);
}

/**
 * Clear a list of members.
 *   @member: The member list.
 */
void el_member_clear(struct el_member_t *member)
{
	struct el_member_t *tmp;

	while(member != NULL) {
		tmp = member;
		member = tmp->next;

		el_member_delete(tmp);
	}
}


/**
 * Create an enumerated type.
 *   @id: Consumed. The identifier.
 *   @name: The name valued list.
 *   &returns: The enumerated type.
 */
struct el_enum_t *el_enum_new(char *id, struct el_repr_t *repr)
{
	struct el_enum_t *venum;

	venum = malloc(sizeof(struct el_enum_t));
	venum->id = id;
	venum->repr = repr;

	return venum;
}

struct el_enum_t *el_enum_copy(const struct el_enum_t *venum)
{
	return el_enum_new(strdup(venum->id), NULL);
}

/**
 * Delete an enumerator type.
 *   @venum: The enumerated type.
 */
void el_enum_delete(struct el_enum_t *venum)
{
	struct el_repr_t *repr;

	while(venum->repr != NULL) {
		venum->repr = (repr = venum->repr)->next;
		el_repr_delete(repr);
	}

	free(venum->id);
	free(venum);
}

struct el_repr_t *el_enum_find(struct el_enum_t *venum, const char *id)
{
	struct el_repr_t *repr;

	for(repr = venum->repr; repr != NULL; repr = repr->next) {
		if(strcmp(repr->id, id) == 0)
			return repr;
	}

	return NULL;
}


/**
 * Create a new representation.
 *   @id: Consumed. The identifier.
 *   @val: The value.
 *   &returns: The representation.
 */
struct el_repr_t *el_repr_new(char *id, int64_t val)
{
	struct el_repr_t *repr;

	repr = malloc(sizeof(struct el_repr_t));
	repr->id = id;
	repr->val = val;
	repr->next = NULL;

	return repr;
}

struct el_repr_t *el_repr_copy(const struct el_repr_t *repr)
{
	return el_repr_new(strdup(repr->id), repr->val);
}

struct el_repr_t *el_repr_clone(const struct el_repr_t *repr)
{
	struct el_repr_t *head, **iter;

	iter = &head;
	while(repr != NULL) {
		*iter = el_repr_copy(repr);
		iter = &(*iter)->next;
		repr = repr->next;
	}
	*iter = NULL;

	return head;
}

/**
 * Delete a representation.
 *   @repr: The representation.
 */
void el_repr_delete(struct el_repr_t *repr)
{
	free(repr->id);
	free(repr);
}


/**
 * Create an error type.
 *   @val: The value type.
 *   @err: The error tye.
 *   &returns: The error type.
 */
struct el_etype_t *el_etype_new(struct el_type_t val, struct el_type_t err)
{
	struct el_etype_t *pair;

	pair = malloc(sizeof(struct el_etype_t));
	*pair = (struct el_etype_t){ val, err };

	return pair;
}

/**
 * Delete an error type.
 *   @etype: The error type.
 */
void el_etype_delete(struct el_etype_t *etype)
{
	el_type_delete(etype->val);
	el_type_delete(etype->err);
	free(etype);
}


/**
 * Create an array type.
 *   @len: The length.
 *   @type: Consumed. The type.
 *   &returns: The array type.
 */
struct el_arr_t *el_arr_new(struct el_type_t type, uint32_t len)
{
	struct el_arr_t *arr;

	arr = malloc(sizeof(struct el_arr_t));
	arr->len = len;
	arr->type = type;

	return arr;
}

void el_arr_delete(struct el_arr_t *arr)
{
	el_type_delete(arr->type);
	free(arr);
}
		

struct el_type_t el_type_err(struct el_type_t val, struct el_type_t err)
{
	struct el_struct_t *vstruct;

	vstruct = el_struct_new(strdup("!e"), NULL);
	vstruct->member = el_member_new(strdup("val"), val);
	vstruct->member->next = el_member_new(strdup("err"), err);

	return el_type_struct(vstruct);
}
