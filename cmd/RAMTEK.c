#include <stdio.h>
#include <chaos.h>

int
main (int argc, char **argv)
{
	int c;

	if (freopen("/dev/ramtek", "w", stdout) == NULL)
		chreject(0, "Can't open ramtek printer device");
	else {
		while ((c = getchar()) != EOF)
			putchar(c);
	}
}
