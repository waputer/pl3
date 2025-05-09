#include <stdint.h>

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
