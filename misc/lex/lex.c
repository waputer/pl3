#include "../emech/emech.h"
#include "../mdbg/mdbg.h"
#include "../membuf/membuf.h"

#include "lex.h"

#include <stdarg.h>


static bool isident(char ch)
{
	return lex_isalnum(ch) || ch == '_';
}

/*
 * local declarations
 */
static struct lex_token_t *get_keyword(struct lex_parse_t *parse, const void *arg);
static struct lex_token_t *get_symbol(struct lex_parse_t *parse, const void *arg);
static struct lex_token_t *get_ident(struct lex_parse_t *parse, const void *arg);
static struct lex_token_t *get_ident2(struct lex_parse_t *parse, const void *arg);
static struct lex_token_t *get_quote1(struct lex_parse_t *parse, const void *arg);
static struct lex_token_t *get_quote2(struct lex_parse_t *parse, const void *arg);
static struct lex_token_t *get_num(struct lex_parse_t *parse, const void *arg);
static struct lex_token_t *get_slashslash(struct lex_parse_t *parse, const void *arg);
static struct lex_token_t *get_slashstar(struct lex_parse_t *parse, const void *arg);


/**
 * Create a parser.
 *   @file: The file.
 *   @path: Consumed. The path.
 *   @init: The initial size.
 *   @trim; Always trim option.
 *   &returns: The parser.
 */
struct lex_parse_t *lex_parse_new(FILE *file, char *path, uint32_t init, bool trim)
{
	uint32_t i;
	struct lex_parse_t *parse;

	parse = malloc(sizeof(struct lex_parse_t));
	parse->file = file;
	parse->buf = malloc(2 * init);
	parse->idx = 0;
	parse->size = init;

	parse->path = path;
	parse->line = parse->col = 0;

	parse->trim = trim;

	parse->rule = malloc(16 * sizeof(struct lex_rule_t));
	parse->cnt = 0;
	parse->max = 16;

	parse->token = lex_token_new(parse, LEX_START, strdup(""), 0, 0);

	for(i = 0; i < init; i++)
		parse->buf[i] = fgetc(file);

	return parse;
}

/**
 * Delete a parser.
 *   @parser: The parser.
 */
void lex_parse_delete(struct lex_parse_t *parse)
{
	struct lex_token_t *token;

	while(parse->token != NULL) {
		token = parse->token;
		parse->token = token->next;
		free(token->str);
		free(token);
	}

	free(parse->rule);
	free(parse->buf);
	free(parse->path);
	free(parse);
}


/**
 * Open a parser.
 *   @parse: Out. The opened parser.
 *   @path: Consumed. The path.
 *   @init: The initial size.
 *   @trim: Always trim option.
 *   &returns: Error.
 */
char *lex_parse_open(struct lex_parse_t **parse, char *path, uint32_t init, bool trim)
{
	onerr( free(path); );

	FILE *file;

	file = fopen(path, "r");
	if(file == NULL)
		fail("Cannot open '%s' for reading. %s.", path, strerror(errno));

	*parse = lex_parse_new(file, path, init, trim);
	return NULL;
}

/**
 * Close a parser.
 *   @parser: The parser.
 */
void lex_parse_close(struct lex_parse_t *parse)
{
	fclose(parse->file);
	lex_parse_delete(parse);
}


/**
 * Parse the next token from the input.
 *   @parse: The parser.
 *   &returns: The next token.
 */
struct lex_token_t *lex_parse_next(struct lex_parse_t *parse)
{
	uint32_t i;
	struct lex_token_t *token;

	if(parse->trim) {
		while(lex_isspace(lex_char(parse, 0)))
			lex_read(parse);
	}

	for(i = 0; i < parse->cnt; i++) {
		token = parse->rule[i].func(parse, parse->rule[i].arg);
		if(token != NULL) {
			if(token->id != LEX_SKIP)
				return token;

			lex_token_delete(token);

			return lex_parse_next(parse);//TODO: recursion is probably bad here
		}
	}

	if(lex_char(parse, 0) == EOF)
		return lex_token_new(parse, LEX_EOF, strdup(""), parse->line, parse->col);
	else
		return lex_token_new(parse, LEX_ERR, mprintf("Unexpected input '%c'.", lex_char(parse, 0)), parse->line, parse->col);
}

/**
 * Parse a token from the input stream.
 *   @parse: The parser.
 *   @id: The identifier.
 *   @len: The length.
 *   &returns: The token.
 */
struct lex_token_t *lex_parse_token(struct lex_parse_t *parse, int id, uint32_t len)
{
	char *str;
	uint32_t i, line, col;

	line = parse->line;
	col = parse->col;

	str = malloc(len + 1);
	for(i = 0; i < len; i++)
		str[i] = lex_read(parse);

	str[len] = '\0';

