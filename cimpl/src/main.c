#include "common.h"


#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>


/**
 * Create an context.
 *   &returns: The context.
 */
struct el_ctx_t *el_ctx_new(void)
{
	struct el_ctx_t *ctx;

	ctx = malloc(sizeof(struct el_ctx_t));
	ctx->err = NULL;
	ctx->tail = &ctx->err;

	return ctx;
}

/**
 * Delete a context.
 *   @ctx: The context.
 */
void el_ctx_delete(struct el_ctx_t *ctx)
{
	while(ctx->err != NULL) {
		struct el_error_t *tmp = ctx->err;

		ctx->err = tmp->next;
		free(tmp->msg);
		free(tmp);
	}

	free(ctx);
}


/**
 * Report an error.
 *   @ctx: The context.
 *   @tag: The tag.
 *   @fmt: The printf-style format.
 *   @...: The printf-style arguments.
 */
void el_ctx_error(struct el_ctx_t *ctx, struct el_tag_t *tag, const char *restrict fmt, ...)
{
	struct el_error_t *err;
	va_list args;

	va_start(args, fmt);

	err = malloc(sizeof(struct el_error_t));
	err->tag = tag;
	err->msg = vmprintf(fmt, args);
	err->next = NULL;
	*ctx->tail = err;
	ctx->tail = &err->next;

	va_end(args);

	fprintf(stderr, "%s:%u:%u: %s\n", err->tag->file->buf, err->tag->line+1, err->tag->col+1, err->msg);
	exit(1);
}

void el_ctx_report(struct el_ctx_t *ctx, FILE *file)
{
	struct el_error_t *err;

	for(err = ctx->err; err != NULL; err = err->next) {
		fprintf(stderr, "%s:%u:%u: %s\n", err->tag->file->buf, err->tag->line+1, err->tag->col+1, err->msg);
	}
}


char *rt_path[] = {
	//"../rt/src/io/file.el",
	"../rt/src/os/linux/linux.el",
	"../rt/src/os/linux/io.el",
	NULL
};

char *el_path[] = {
	"../self/src/parse.el",
	NULL
};

char *test_build(char **path)
{
	char **iter;
	struct el_top_t *top;
	struct el_unit_t *unit;

	onerr( el_top_delete(top); );

	top = el_top_new();

	for(iter = rt_path; *iter != NULL; iter++) {
		unit = el_top_unit(top);
		chkret(el_parse_path(unit, *iter));
	}

	for(iter = path; *iter != NULL; iter++) {
		unit = el_top_unit(top);
		chkret(el_parse_path(unit, *iter));
	}

	struct el_ctx_t *ctx;

	ctx = el_ctx_new();
	//el_resolve_top(top, ctx);

	fatal("HI");
	//if(ctx->err == NULL) el_fill_top(top, ctx);
	//if(ctx->err == NULL) el_check_top(top, ctx);
	//if(ctx->err == NULL) el_simpl_top(top, ctx);

	if(ctx->err != NULL) {
		el_ctx_report(ctx, stdout);
		el_ctx_delete(ctx);
		fail("error");
	}
	el_ctx_delete(ctx);

	//el_fix_top(top);
	//el_fill_top(top);
	//chkret(el_llvm_write(top, stdout));
	chkret(el_llvm_save(top, "test.ll"));

	el_top_delete(top);

	return NULL;
}

int runprog(const char *path, char *const argv[], char **out)
{
	int ret, pid, fd[2];

	if(pipe(fd) < 0)
		fatal("Failed call to `pipe`.");

	if((pid = fork()) == 0) {
		close(fd[0]);
		if(out != NULL) {
			dup2(fd[1], STDOUT_FILENO);
			dup2(fd[1], STDERR_FILENO);
		}
		execvp(path, argv);
		exit(0);
	}

	close(fd[1]);
	waitpid(pid, &ret, 0);

	size_t i = 0;
	ssize_t sz;
	char *buf = malloc(1025);

	while((sz = read(fd[0], buf + i, 1024)) > 0) {
		i += sz;
		buf = realloc(buf, i + 1025);
	}

	buf[i] = '\0';
	close(fd[0]);

	if(out != NULL)
		*out = buf;
	else
		free(buf);

	return ret;
}

