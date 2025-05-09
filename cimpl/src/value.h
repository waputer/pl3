#pragma once

/**
 * Constant enumerator.
 */
enum el_const_e {
	el_const_bool_v,
	el_const_i8_v,
	el_const_u8_v,
	el_const_i16_v,
	el_const_u16_v,
	el_const_i32_v,
	el_const_u32_v,
	el_const_i64_v,
	el_const_u64_v,
	el_const_num_v,
	el_const_ptr_v,
	el_const_str_v,
};

/**
 * Constant union.
 */
union el_const_u {
	bool flag;
	int8_t i8;
	uint8_t u8;
	int16_t i16;
	uint16_t u16;
	int32_t i32;
	uint32_t u32;
	int64_t i64;
	uint64_t u64;
	char *str;
	struct el_num_t *num;
};

/**
 * Constant structure.
 *   @type: The type.
 *   @data: The data.
 *   @dty: Data type.
 */
struct el_const_t {
	enum el_const_e type;
	union el_const_u data;
	struct el_type_t dty;
};

/*
 * constant declarations
 */
struct el_const_t *el_const_new(enum el_const_e type, union el_const_u data);
void el_const_delete(struct el_const_t *vconst);

struct el_const_t *el_const_bool(bool val);
struct el_const_t *el_const_i8(int8_t val);
struct el_const_t *el_const_u8(uint8_t val);
struct el_const_t *el_const_i16(int16_t val);
struct el_const_t *el_const_u16(uint16_t val);
struct el_const_t *el_const_i32(int32_t val);
struct el_const_t *el_const_u32(uint32_t val);
struct el_const_t *el_const_i64(int64_t val);
struct el_const_t *el_const_u64(uint64_t val);
struct el_const_t *el_const_num(struct el_num_t *val);
struct el_const_t *el_const_ptr(struct el_num_t *val, struct el_type_t dty);
struct el_const_t *el_const_str(char *str);

struct el_type_t el_const_dtype(enum el_const_e type);



/**
 * Initializer data union.
 *   @expr: The expression.
 *   @nest: The nested data.
 */
union el_init_u {
	struct el_expr_t *expr;
	struct el_init_t *nest;
};

/**
 * Initializer structure.
 *   @id: The optional identifer.
 *   @nest, cast: The nesting and cast flag.
 *   @idx: The index.
 *   @data: The data.
 *   @next: The next value.
 */
struct el_init_t {
	char *id;

	bool nest, cast;
	int32_t idx;
	union el_init_u data;
	
	struct el_init_t *next;
};


/*
 * value declarations
 */
struct el_value_t el_value_new(struct el_type_t type, union el_value_u data);
struct el_value_t el_value_copy(struct el_value_t value);
void el_value_delete(struct el_value_t value);

void el_value_replace(struct el_value_t *dst, struct el_value_t src);

struct el_value_t el_value_void(void);
struct el_value_t el_value_bool(bool flag);
struct el_value_t el_value_i32(int32_t i32);
struct el_value_t el_value_u32(uint32_t u32);
struct el_value_t el_value_i64(int64_t i64);
struct el_value_t el_value_u64(uint64_t u64);
struct el_value_t el_value_num(struct el_num_t *num);
struct el_value_t el_value_str(char *str);
struct el_value_t el_value_ptr(struct el_ptr_t *ptr, struct el_num_t *num);

/*
 * number declarations
 */
struct el_num_t *el_num_zero(void);
struct el_num_t *el_num_i64(int64_t val);
struct el_num_t *el_num_copy(const struct el_num_t *num);
char *el_num_read(struct el_num_t **num, const char *str);
void el_num_delete(struct el_num_t *num);

/*
 * initailizer declarations
 */
struct el_init_t *el_init_new(bool nest, union el_init_u data, char *id);
struct el_init_t *el_init_copy(const struct el_init_t *init);
void el_init_delete(struct el_init_t *init);

struct el_init_t *el_init_expr(struct el_expr_t *expr, char *id);

struct el_init_t *el_init_clone(struct el_init_t *init);
void el_init_clear(struct el_init_t *init);

struct el_init_t *el_init_get(struct el_init_t *init, uint32_t idx);
uint32_t el_init_len(struct el_init_t *init);


extern struct el_ptr_t el_ptr_void;
extern struct el_ptr_t el_ptr_str;
