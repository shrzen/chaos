/*
 * Distribution tape pseudo-server for reading/writing Symbolics software
 * distribution tapes.
 * Usage is: chsymtape [-w] host-name tape-file
 * -w flag means write a tape instead of read
 */
#include <stdio.h>
#include <chaos/user.h>
#include <sys/types.h>
#include <sys/mtio.h>

#define DTAPESIZE 4096
char tapebuf[DTAPESIZE];
int writing;

main(argc, argv)
int argc;
char **argv;
{
	int netfd, tapefd, nread, nwrite, addr, file, block;
	char netname[100];

	if (argc > 1 && strcmp(argv[1], "-w") == 0) {
		writing = 1;
		argc--;
		argv++;
	}
	if (argc != 3) {
		fprintf(stderr, "Usage is: chsymtape [-w] host-name tape-file\n");
		exit(1);
	}
	if ((addr = chaos_addr(argv[1])) == 0) {
		fprintf(stderr, "Unknown host: %s\n", argv[1]);
		exit(1);
	}
	if ((netfd = open(sprintf(netname, "%s/%d/TAPE", CHRFCDEV, addr), !writing)) < 0) {
		fprintf(stderr, "Can't connect to %s (0%o)\n", argv[1], addr);
		exit(1);
	}
	if ((tapefd = open(argv[2], writing)) < 0) {
		fprintf(stderr, "Can't open tapefile: %s\n", argv[2]);
		exit(1);
	}
	file = 0;
	block = 0;
	if (writing) {
		struct mtop mtop;

		mtop.mt_op = MTWEOF;
		mtop.mt_count = 1;
		while ((nread = read(netfd, tapebuf, DTAPESIZE)) >= 0)
			if (nread > 0)
				if (write(tapefd, tapebuf, nwrite = ((nread + 3) & 3)) !=
				    nwrite) {
					fprintf(stderr, "Tape write error!!\n");
					exit(1);
				} else
					block++;
			else if (ioctl(tapefd, MTIOCTOP, &mtop) < 0) {
				fprintf(stderr, "Can't write an EOF mark on tape!!\n");
				exit(1);
			} else {
				file++;
				block = 0;
			}
		if (ioctl(tapefd, MTIOCTOP, &mtop) < 0) {
			fprintf(stderr, "Can't write last EOF mark on tape!!\n");
			exit(1);
		}
	} else {
		while ((nread = read(tapefd, tapebuf, DTAPESIZE)) >= 0)
			if (nread > 0)
				if (write(netfd, tapebuf, nread) != nread) {
					fprintf(stderr,
						"Error writing block %d, file %d to net!\n",
						block, file);
					exit(1);
				} else
					block++;
			else if (ioctl(netfd, CHIOCOWAIT, 1) < 0) {
				fprintf(stderr, "Can't write EOF to net!\n");
				exit(1);
			} else {
				file++;
				if (block == 0)
					break;
				block = 0;
			}
		printf("chsymtape: %d files read from tape\n", file);
	}
}

