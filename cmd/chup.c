// chup --- get hosts uptime

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <hosttab.h>
#include <chaos.h>

int
main(int argc, char **argv)
{
	int fd;
	int addr;
	unsigned high, low;
	long uptime;
	int days, hours, minutes, seconds;

	if (argc == 1) {
		fprintf(stderr, "usage: chup HOST...\n");
		exit(1);
	}

	while (--argc) {
		argv++;

		addr = chaos_addr(*argv, 0);
		if (addr == 0) {
			fprintf(stderr, "host %s unknown\n", *argv);
			exit(1);
		}

		fd = chopen(addr, "UPTIME", 0, 0, 0, 0, 0);
		if (fd < 0) {
			fprintf(stderr, "Host %s (0%o) is not responding\n", *argv, addr);
			continue;
		}

		/* 32 bits of uptime */
		read(fd, &low, 2);
		read(fd, &high, 2);
		uptime = ((long)high << 16) + (long)low;

		days = uptime / (60L*60L*60L*24L);
		uptime %= (60L*60L*60L*24L);

		hours = uptime / (60L*60L*60L);
		uptime %= (60L*60L*60L);

		minutes = uptime / (60L*60L);
		uptime %= (60L*60L);

		seconds = (uptime + 30L) / 60L;

		printf("%s (0%o) has been up for %d day%s, ",
		       *argv, addr, days, (days != 1 ? "s" : ""));
		printf("%d hour%s, %d minute%s, %d second%s\n",
		       hours, (hours != 1 ? "s" : ""),
		       minutes, (minutes != 1 ? "s" : ""),
		       seconds, (seconds != 1 ? "s" : ""));
	}

	exit(0);
}
