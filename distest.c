#include <stdio.h>
#include <stdint.h>

#include "disasm.h"

int res = 0;
int addr = -1;

uint8_t get_byte(void)
{
	addr++;
	res = getchar();
	return res;
}

int main (int argc, char *argv)
{
	char mnemonic[64];
	while (res != EOF) {
		disasm(addr, get_byte, mnemonic);
		printf("%04x %s\n", addr, mnemonic);
	}
}
