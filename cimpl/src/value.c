#include "common.h"


/**
 * Create a constant.
 *   @type: The type.
 *   @data: The data.
 *   &returns: The constant.
 */
struct el_const_t *el_const_new(enum el_const_e type, union el_const_u data)
{
	struct el_const_t *vconst;

	vconst = malloc(sizeof(struct el_const_t));
	vconst->type = type;
	vconst->data = data;
	vconst->dty = el_type_copy(el_const_dtype(type));

	return vconst;
}

/**
 * Delete a constant.
 *   @vconst: The constant.
 */
void el_const_delete(struct el_const_t *vconst)
{
	switch(vconst->type) {
	case el_const_bool_v: break;
	case el_const_i8_v: break;
	case el_const_u8_v: break;
	case el_const_i16_v: break;
	case el_const_u16_v: break;
	case el_const_i32_v: break;
	case el_const_u32_v: break;
	case el_const_i64_v: break;
	case el_const_u64_v: break;
	case el_const_num_v: el_num_delete(vconst->data.num); break;
	case el_const_str_v: free(vconst->data.str); break;
	case el_const_ptr_v: el_num_delete(vconst->data.num); break;
	}

	el_type_delete(vconst->dty);
	free(vconst);
}


/**
 * Create an boolean.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_bool(bool val)
{
	return el_const_new(el_const_bool_v, (union el_const_u){ .flag = val });
}

/**
 * Create an 8-bit signed integer.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_i8(int8_t val)
{
	return el_const_new(el_const_i8_v, (union el_const_u){ .i8 = val });
}

/**
 * Create an 8-bit unsigned integer.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_u8(uint8_t val)
{
	return el_const_new(el_const_u8_v, (union el_const_u){ .u8 = val });
}

/**
 * Create an 16-bit signed integer.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_i16(int16_t val)
{
	return el_const_new(el_const_i16_v, (union el_const_u){ .i16 = val });
}

/**
 * Create an 16-bit unsigned integer.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_u16(uint16_t val)
{
	return el_const_new(el_const_u16_v, (union el_const_u){ .u16 = val });
}

/**
 * Create an 32-bit signed integer.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_i32(int32_t val)
{
	return el_const_new(el_const_i32_v, (union el_const_u){ .i32 = val });
}

/**
 * Create an 32-bit unsigned integer.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_u32(uint32_t val)
{
	return el_const_new(el_const_u32_v, (union el_const_u){ .u32 = val });
}

/**
 * Create an 64-bit signed integer.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_i64(int64_t val)
{
	return el_const_new(el_const_i64_v, (union el_const_u){ .i64 = val });
}

/**
 * Create an 64-bit unsigned integer.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_u64(uint64_t val)
{
	return el_const_new(el_const_u64_v, (union el_const_u){ .u64 = val });
}

/**
 * Create an number integer.
 *   @val: The value.
 *   &returns: The constant.
 */
struct el_const_t *el_const_num(struct el_num_t *val)
{
	return el_const_new(el_const_num_v, (union el_const_u){ .num = val });
}

struct el_const_t *el_const_ptr(struct el_num_t *val, struct el_type_t dty)
{
	struct el_const_t *vconst;

	vconst = el_const_new(el_const_ptr_v, (union el_const_u){ .num = val });
	el_type_replace(&vconst->dty, dty);

	return vconst;
}

/**
 * Create a string.
 *   @str: The string.
 *   &returns: The constant.
 */
struct el_const_t *el_const_str(char *str)
{
	return el_const_new(el_const_str_v, (union el_const_u){ .str = str });
}


//struct el_type_t el_type_void0 = { el_void_v };
struct el_ptr_t el_ptr_void = { 0, { el_void_v } };
struct el_ptr_t el_ptr_str = { EL_CONST, { el_u8_v } };
//struct el_type_t el_type_ptr0 = { el_ptr_v , &el_type_void0 };
//struct el_type_t el_type_ptr0 = { el_ptr_v , &el_type_void0 };

/**
 * Create the type associated with a constant.
 *   @type: The constant type.
 *   &returns: The data type.
 */
struct el_type_t el_const_dtype(enum el_const_e type)
{
	switch(type) {
	case el_const_bool_v: return el_type_bool();
	case el_const_i8_v: return el_type_i8();
	case el_const_u8_v: return el_type_u8();
	case el_const_i16_v: return el_type_i16();
	case el_const_u16_v: return el_type_u16();
	case el_const_i32_v: return el_type_i32();
	case el_const_u32_v: return el_type_u32();
	case el_const_i64_v: return el_type_i64();
	case el_const_u64_v: return el_type_u64();
	case el_const_num_v: return el_type_num();
	case el_const_str_v: return el_type_ptr(&el_ptr_str);
	//case el_const_ptr_v: return el_type_ptr(el_ptr_new(0, el_type_void()));
	case el_const_ptr_v: return el_type_ptr(&el_ptr_void);
	}