	return lex_token_new(parse, id, str, line, col);
}

/**
 * Resize the lexer buffer.
 *   @parser: The parser.
 *   @size: The new size.
 */
void lex_parse_resize(struct lex_parse_t *parse, uint32_t size)
{
	uint32_t i;
	int16_t *buf;

	buf = malloc(2 * size);

	memcpy(buf, parse->buf + parse->idx, 2 * (parse->size - parse->idx));
	memcpy(buf + parse->size - parse->idx, parse->buf,  2 * parse->idx);

	for(i = parse->size; i < size; i++)
		buf[i] = fgetc(parse->file);

	free(parse->buf);
	parse->idx = 0;
	parse->size = size;
	parse->buf = buf;
}


/**
 * Create an error message on the parser.
 *   @parse: The parser.
 *   @fmt: The printf-style format string.
 *   @...: The printf-style arguments.
 */
char *lex_parse_error(const struct lex_parse_t *restrict parse, const char *restrict fmt, ...)
{
	char *tmp, *err;
	va_list args;

	va_start(args, fmt);
	tmp = vmprintf(fmt, args);
	va_end(args);

	err = mprintf("%s:%u:%u: %s", parse->path, parse->line + 1, parse->col + 1, tmp);
	free(tmp);

	return err;
}


/**
 * Add a rule to the parser.
 *   @parse: The parser.
 *   @func: The function.
 *   @arg: The argument.
 */
void lex_rule_add(struct lex_parse_t *parse, struct lex_token_t *(*func)(struct lex_parse_t *, const void *), const void *arg)
{
	if(parse->cnt == parse->max)
		parse->rule = realloc(parse->rule, (parse->max *= 2) * sizeof(struct lex_rule_t));

	parse->rule[parse->cnt++] = (struct lex_rule_t){ func, arg };
}

/**
 * Add a rule for parsing a set of keywords.
 *   @parse: The parser.
 *   @cnt: The number of keywords.
 *   @keyword: The keywords array.
 */
void lex_rule_keyword(struct lex_parse_t *parse, uint32_t cnt, const struct lex_map_t *keyword)
{
	uint32_t i;

	for(i = 0; i < cnt; i++)
		lex_rule_add(parse, get_keyword, &keyword[i]);
}
static struct lex_token_t *get_keyword(struct lex_parse_t *parse, const void *arg)
{
	uint32_t i;
	const struct lex_map_t *map = arg;
	const char *str = map->str;

	for(i = 0; str[i] != '\0'; i++) {
		if(lex_char(parse, i) != str[i])
			return NULL;
	}

	if((str[i] != '\0') || lex_isid(lex_char(parse, i), true))
		return NULL;

	return lex_parse_token(parse, map->id, i);
}

/**
 * Add a rule for parsing a set of symbols.
 *   @parse: The parser.
 *   @cnt: The number of symbols.
 *   @symbol: The symbols array.
 */
void lex_rule_symbol(struct lex_parse_t *parse, uint32_t cnt, const struct lex_map_t *symbol)
{
	uint32_t i;

	for(i = 0; i < cnt; i++)
		lex_rule_add(parse, get_symbol, &symbol[i]);
}
static struct lex_token_t *get_symbol(struct lex_parse_t *parse, const void *arg)
{
	uint32_t i;
	const struct lex_map_t *map = arg;
	const char *str = map->str;

	for(i = 0; str[i] != '\0'; i++) {
		if(lex_char(parse, i) != str[i])
			return NULL;
	}

	if(str[i] != '\0')
		return NULL;

	return lex_parse_token(parse, map->id, i);
}

/**
 * Add a rule for parsing an identifier.
 *   @parse: The parser.
 *   @id: The identifier.
 */
void lex_rule_ident(struct lex_parse_t *parse, int id)
{
	lex_rule_add(parse, get_ident, (const void *)(intptr_t)id);
}
static struct lex_token_t *get_ident(struct lex_parse_t *parse, const void *arg)
{
	uint32_t i;

	if(!lex_isalpha(lex_char(parse, 0)) && (lex_char(parse, 0) != '_'))
		return NULL;

	i = 1;
	while(lex_isalnum(lex_char(parse, i)) || (lex_char(parse, i) == '_'))
		i++;

	return lex_parse_token(parse, (int)(intptr_t)arg, i);
}

/**
 * Add a rule for parsing an identifier.
 *   @parse: The parser.
 *   @id: The identifier.
 */
void lex_rule_ident2(struct lex_parse_t *parse, int id)
{
	lex_rule_add(parse, get_ident2, (const void *)(intptr_t)id);
}
static struct lex_token_t *get_ident2(struct lex_parse_t *parse, const void *arg)
{
	uint32_t i;

	if(!lex_isalpha(lex_char(parse, 0)) && (lex_char(parse, 0) != '_'))
		return NULL;

