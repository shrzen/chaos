#include <stdio.h>
#include <hosttab.h>

main (argc, argv)
char **argv;
{
	int	fd;
	int	addr;
	unsigned high, low;
	long	now;
	int	days, hours, minutes, seconds;
 	char contact[50];

	if (argc == 1) {
		fprintf (stderr, "Usage: chtime host [ host ] ...\n");
		exit (1);
	}

	while (--argc) {
		argv++;
		sprintf(contact, "/dev/chaos/%d/TIME", addr = chaos_addr(*argv, 0));
		if ((fd = open (contact, 0)) < 0 ) {
			fprintf (stderr, "Host %s (0%o) is not responding\n", *argv, addr);
			continue;
		}

		/* 32 bits of time */

		read (fd, &low, 2);
		read (fd, &high, 2);
		now = ((long)high << 16) + (long)low;
		now -= 60L*60*24*((1970-1900)*365L + 1970/4 - 1900/4);
		printf("%20s (0%o):\t%s", *argv, addr, ctime(&now));
	}
}
