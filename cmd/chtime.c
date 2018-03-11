// chtime --- get hosts current time

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <hosttab.h>
#include <chaos.h>

int
main(int argc, char **argv)
{
	int fd;
	int addr;
	unsigned short high, low;
	long now;

	if (argc == 1) {
		fprintf(stderr, "usage: chtime host...\n");
		exit(1);
	}

	while (--argc) {
		argv++;

		addr = chaos_addr(*argv, 0);
		if (addr == 0) {
			fprintf(stderr, "host %s unknown\n", *argv);
			exit(1);
		}

		fd = chopen(addr, "TIME", 0, 0, 0, 0, 0);
		if (fd < 0) {
			fprintf(stderr, "Host %s (0%o) is not responding\n", *argv, addr);
			continue;
		}

		/* 32 bits of time */
		read(fd, &low, 2);
		read(fd, &high, 2);
		now = ((long)high << 16) + (long)low;

		now -= 60L*60*24*((1970-1900)*365L + 1970/4 - 1900/4);
		printf("%20s (0%o):\t%s", *argv, addr, ctime(&now));
	}

	exit(0);
}
