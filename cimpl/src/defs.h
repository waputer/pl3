#pragma once

/*
 * structure prorotypes
 */
struct el_arg_t;
struct el_decl_t;
struct el_block_t;
struct el_stmt_t;
struct el_str_t;
struct el_tag_t;
struct el_top_t;
struct el_tydecl_t;
struct el_unit_t;


/**
 * String structure.
 *   @buf: The buffer.
 *   @refcnt: The reference count.
 */
struct el_str_t {
	char *buf;
	uint32_t refcnt;
};

/**
 * Tag structure.
 *   @file: The file.
 *   @line, col: The line and column.
 */
struct el_tag_t {
	struct el_str_t *file;
	uint32_t line, col;
};

/**
 * Context structure.
 *   @err, tail: The error list and tail.
 */
struct el_ctx_t {
	struct el_error_t *err, **tail;
};

/**
 * Error message structure.
 *   @tag: The tag.
 *   @msg: The message.
 *   @next: The next error.
 */
struct el_error_t {
	struct el_tag_t *tag;
	char *msg;

	struct el_error_t *next;
};


/**
 * Type kind enumerator.
 *   @el_void_v: Void.
 *   @el_i8_v: 8-bit signed integer.
 *   @el_u8_v: 8-bit unsigned integer.
 *   @el_i16_v: 16-bit signed integer.
 *   @el_u16_v: 16-bit unsigned integer.
 *   @el_i32_v: 32-bit signed integer.
 *   @el_u32_v: 32-bit unsigned integer.
 *   @el_i64_v: 64-bit signed integer.
 *   @el_u64_v: 64-bit unsigned integer.
 *   @el_num_v: Number.
 *   @el_str_v: String.
 *   @el_alias_v: Type alias.
 *   @el_ptr_v: Pointer.
 *   @el_func_v: Function.
 *   @el_struct_v: Structure.
 */
enum el_type_e {
	el_void_v,
	el_u8_v,
	el_i8_v,
	el_u16_v,
	el_i16_v,
	el_i32_v,
	el_u32_v,
	el_i64_v,
	el_u64_v,
	el_bool_v,
	el_num_v,
	el_str_v,
	el_alias_v,
	el_ptr_v,
	el_func_v,
	el_struct_v,
	el_union_v,
	el_enum_v,
	el_arr_v,
	el_args_v
};

/**
 * Type data structure.
 *   @alias: Type alias.
 *   @ptr: Pointer type.
 *   @func: Function type.
 *   @vstruct: Structure.
 */
union el_type_u {
	struct el_alias_t *alias;
	struct el_arr_t *arr;
	struct el_ptr_t *ptr;
	struct el_func_t *func;
	struct el_struct_t *vstruct;
	struct el_enum_t *venum;
	struct el_etype_t *etype;
};

/**
 * Type structure.
 *   @kind: The kind of type.
 *   @data: The type data.
 */
struct el_type_t {
	enum el_type_e kind;
	union el_type_u data;
};


#define EL_CONST 0x01

/**
 * Pointer type structure.
 *   @flags: Pointer flags.
 *   @sub: The subtype.
 */
struct el_ptr_t {
	uint32_t flags;
	struct el_type_t sub;
};

/**
 * Array type structure.
 *   @type: The element type.
 *   @len: The length.
 */
struct el_arr_t {
	struct el_type_t type;
	uint32_t len;
};

struct el_etype_t {
	struct el_type_t val, err;
};


typedef char *(*el_check_f)(void *ref, struct el_func_t *func, struct el_arg_t *arg);

typedef void *(*el_copy_f)(void *ref);
typedef void (*el_delete_f)(void *ref);

/**
 * Type check interface.
 *   @proc: The checking function.
 *   @copy: Copy.
 *   @delete: Delete.
 */
struct el_check_i {
	el_check_f proc;
	el_copy_f copy;
	el_delete_f delete;
};

/**
 * Custom type check structure.
 *   @ref: The reference.
 *   @iface: The interaface.
 */
struct el_check_t {
	void *ref;
	const struct el_check_i *iface;
};

/**
 * Parameter type structure.
 *   @id: The identifier.
 *   @type: The type.
 */
struct el_param_t {
	char *id;
	struct el_type_t type;
};

/**
 * Function type structure.
 *   @ret: The return type.
 *   @cnt: The parameter count.
 *   @param: The parameter array.
 *   @var: The variable argument flag.
 *   @check: Custom type checker.
 */
struct el_func_t {
	struct el_type_t ret;

	uint32_t cnt;
	struct el_param_t *param;
	bool var;

	struct el_check_t check;
};


/**
 * Value union.
 *   @i8: 8-bit signed integer.
 *   @i8: 8-bit unsigned integer.
 *   @i16: 16-bit signed integer.
 *   @i16: 16-bit unsigned integer.
 *   @i32: 32-bit signed integer.
 *   @i32: 32-bit unsigned integer.
 *   @i64: 64-bit signed integer.
 *   @i64: 64-bit unsigned integer.
 *   @str: String.
 *   @num: The number.
 */
union el_value_u {
	int32_t i8;
	uint32_t u8;
	int32_t i16;
	uint32_t u16;
	int32_t i32;
	uint32_t u32;
	int64_t i64;
	uint64_t u64;
	bool flag;
	char *str;
	struct el_num_t *num;
};

/**
 * Value structure.
 *   @type: The type.
 *   @data: The data.
 */
struct el_value_t {
	struct el_type_t type;
	union el_value_u data;
};


/**
 * Number structure.
 *   @mpz: The GMP integer.
 */
struct el_num_t {
	mpz_t mpz;
};


/**
 * Structure structure.
 *   @id: The structure name.
 *   @member: The member list.
 */
struct el_struct_t {
	char *id;
	struct el_member_t *member;
};

/**
 * Member structure.
 *   @id: The identifier.
 *   @type: THe type.
 *   @next: The next member.
 */
struct el_member_t {
	char *id;
	struct el_type_t type;
	struct el_member_t *next;
};

/**
 * Enumerated type.
 */
struct el_enum_t {
	char *id;
	struct el_repr_t *repr;
};

/**
 * Named value.
 */
struct el_repr_t {
	char *id;
	int64_t val;

	struct el_repr_t *next;
};


/**
 * Alias structure.
 *   @id: The identifier.
 *   @type: The type.
 */
struct el_alias_t {
	char *id;
	struct el_type_t type;
};
