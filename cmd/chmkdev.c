/* chmkdev -- create special files
 */

#include <stdlib.h>
#include <stdio.h>

#include <chaos.h>

struct node {
	char *dev;
	int maj;
	int min;
} nodes[] = {
	{ CHURFCDEV, CHRMAJOR, CHURFCMIN },
	{ CHAOSDEV, CHRMAJOR, CHAOSMIN },
	{ 0 },
};

int
main (int argc, char **argv)
{
	printf(": Make sure the major devices are right!\n");
	for (struct node *np = nodes; np->dev; np++)
		printf("/bin/mknod %s c %d %d\n", np->dev, np->maj, np->min);
	exit (0);
}
