
#include "memory.h"

int memory_init()
{
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd < 0) {
		printf("ERROR: Could not open /dev/mem\n");
		close(memfd);
		return -1;
	}
	return 0;
}

void memory_deinit()
{
	close(memfd);
}