	i = 1;
	while(isident(lex_char(parse, i)) || ((lex_char(parse, i) == ':') && isident(lex_char(parse, i+1))))
		i++;

	return lex_parse_token(parse, (int)(intptr_t)arg, i);
}

/**
 * Add a rule for parsing a single-quoted string.
 *   @parse: The parser.
 *   @id: The identifier.
 */
void lex_rule_quote1(struct lex_parse_t *parse, int id)
{
	lex_rule_add(parse, get_quote1, (const void *)(intptr_t)id);
}
static struct lex_token_t *get_quote1(struct lex_parse_t *parse, const void *arg)
{
	uint32_t line, col;
	struct membuf_t buf;

	if(lex_char(parse, 0) != '\'')
		return NULL;

	line = parse->line;
	col = parse->col;
	buf = membuf_init(32);

	lex_read(parse);

	while(true) {
		if(lex_char(parse, 0) == '\\') {
			lex_read(parse);

			switch(lex_char(parse, 0)) {
			case 'n': membuf_addch(&buf, '\n'); break;
			case 't': membuf_addch(&buf, '\t'); break;
			case 'r': membuf_addch(&buf, '\r'); break;
			case 'v': membuf_addch(&buf, '\v'); break;
			case '\'': membuf_addch(&buf, '\''); break;
			case '\\': membuf_addch(&buf, '\\'); break;
			return lex_token_new(parse, LEX_ERR, mprintf("Unknown escape sequence '\%c'.", lex_char(parse, 0)), parse->line, parse->col);
			}

			lex_read(parse);
		}
		else if(lex_char(parse, 0) == '\'')
			break;
		else
			membuf_addch(&buf, lex_read(parse));
	}

	lex_read(parse);
	membuf_addch(&buf, '\0');

	return lex_token_new(parse, (int)(intptr_t)arg, membuf_done(&buf, NULL), line, col);
}

/**
 * Add a rule for parsing a double-quoted string.
 *   @parse: The parser.
 *   @id: The identifier.
 */
void lex_rule_quote2(struct lex_parse_t *parse, int id)
{
	lex_rule_add(parse, get_quote2, (const void *)(intptr_t)id);
}
static struct lex_token_t *get_quote2(struct lex_parse_t *parse, const void *arg)
{
	uint32_t line, col;
	struct membuf_t buf;

	if(lex_char(parse, 0) != '"')
		return NULL;

	line = parse->line;
	col = parse->col;
	buf = membuf_init(32);

	lex_read(parse);

	while(true) {
		if(lex_char(parse, 0) == '\\') {
			lex_read(parse);

			switch(lex_char(parse, 0)) {
			case 'n': membuf_addch(&buf, '\n'); break;
			case 't': membuf_addch(&buf, '\t'); break;
			case 'r': membuf_addch(&buf, '\r'); break;
			case 'v': membuf_addch(&buf, '\v'); break;
			case '"': membuf_addch(&buf, '"'); break;
			case '\\': membuf_addch(&buf, '\\'); break;
			return lex_token_new(parse, LEX_ERR, mprintf("Unknown escape sequence '\%c'.", lex_char(parse, 0)), parse->line, parse->col);
			}

			lex_read(parse);
		}
		else if(lex_char(parse, 0) == '"')
			break;
		else
			membuf_addch(&buf, lex_read(parse));
	}

	lex_read(parse);
	membuf_addch(&buf, '\0');

	return lex_token_new(parse, (int)(intptr_t)arg, membuf_done(&buf, NULL), line, col);
}

/**
 * Add a rule for parsing an inteeger.
 *   @parse: The parser.
 *   @id: The identifier.
 */
void lex_rule_num(struct lex_parse_t *parse, int id)
{
	lex_rule_add(parse, get_num, (const void *)(intptr_t)id);
}
static struct lex_token_t *get_num(struct lex_parse_t *parse, const void *arg)
{
	uint32_t i;

	if(!lex_isdigit(lex_char(parse, 0)))
		return NULL;

	i = 1;
	while(lex_isalnum(lex_char(parse, i)) || (lex_char(parse, i) == '.'))
		i++;

	return lex_parse_token(parse, (int)(intptr_t)arg, i);
}

/**
 * Add a rule for parsing an single line comment with '//'.
 *   @parse: The parser.
 *   @id: The identifier.
 */
void lex_rule_slashslash(struct lex_parse_t *parse, int id)
{
	lex_rule_add(parse, get_slashslash, (const void *)(intptr_t)id);
}
static struct lex_token_t *get_slashslash(struct lex_parse_t *parse, const void *arg)
{
	if((lex_char(parse, 0) == '/') && (lex_char(parse, 1) == '/')) {
		uint32_t i = 2;

		while((lex_char(parse, i) != '\n') && (lex_char(parse, i) != EOF))
			i++;

		if(lex_char(parse, i) == '\n')
			i++;

		return lex_parse_token(parse, (int)(intptr_t)arg, i);
	}
	else
		return NULL;
}

