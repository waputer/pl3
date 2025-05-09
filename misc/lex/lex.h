#pragma once

/*
 * common header
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


struct lex_parse_t;


#define LEX_START (-1)
#define LEX_ERR   (-2)
#define LEX_EOF   (-3)
#define LEX_SKIP  (-4)
#define LEX_ID    (-0x10000)
#define LEX_NUM   (-0x10001)


/**
 * ID to string apping structure.
 *   @id: The identifier.
 *   @str: The string.
 */
struct lex_map_t {
	int id;
	const char *str;
};

/**
 * Token structure.
 *   @id: The identifier.
 *   @str: The string.
 *   @path: The path.
 *   @line, col: The line and column.
 *   @next: The next token.
 */
struct lex_token_t {
	int32_t id;
	char *str;

	const char *path;
	uint32_t line, col;

	struct lex_token_t *next;
};

/**
 * Lexing rule structure.
 *   @func: The function.
 *   @arg: The argument.
 */
struct lex_rule_t {
	struct lex_token_t *(*func)(struct lex_parse_t *, const void *);
	const void *arg;
};

/**
 * Lexer structure.
 *   @file: The file.
 *   @buf: The text buffer.
 *   @idx, size: The current index and size.
 *   @path: The path.
 *   @line, col: The line and column.
 *   @trim: The always trim option.
 *   @rule: The rules array.
 *   @cnt, max: The count and capacity of the rules.
 *   @token: THe token list.
 */
struct lex_parse_t {
	FILE *file;
	int16_t *buf;
	uint32_t idx, size;

	char *path;
	uint32_t line, col;

	bool trim;

	struct lex_rule_t *rule;
	uint32_t cnt, max;

	struct lex_token_t *token;
};


/**
 * Number type enumerator.
 *   @lex_int_v: Decimal integer.
 *   @lex_hex_v: Hexidecimal integer.
 *   @lex_oct_v: Octal integer.
 *   @lex_bin_v: Binary integer.
 *   @lex_flt_v: Floating-point number.
 */
enum lex_num_e {
	lex_int_v,
	lex_hex_v,
	lex_oct_v,
	lex_bin_v,
	lex_flt_v,
};

/**
 * File location.
 *   @path: The path.
 *   @line, col: The line and column.
 */
struct lex_loc_t {
	const char *path;
	uint32_t line, col;
};


/*
 * parser declarations
 */
struct lex_parse_t *lex_parse_new(FILE *file, char *path, uint32_t init, bool trim);
void lex_parse_delete(struct lex_parse_t *parser);

char *lex_parse_open(struct lex_parse_t **parse, char *path, uint32_t init, bool trim);
void lex_parse_close(struct lex_parse_t *parse);

struct lex_token_t *lex_parse_next(struct lex_parse_t *parse);
struct lex_token_t *lex_parse_token(struct lex_parse_t *parse, int id, uint32_t len);
void lex_parse_resize(struct lex_parse_t *lex, uint32_t size);

char *lex_parse_error(const struct lex_parse_t *restrict parse, const char *restrict fmt, ...) __attribute__ ((format (printf, 2, 3)));

/*
 * rule declarations
 */
void lex_rule_add(struct lex_parse_t *parse, struct lex_token_t *(*func)(struct lex_parse_t *, const void *), const void *arg);
void lex_rule_keyword(struct lex_parse_t *parse, uint32_t cnt, const struct lex_map_t *keyword);
void lex_rule_symbol(struct lex_parse_t *parse, uint32_t cnt, const struct lex_map_t *symbol);
void lex_rule_ident(struct lex_parse_t *parse, int id);
void lex_rule_ident2(struct lex_parse_t *parse, int id);
void lex_rule_quote1(struct lex_parse_t *parse, int id);
void lex_rule_quote2(struct lex_parse_t *parse, int id);
void lex_rule_num(struct lex_parse_t *parse, int id);
void lex_rule_slashslash(struct lex_parse_t *parse, int id);
void lex_rule_slashstar(struct lex_parse_t *parse, int id);

/*
 * string conversion declarations
 */
char *lex_str_i16(const char *str, int base, int16_t *ret);
char *lex_str_i32(const char *str, int base, int32_t *ret);

/*
 * token declarations
 */
struct lex_token_t *lex_token_new(struct lex_parse_t *parse, int32_t id, char *str, uint32_t line, uint32_t col);
void lex_token_delete(struct lex_token_t *token);

char *lex_token_error(const struct lex_token_t *restrict token, const char *restrict fmt, ...);
char *lex_token_verror(const struct lex_token_t *restrict token, const char *restrict fmt, va_list args);

/*
 * convenience functions
 */
char *lex_error(struct lex_parse_t *restrict parse, const char *restrict fmt, ...) __attribute__ ((format (printf, 2, 3)));



/**
 * Retrieve the `i`th character from the parser.
 *   @lex: The lexer.
 *   @i: The index.
 *   &returns: The next input.
 */
static inline int lex_char(struct lex_parse_t *parse, uint32_t i)
{
	if(i >= parse->size)
		lex_parse_resize(parse, 2 * i);

	return parse->buf[(parse->idx + i) % parse->size];
}

