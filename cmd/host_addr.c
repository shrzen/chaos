#include <chaos.h>
#include <hosttab.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	if (argc <= 1) { fprintf(stderr, "host_name host\n"); exit(1); }
	printf("%o\n", ho_addr(argv[1]));
	exit(0);
}