/**
 * Add a rule for parsing an comments surrounded bye '/ *' and '* /'.
 *   @parse: The parser.
 *   @id: The identifier.
 */
void lex_rule_slashstar(struct lex_parse_t *parse, int id)
{
	lex_rule_add(parse, get_slashstar, (const void *)(intptr_t)id);
}
static struct lex_token_t *get_slashstar(struct lex_parse_t *parse, const void *arg)
{
	if((lex_char(parse, 0) == '/') && (lex_char(parse, 1) == '*')) {
		uint32_t i = 2;

		if(lex_char(parse, i) == EOF)
			return lex_token_new(parse, LEX_ERR, mprintf("Unterminated comment. Unexpected end of input."), parse->line, parse->col);

		while((lex_char(parse, i) != '*') || (lex_char(parse, i + 1) != '/'))
			i++;

		return lex_parse_token(parse, (int)(intptr_t)arg, i + 2);
	}
	else
		return NULL;
}


/**
 * Convert a string to a 16-bit signed integer.
 *   @str: The string.
 *   @base: The number system base. Set to zero for autodetect.
 *   @ret: Out. The parsing string.
 *   &returns: Error.
 */
char *lex_str_i16(const char *str, int base, int16_t *ret)
{
	long long v;
	char *endptr;

	errno = 0;
	v = strtol(str, &endptr, base);
	if(((v == 0) && (errno != 0)) || (*endptr != '\0'))
		return mprintf("Value '%s' is not a valid integer.", str);
	else if(v > INT16_MAX)
		return mprintf("Value '%s' overflows a 16-bit signed integer.", str);
	else if(v < INT16_MIN)
		return mprintf("Value '%s' underflows a 16-bit signed integer.", str);

	*ret = v;

	return NULL;
}

/**
 * Convert a string to a 32-bit signed integer.
 *   @str: The string.
 *   @base: The number system base. Set to zero for autodetect.
 *   @ret: Out. The parsing string.
 *   &returns: Error.
 */
char *lex_str_i32(const char *str, int base, int32_t *ret)
{
	long long v;
	char *endptr;

	errno = 0;
	v = strtol(str, &endptr, base);
	if(((v == 0) && (errno != 0)) || (*endptr != '\0'))
		return mprintf("Value '%s' is not a valid integer.", str);
	else if(v > INT32_MAX)
		return mprintf("Value '%s' overflows a 32-bit signed integer.", str);
	else if(v < INT32_MIN)
		return mprintf("Value '%s' underflows a 32-bit signed integer.", str);

	*ret = v;

	return NULL;
}


/**
 * Create a token.
 *   @parse: The parser, used for line info.
 *   @id: The token ID.
 *   @str: Consumed. The token string.
 *   @line: The line.
 *   @col: The columne.
 *   &returns: The token.
 */
struct lex_token_t *lex_token_new(struct lex_parse_t *parse, int32_t id, char *str, uint32_t line, uint32_t col)
{
	struct lex_token_t *token;

	token = malloc(sizeof(struct lex_token_t));
	token->path = parse->path;
	token->line = line;
	token->col = col;
	token->id = id;
	token->str = str;
	token->next = NULL;

	return token;
}

/**
 * Delete a token.
 *   @token: The token.
 */
void lex_token_delete(struct lex_token_t *token)
{
	free(token->str);
	free(token);
}


/**
 * Create an error message on the parser.
 *   @parse: The parser.
 *   @fmt: The printf-style format string.
 *   @...: The printf-style arguments.
 */
char *lex_token_error(const struct lex_token_t *restrict token, const char *restrict fmt, ...)
{
	char *err;
	va_list args;

	va_start(args, fmt);
	err = lex_token_verror(token, fmt, args);
	va_end(args);

	return err;
}

/**
 * Create an error message on the parser.
 *   @parse: The parser.
 *   @fmt: The printf-style format string.
 *   @...: The printf-style arguments.
 */
char *lex_token_verror(const struct lex_token_t *restrict token, const char *restrict fmt, va_list args)
{
	char *tmp, *err;

	tmp = vmprintf(fmt, args);
	err = mprintf("%s:%u:%u: %s", token->path, token->line + 1, token->col + 1, tmp);
	free(tmp);

	return err;
}


char *lex_error(struct lex_parse_t *restrict parse, const char *restrict fmt, ...)
{
	char *err;
	va_list args;

	va_start(args, fmt);
	err = lex_token_verror(lex_peek(parse), fmt, args);
	va_end(args);

	return err;
}