	unreachable();
}


/**
 * Create a value.
 *   @type: Consumed. The type.
 *   @data: Consumed. The data.
 *   &returns: The value.
 */
struct el_value_t el_value_new(struct el_type_t type, union el_value_u data)
{
	return (struct el_value_t){ type, data };
}

/**
 * Copy a value.
 *   @value: The value.
 *   &returns: The copy.
 */
struct el_value_t el_value_copy(struct el_value_t value)
{
	switch(value.type.kind) {
	case el_void_v:
	case el_i8_v:
	case el_u8_v:
	case el_i16_v:
	case el_u16_v:
	case el_i32_v:
	case el_u32_v:
		fatal("stub");

	case el_i64_v:
	case el_u64_v:
		fatal("stub");

	case el_bool_v:
		fatal("stub");

	case el_num_v:
		fatal("stub");

	case el_str_v:
		fatal("stub");

	case el_ptr_v:
	case el_arr_v:
		fatal("stub");

	case el_alias_v:
		fatal("stub");

	case el_func_v:
		fatal("stub");

	case el_struct_v:
		fatal("stub");

	case el_args_v:
	case el_enum_v:
	case el_union_v:
		fatal("stub");
	}

	unreachable();
}

/**
 * Delete a value.
 *   @value: The value.
 */
void el_value_delete(struct el_value_t value)
{
	switch(value.type.kind) {
	case el_void_v:
	case el_i8_v:
	case el_u8_v:
	case el_i16_v:
	case el_u16_v:
	case el_i32_v:
	case el_u32_v:
	case el_i64_v:
	case el_u64_v:
	case el_bool_v:
		break;

	case el_num_v:
	case el_ptr_v:
		el_num_delete(value.data.num);
		break;

	case el_arr_v:
		fatal("stub");

	case el_str_v:
		free(value.data.str);
		break;

	case el_alias_v:
		fatal("stub");

	case el_func_v:
		fatal("stub");

	case el_struct_v:
		fatal("stub");

	case el_args_v:
	case el_enum_v:
	case el_union_v:
		fatal("stub");
	}

	el_type_delete(value.type);
}


/**
 * Replace a destination value.
 *   @dst: The destination.
 *   @src: The source.
 */
void el_value_replace(struct el_value_t *dst, struct el_value_t src)
{
	el_value_delete(*dst);
	*dst = src;
}


/**
 * Create a void value.
 *   &returns: The value.
 */
struct el_value_t el_value_void(void)
{
	return el_value_new(el_type_void(), (union el_value_u){ });
}

/**
 * Create a boolean value.
 *   @flag: The flag.
 *   &returns: The value.
 */
struct el_value_t el_value_bool(bool flag)
{
	return el_value_new(el_type_bool(), (union el_value_u){ .flag = flag });
}

/**
 * Create a signed 32-bit integer value.
 *   @i32: The value.
 *   &returns: The value.
 */
struct el_value_t el_value_i32(int32_t i32)
{
	return el_value_new(el_type_i32(), (union el_value_u){ .i32 = i32 });
}

/**
 * Create an unsigned 32-bit integer value.
 *   @u32: The value.
 *   &returns: The value.
 */
struct el_value_t el_value_u32(uint32_t u32)
{
	return el_value_new(el_type_u32(), (union el_value_u){ .u32 = u32 });
}

/**
 * Create a signed 64-bit integer value.
 *   @i64: The value.
 *   &returns: The value.
 */
struct el_value_t el_value_i64(int64_t i64)
{
	return el_value_new(el_type_i64(), (union el_value_u){ .i64 = i64 });
}

/**
 * Create an unsigned 64-bit integer value.
 *   @u64: The value.
 *   &returns: The value.
 */
struct el_value_t el_value_u64(uint64_t u64)
{
	return el_value_new(el_type_u64(), (union el_value_u){ .u64 = u64 });
}

/**
 * Create a number value.
 *   @num: Consumed. The number.
 *   &returns: The value.
 */
struct el_value_t el_value_num(struct el_num_t *num)
{
	return el_value_new(el_type_num(), (union el_value_u){ .num = num });
}

/**
 * Create a string value.
 *   @str: Consumed. The string.
 *   &returns: The value.
 */
