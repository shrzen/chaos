#include <stdio.h>
#include <chaos/dev.h>
#include <chaos/user.h>
#define CHRMAJOR	25
#define CHCMAJOR	26
struct	node	{
	char	*dev;
	int	maj;
	int	min;
} nodes[] = {
/*	dev		maj		min	*/
{	CHURFCDEV,	CHRMAJOR,	CHURFCMIN,		},
{	CHLISTDEV,	CHRMAJOR,	CHLISTMIN,		},
{	CHCPRODEV,	CHCMAJOR,	CHBADCHAN,		},
{	CHRFCDEV,	CHRMAJOR,	CHRFCMIN,		},
{	CHRFCADEV,	CHRMAJOR,	CHRFCAMIN,		},
{	0,	},
};
main()
{
	register struct node *np;

	printf(": Make sure the major devices are right!\n");
	for (np = nodes; np->dev; np++)
		printf("/etc/mknod %s c %d %d\n", np->dev, np->maj, np->min);
	fprintf(stderr, "!!! Make sure the major devices are correct\n");
}