char *test_exec(char **path, int code)
{
	int ret;

	onerr( );

	chkret(test_build(path));

	ret = runprog("clang", (char *const[]){ "clang", "test.ll", "-o", "test.bin", "-O2", NULL }, NULL);
	if(ret != 0)
		fail("Code generation failed.");

	ret = runprog("./test.bin", (char *const[]){ "./test.bin", NULL }, NULL);
	if(code != WEXITSTATUS(ret))
		fail("Expected exit status %u, got %u.", code, WEXITSTATUS(ret));

	return NULL;
}

/**
 * Run a single test.
 *   @path: THe test path.
 */
void test_run(char *path)
{
	int n;
	FILE *file;
	char *ptr, *msg, *err, line[256], verb[16];
	char *paths[] = { path, NULL };

	file = fopen(path, "r");
	if(file == NULL)
		fprintf(stderr, "Failed to open test '%s'. %s.\n", path, strerror(errno)), abort();

	if(fgets(line, sizeof(line), file) == NULL)
		fprintf(stderr, "Failed to read test '%s'. %s.\n", path, strerror(errno)), abort();

	if((line[0] != '/') || (line[1] != '/'))
		fprintf(stderr, "Invalid test '%s'. Expected '//'.\n", path), abort();

	ptr = line + 2;
	while(isspace(*ptr))
		ptr++;

	if(sscanf(ptr, "%15[a-z]%n", verb, &n) < 1)
		fprintf(stderr, "Invalid format string for test '%s'.\n", path), abort();

	ptr += n;
	while(isspace(*ptr))
		ptr++;

	if(*ptr != '\0') {
		msg = ptr;
		n = strlen(msg);
		while((n > 0) && isspace(msg[n - 1]))
			n--;

		msg[n] = '\0';
	}
	else
		msg = NULL;

	if(strcmp(verb, "skip") == 0)
		err = NULL;
	else if(strcmp(verb, "suc") == 0) {
		err = NULL;//test_build(paths);
		if(err != NULL) {
			fprintf(stderr, "Failed '%s'. Expected success, got '%s'.\n", path, err);
			free(err);
		}
	}
	else if(strcmp(verb, "fail") == 0) {
		err = strdup("");//test_build(paths);
		if(err == NULL)
			fprintf(stderr, "Failed '%s'. Expected failure, got success.\n", path);
		else
			free(err);
	}
	else if(strcmp(verb, "ret") == 0) {
		if(msg == NULL)
			fprintf(stderr, "Missing return value for test '%s'.\n", path), abort();

		fatal("stub");
	}
	else if(strcmp(verb, "exit") == 0) {
		unsigned long v;

		if(msg == NULL)
			fprintf(stderr, "Missing exit status for test '%s'.\n", path), abort();

		errno = 0;
		v = strtoul(msg, NULL, 0);
		if(errno != 0)
			fprintf(stderr, "Invalid exit status for test '%s'.\n", path), abort();

		err = test_exec(paths, v);
		if(err != NULL) {
			fprintf(stderr, "Failed '%s'. %s\n", path, err);
			free(err);
		}
	}

	fclose(file);
}

/**
 * Run all tests.
 */
void test_all(void)
{
	DIR *dir;
	struct stat st;
	struct dirent *ent;

	dir = opendir("test");
	if(dir == NULL)
		fprintf(stderr, "Failed to open 'test'.\n"), abort();

	while((ent = readdir(dir)) != NULL) {
		char path[256 + 32];

		if(ent->d_name[0] == '.')
			continue;

		snprintf(path, sizeof(path), "test/%s", ent->d_name);

		if(stat(path, &st) != 0)
			fprintf(stderr, "Failed to stat.\n"), abort();

		if(S_ISDIR(st.st_mode)) {
			DIR *dir;
			struct dirent *ent;
			char file[512 + 32];

			dir = opendir(path);
			if(dir == NULL)
				fprintf(stderr, "Failed to open '%s'.\n", path), abort();

			while((ent = readdir(dir)) != NULL) {
				if(ent->d_name[0] == '.')
					continue;

				snprintf(file, sizeof(file), "%s/%s", path, ent->d_name);
				test_run(file);

				printf("%s success\n", file);
			}

			closedir(dir);
		}
	}

	closedir(dir);
}