struct el_value_t el_value_str(char *str)
{
	return el_value_new(el_type_str(), (union el_value_u){ .str = str });
}

/**
 * Create a pointer value.
 *   @ptr: Consumed. The pointer type.
 *   @num: Consumed. The number.
 *   &returns: The value.
 */
struct el_value_t el_value_ptr(struct el_ptr_t *ptr, struct el_num_t *num)
{
	return el_value_new(el_type_ptr(ptr), (union el_value_u){ .num = num });
}


/**
 * Create a number of zero.
 *   &returns: The number.
 */
struct el_num_t *el_num_zero(void)
{
	struct el_num_t *num;
	
	num = malloc(sizeof(struct el_num_t));
	mpz_init(num->mpz);
	mpz_set_si(num->mpz, 0);

	return num;
}

struct el_num_t *el_num_i64(int64_t val)
{
	struct el_num_t *num;
	
	num = malloc(sizeof(struct el_num_t));
	mpz_init(num->mpz);
	mpz_set_si(num->mpz, val);

	return num;
}

struct el_num_t *el_num_copy(const struct el_num_t *num)
{
	struct el_num_t *copy;
	
	copy = malloc(sizeof(struct el_num_t));
	mpz_init(copy->mpz);
	mpz_set(copy->mpz, num->mpz);

	return copy;
}

/**
 * Read a number from a string.
 *   @num: Out. The number.
 *   @str: The string.
 *   &returns: The number.
 */
char *el_num_read(struct el_num_t **num, const char *str)
{
	onerr( mpz_clear((*num)->mpz); free(*num); );

	*num = malloc(sizeof(struct el_num_t));
	mpz_init((*num)->mpz);
	if(mpz_set_str((*num)->mpz, str, 0) < 0)
		fail("Invalid number '%s'.", str);

	return NULL;
}

/**
 * Delete a number.
 *   @num: The number.
 */
void el_num_delete(struct el_num_t *num)
{
	mpz_clear(num->mpz);
	free(num);
}


/**
 * Create an initializer.
 *   @nest: The nest flag.
 *   @data: The data.
 *   @id: Optional. Consumed. The identifier.
 *   &returns: The initializer.
 */
struct el_init_t *el_init_new(bool nest, union el_init_u data, char *id)
{
	struct el_init_t *init;

	init = malloc(sizeof(struct el_init_t));
	init->id = id;
	init->idx = -1;
	init->nest = nest;
	init->data = data;
	init->next = NULL;

	return init;
}

/**
 * Copy an initializer.
 *   @init: The initializer.
 *   &returns: The copy.
struct el_init_t *el_init_copy(const struct el_init_t *init)
{
	union el_init_u data;

	if(init->nest)
		data.nest = el_init_clone(init->data.nest);
	else
		data.expr = el_expr_copy(init->data.expr);

	return el_init_new(init->nest, data, init->id ? strdup(init->id) : NULL);
}
 */

/**
 * Delete an initializer.
 *   @init: The initialzer.
 */
void el_init_delete(struct el_init_t *init)
{
	if(init->id != NULL)
		free(init->id);

	if(init->nest)
		el_init_clear(init->data.nest);
	else
		el_expr_delete(init->data.expr);

	free(init);
}


/**
 * Create an expression initializer.
 *   @expr: The expression.
 *   @id: The identifier.
 *   &returns: The initializer.
 */
struct el_init_t *el_init_expr(struct el_expr_t *expr, char *id)
{
	return el_init_new(false, (union el_init_u){ .expr = expr }, id);
}


/**
 * Copy an initializer list.
 *   @init: The initializer list.
 *   &returns: The copied list.
struct el_init_t *el_init_clone(struct el_init_t *init)
{
	struct el_init_t *head = NULL, **iter = &head;

	while(init != NULL) {
		*iter = el_init_copy(init);
		iter = &(*iter)->next;
		init = init->next;
	}

	return head;
}
 */

/**
 * Clear an initializer list.
 *   @init: The initializer.
 */
void el_init_clear(struct el_init_t *init)
{
	struct el_init_t *tmp;

	while(init != NULL) {
		tmp = init;
		init = tmp->next;

		el_init_delete(tmp);
	}
}

struct el_init_t *el_init_get(struct el_init_t *init, uint32_t idx)
{
	while(init != NULL) {
		if(init->idx == idx)
			return init;

		init = init->next;
	}

	return NULL;
}

uint32_t el_init_len(struct el_init_t *init)
{
	uint32_t len;

	for(len = 0; init != NULL; len++)
		init = init->next;

	return len;
}
