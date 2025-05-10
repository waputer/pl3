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
