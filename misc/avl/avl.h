#pragma once

/*
 * common headers
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>


/**
 * Retrive the parent data structure from a member pointer.
 *   @ptr: The member poitner.
 *   @type: The parent type.
 *   @member: The member identifier.
 *   &returns: The parent pointer.
 */
#define avl_getparent(ptr, type, member) ((type *)((void *)(ptr) - offsetof(type, member)))


/**
 * Delete function.
 *   @ptr: The pointer.
 */
typedef void (*avl_delete_f)(void *ptr);

/**
 * Comparison function.
 *   @lhs: The left-hand side.
 *   @rhs: The right-hand side.
 *   &returns: Their order.
 */
typedef int (*avl_compare_f)(const void *lhs, const void *rhs);


/**
 * AVL tree root structure.
 *   @count: The number of nodes.
 *   @node: The root node.
 *   @compare: The comparison callback.
 */
struct avl_root_t {
	uint64_t count;
	struct avl_node_t *node;

	avl_compare_f compare;
};

/**
 * AVL tree node storage.
 *   @bal: The balance factor.
 *   @ref: The reference.
 *   @parent, left, right: The parent, left, and right children.
 */
struct avl_node_t {
	int_fast8_t bal;

	const void *ref;
	struct avl_node_t *parent, *left, *right;
};

/*
 * avl root declarations
 */
struct avl_root_t avl_root_init(avl_compare_f compare);
void avl_root_destroy(struct avl_root_t *root, ssize_t offset, avl_delete_f delete);

struct avl_node_t *avl_root_first(struct avl_root_t *root);
struct avl_node_t *avl_root_last(struct avl_root_t *root);

struct avl_node_t *avl_root_get(struct avl_root_t *root, const void *ref);
void avl_root_add(struct avl_root_t *root, struct avl_node_t *add);
struct avl_node_t *avl_root_remove(struct avl_root_t *root, const void *ref);

/*
 * avl node declarations
 */
void avl_node_init(struct avl_node_t *node, const void *ref);

struct avl_node_t *avl_node_prev(struct avl_node_t *node);
struct avl_node_t *avl_node_next(struct avl_node_t *node);

/*
 * comparison declarations
 */
int avl_cmp_ptr(const void *left, const void *right);
int avl_cmp_str(const void *left, const void *right);

int avl_cmp_int(const void *left, const void *right);
int avl_cmp_uint(const void *left, const void *right);
int avl_cmp_char(const void *left, const void *right);
int avl_cmp_short(const void *left, const void *right);
int avl_cmp_long(const void *left, const void *right);
int avl_cmp_size(const void *left, const void *right);
int avl_cmp_i8(const void *left, const void *right);
int avl_cmp_u8(const void *left, const void *right);
int avl_cmp_i16(const void *left, const void *right);
int avl_cmp_u16(const void *left, const void *right);
int avl_cmp_i32(const void *left, const void *right);
int avl_cmp_u32(const void *left, const void *right);
int avl_cmp_i64(const void *left, const void *right);
int avl_cmp_u64(const void *left, const void *right);
int avl_cmp_float(const void *left, const void *right);
int avl_cmp_double(const void *left, const void *right);