/**
 * Read a single character from the parser.
 *   @parser: The parser.
 *   &returns: The read character.
 */
static inline int lex_read(struct lex_parse_t *parse)
{
	int ch;

	ch = parse->buf[parse->idx];
	parse->buf[parse->idx] = fgetc(parse->file);
	parse->idx = (parse->idx + 1) % parse->size;

	if(ch == '\n') {
		parse->line++;
		parse->col = 0;
	}
	else
		parse->col++;

	return ch;
}

/**
 * Skip characters from the lexer.
 *   @parse: The parser.
 *   @cnt: The number of characters to skip.
 */
static inline void lex_skip(struct lex_parse_t *parse, uint32_t cnt)
{
	while(cnt-- > 0)
		lex_read(parse);
}


/**
 * Advance the parser by a single token.
 *   @parse: The parser.
 */
static inline void lex_adv(struct lex_parse_t *parse)
{
	if(parse->token != NULL) {
		struct lex_token_t *tmp;

		tmp = parse->token;
		parse->token = tmp->next;
		lex_token_delete(tmp);
	}
	else
		lex_token_delete(lex_parse_next(parse));
}

/**
 * Retrieve the `i`th token from the parser.
 *   @parse: The parser.
 *   @i: The token index.
 *   &returns: The token.
 */
static inline struct lex_token_t *lex_get(struct lex_parse_t *parse, uint32_t i)
{
	struct lex_token_t **token = &parse->token;

	i++;
	while(true) {
		if(*token == NULL)
			*token = lex_parse_next(parse);

		if(i == 0)
			break;

		i--;
		token = &(*token)->next;
	}

	return *token;
}

/**
 * Retrieve the last token from the parser.
 *   @parse: The parser.
 *   &returns: The token.
 */
static inline struct lex_token_t *lex_last(struct lex_parse_t *parse)
{
	return lex_get(parse, -1);
}

/**
 * Peek at the top token on the parser.
 *   @parse: The parser.
 *   &returns: The token.
 */
static inline struct lex_token_t *lex_peek(struct lex_parse_t *parse)
{
	return lex_get(parse, 0);
}

/**
 * Advance the parser and get the next token.
 *   @parse: The parser.
 *   &returns: The next token.
 */
static inline struct lex_token_t *lex_next(struct lex_parse_t *parse)
{
	lex_adv(parse);

	return lex_peek(parse);
}

/**
 * Try to match the next token.
 *   @parse: The parser.
 *   @id: The identifier.
 *   &returns: The token if matched, null otherwise.
 */
static inline struct lex_token_t *lex_try(struct lex_parse_t *parse, int32_t id)
{
	struct lex_token_t *token;

	token = lex_peek(parse);
	if(token->id != id)
		return NULL;

	lex_adv(parse);

	return token;
}


/**
 * Check if at the end-of-file.
 *   @parse: The parser.
 *   &returns: True if end-of-file.
 */
static inline bool lex_eof(struct lex_parse_t *parse)
{
	return lex_char(parse, 0) == EOF;
}
static inline struct lex_loc_t lex_loc(const struct lex_parse_t *parse)
{
	return (struct lex_loc_t){ parse->path, parse->line, parse->col };
}


/**
 * Check if a character is alphabetical or unicode.
 *   @ch: The character.
 *   &returns: True if alphabetical or unicode.
 */
static inline bool lex_isalpha(int16_t ch)
{
	return ((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')) || (ch >= 128);
}

/**
 * Check if a character is a digit.
 *   @ch: The character.
 *   &returns: True if digit.
 */
static inline bool lex_isdigit(int16_t ch)
{
	return (ch >= '0') && (ch <= '9');
}

/**
 * Check if a character is alphabetical, unicode, or digit.
 *   @ch: The character.
 *   &returns: True if alphabetical, unicode, or digit.
 */
static inline bool lex_isalnum(int16_t ch)
{
	return lex_isalpha(ch) || lex_isdigit(ch);
}

/**
 * Check if a character is any whitespace character.
 *   @ch: The character.
 *   &returns: True if whitespace.
 */
static inline bool lex_isspace(int16_t ch)
{
	return (ch == ' ') || (ch == '\t') || (ch == '\v') || (ch == '\f') || (ch == '\r') || (ch == '\n');
}

/**
 * Check if a character is a valid for an identifier.
 *   @ch: The character.
 *   @dig: Flag whether to include digits.
 *   &returns: True if identifier.
 */
static inline bool lex_isid(int16_t ch, bool dig)
{
	return lex_isalpha(ch) || (ch == '_') || (dig && lex_isdigit(ch));
}

/**
 * Check if a character is any symbol. This excludes brackets, parenthesis, or
 * braces.
 *   @ch: The character.
 *   &returns: True if symbol.
 */
static inline bool lex_issym(int16_t ch)
{
	return (ch == '~') || (ch == '|') || (ch == '&') || (ch == '^') || (ch == '+') || (ch == '-') || (ch == '*') || (ch == '/') || (ch == '.') || (ch == ',') || (ch == '=') || (ch == '>') || (ch == '<') || (ch == '?');
}
