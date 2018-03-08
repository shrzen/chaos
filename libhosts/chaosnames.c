// chaosnames --- ---!!!

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

extern int chaosnames(FILE *fp);

void
usage(void)
{
	fprintf(stderr, "usage: chaosnames [FILE]\n");
	fprintf(stderr, "---!!!\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -h             help message\n");
}

int
main(int argc, char **argv)
{
	int c = 0;
	int errflg = 0;
	FILE *fd = NULL;
	int ret = 0;

	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
		case 'h':
			usage();
			exit(0);
		case '?': default:
			fprintf(stderr, "invalid option -- '-%c'\n", optopt);
			errflg++;
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (errflg) {
		usage();
		exit(1);
	}

	if (argc >= 2) {
		usage();
		exit (1);
	}

	if (argc == 0)
		fd = stdout;
	else {
		fd = fopen(argv[0], "r");
		if (fd == NULL) {
			fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
			exit (1);
		}
	}
	ret = chaosnames(fd);

	exit(ret);
}