/**
 * List structure.
 *   @arr: The array.
 *   @cnt, max: The count and maximum.
 */
struct cli_lst_t {
	char **arr;
	uint32_t cnt, max;
};

/**
 * Create an empty string list.
 *   &returns: The empty list.
 */
struct cli_lst_t *cli_lst_new(void)
{
	struct cli_lst_t *lst;

	lst = malloc(sizeof(struct cli_lst_t));
	lst->arr = malloc(16 * sizeof(char *));
	lst->cnt = 0;
	lst->max = 16;

	return lst;
}

/**
 * Delete a string list.
 *   @lst: The list.
 */
void cli_lst_delete(struct cli_lst_t *lst)
{
	free(lst->arr);
	free(lst);
}


/**
 * Add a string to the list.
 *   @lst: The list.
 *   @str: The string.
 */
void cli_lst_add(struct cli_lst_t *lst, char *str)
{
	if(lst->cnt >= lst->max)
		lst->arr = realloc(lst->arr, (lst->max *= 2) * sizeof(char *));

	lst->arr[lst->cnt++] = str;
}


/**
 * Report an error.
 *   @fmt: The format string.
 *   @...: The arguments.
 *   &noreturn
 */
void cli_error(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fputc('\n', stderr);
	va_end(args);

	exit(1);
}


/*
 * local declarations
 */
static char *do_build(const char *out, struct cli_lst_t *lst);


/**
 * Main enty point.
 *   @argc: The number of arguments.
 *   @argv: The argument array.
 *   &returns: The exit status.
 */
int main(int argc, char **argv)
{
	//test_all();
	//chkexit(test_exec(el_path, 0));

	uint32_t i, k;
	struct cli_lst_t *in;
	const char *out = NULL;

	in = cli_lst_new();

	i = 1;
	while(i < argc) {
		if(argv[i][0] == '-') {
			k = 1;

			while(true) {
				if(argv[i][k] == 'o') {
					if(argv[i][k+1] == '\0') {
						i++;
						if(argv[i] == NULL)
							cli_error("Missing output after '-o'.");

						out = argv[i];
					}
					else
						out = argv[i] + k + 1;

					i++;
					break;
				}
				else
					cli_error("Unknown flag '-%c'.", argv[i][k]);
			}
		}
		else
			cli_lst_add(in, argv[i++]);
	}

	if(in->cnt == 0)
		cli_error("el: missing input");

	if(out == NULL)
		out = "out.ll";

	chkexit(do_build(out, in));

	cli_lst_delete(in);

	mdbg_assert();

	return 0;

	/*
	struct el_block_t *block;

	block = el_block_new(NULL);
	chkexit(el_parse_path(block, "test.el"));
	//chkexit(el_parse_path(block, "bar.el"));
	chkexit(el_check_top(block));
	el_fix_block(block);
	el_fill_block(block, NULL);
	chkexit(el_llvm_write(block, stdout));
	chkexit(el_llvm_save(block, "test.ll"));
	el_block_delete(block);

	mdbg_assert();

	return 0;
	*/
}


/**
 * Perform a build.
 *   @out: The output
 *   @lst: The path list.
 *   &returns: Error.
 */
static char *do_build(const char *out, struct cli_lst_t *lst)
{
	uint32_t i;
	struct el_top_t *top;
	struct el_unit_t *unit;
	struct el_ctx_t *ctx;

	onerr( el_top_delete(top); el_ctx_delete(ctx); );

	top = el_top_new();
	ctx = el_ctx_new();

	for(i = 0; i < lst->cnt; i++) {
		unit = el_top_unit(top);
		chkret(el_parse_path(unit, lst->arr[i]));
	}

	el_chk_top(top, ctx);
	//if(ctx->err == NULL) el_resolve_top(top, ctx);
	//if(ctx->err == NULL) el_fill_top(top, ctx);
	//if(ctx->err == NULL) el_check_top(top, ctx);
	//if(ctx->err == NULL) el_simpl_top(top, ctx);

	if(ctx->err != NULL) {
		el_ctx_report(ctx, stdout);
		el_ctx_delete(ctx);
		fail("error");
	}

	chkret(el_llvm_save(top, out));

	el_ctx_delete(ctx);
	el_top_delete(top);

	return NULL;
}
