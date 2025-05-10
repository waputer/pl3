#define _GNU_SOURCE
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/syscall.h>

extern void os_exit(uint32_t code) asm("\"os:exit\"");
extern void run(char **) asm("\"run\"");

int main(int argc, char **argv)
{
	run(argv);
	os_exit(0);

	return 0;
}

void bi_btrace(void **ptr, uint32_t size) asm("\"bi:btrace\"");
void bi_btrace(void **ptr, uint32_t size)
{
}

double str_get_fp64(const char *str) {
	double t = 0;

	while((*str >= '0') && (*str <= '9')) {
		t = 10.0 * t + *str - '0';
		str++;
	}

	if(*str == '.') {
		double r = 1;

		str++;

		while((*str >= '0') && (*str <= '9')) {
			r /= 10.0;
			t += r * (*str - '0');
			str++;
		}
	}

	if(*str == 'e') {
		int32_t e = 0;
		bool neg = false;

		str++;
		if(*str == '-') {
			str++;
			neg = true;
		}

		while((*str >= '0') && (*str <= '9')) {
			e = 10 * e + *str - '0';
			str++;
		}

		if(neg) {
			while(e-- > 0)
				t /= 10;
		}
		else {
			while(e-- > 0)
				t *= 10;
		}
	}

	return t;
};

float str_get_fp32(const char *str) {
	float t = 0;

	while((*str >= '0') && (*str <= '9')) {
		t = 10.0 * t + *str - '0';
		str++;
	}

	if(*str == '.') {
		float r = 1;

		str++;

		while((*str >= '0') && (*str <= '9')) {
			r /= 10.0;
			t += r * (*str - '0');
			str++;
		}
	}

	if(*str == 'e') {
		int32_t e = 0;
		bool neg = false;

		str++;
		if(*str == '-') {
			str++;
			neg = true;
		}

		while((*str >= '0') && (*str <= '9')) {
			e = 10 * e + *str - '0';
			str++;
		}

		if(neg) {
			while(e-- > 0)
				t /= 10;
		}
		else {
			while(e-- > 0)
				t *= 10;
		}
	}

	return t;
};


int32_t str_dig_fp32(char *buf, float val) {
	uint32_t dig;
	int32_t e = 0;

	while(val > 10.0){
		e++;
		val /= 10.0;
	}

	while(val < 1.0){
		e--;
		val *= 10.0;
	}

	int32_t i = 0;
	while(val > 0.0) {
		uint32_t dig = val;

		*buf++ = (char)(dig + '0');
		val = 10.0 * (val - (float)dig);
	}

	*buf = '\0';
	return e + 1;
}

int32_t str_dig_fp64(char *buf, double val) {
	uint32_t dig;
	int32_t e = 0;

	while(val > 10.0){
		e++;
		val /= 10.0;
	}

	while(val < 1.0){
		e--;
		val *= 10.0;
	}

	int32_t i = 0;
	while(val > 0.0) {
		uint32_t dig = val;

		*buf++ = (char)(dig + '0');
		val = 10.0 * (val - (double)dig);
	}

	*buf = '\0';
	return e + 1;
}

float cast_fp64tofp32(double val) { return (float)val; }
double cast_fp32tofp64(float val) { return (double)val; }
float cast_i64tofp32(int64_t val) { return (float)val; }
double cast_i64tofp64(int64_t val) { return (double)val; }

int32_t bi_cmp_xchg32(int32_t *ptr, int32_t old, int32_t new) {
	return __sync_val_compare_and_swap(ptr, old, new);
}

static inline int64_t syscall4(int64_t rax, int64_t rdi, int64_t rsi, int64_t rdx, int64_t rcx, int64_t r8)
{
	int64_t ret;
	asm volatile("movq %6,%%r8 ; movq %5,%%rcx ; movq %4,%%rdx ; movq %3,%%rsi ; movq %2,%%rdi ; movq %1,%%rax ; syscall ; movq %%rax,%0" : "=r"(ret) : "rmi"(rax), "rmi"(rdi), "m"(rsi), "rmi"(rdx), "rmi"(rcx), "rmi"(r8));
	return ret;
}
int32_t os_thread(uint32_t stack, void (*func)(void *, void *, uint32_t), void *arg)
{
	int64_t pid;

	void *mem;
	mem = mmap(0, stack, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	pid = syscall4(SYS_clone, CLONE_VM | CLONE_FS | CLONE_SIGHAND | CLONE_THREAD, (uint64_t)(mem + stack), 0, 0, 0);
	if(pid == 0) { func(arg, mem, stack); for(;;); }
	return pid;
}

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
void _getarg(va_list *list, uint32_t sz, void *dst)
{
	sz /= 8;
	switch(sz) {
	case 1:
	case 2:
	case 3:
	case 4: {
		uint32_t v = va_arg(*list, uint32_t);
		memcpy(dst, &v, sz);
	} break;

	case 8: {
		uint64_t v = va_arg(*list, uint64_t);
		memcpy(dst, &v, sz);
	} break;

	case 16: {
		//struct { uint64_t val[2]; } data;
		//data = va_arg(*list, typeof(data));
		uint64_t data[2];
		data[0] = va_arg(*list, uint64_t);
		data[1] = va_arg(*list, uint64_t);
		memcpy(dst, &data, 16);
	} break;

	default:
		fprintf(stderr, "Unsupported getarg size (%u)\n", sz);
		os_exit(1);
	}
}
