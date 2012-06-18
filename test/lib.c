#include "lib.h"

#include <stdio.h>
#include <ctype.h>

void hexdump(char *buf, int buf_len)
{
	int start_offset;
	
	for (start_offset = 0; start_offset < buf_len; start_offset += 16) {
		printf("%08x  ", start_offset);
		
		int i;
		for (i = 0; i < 16 && start_offset+i < buf_len; i++) {
			printf("%02x ", buf[start_offset+i]);
			if (i == 8)
				printf(" ");
		}

		printf("  |");

		for (i = 0; i < 16 && start_offset+i < buf_len; i++) {
			char ch = buf[start_offset+i];
			if (isprint(ch))
				putchar(ch);
			else
				putchar('.');
		}		

		printf("|\n");
	}

	printf("%08x\n", start_offset);
}
