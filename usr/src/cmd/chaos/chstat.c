#include <stdio.h>
#include <nlist.h>
#include <sys/types.h>
#include "../chunix/chconf.h"
#include "../chunix/chsys.h"
#include <chaos/chaos.h>
/*
 * Chaos status display
 */

/*
 * First, the appropriate global symbols.
 */
struct	nlist	namelist[] = {
#define aconntab	0
	{	"_Chconntab"},
#define aclock		1
	{	"_Chclock"},
#define amyaddr		2
	{	"_Chmyaddr"},
#define aroutetab	3
	{	"_Chroutetab"},
#define adr11cxcvr	4
	{	"_dr11cxcvr"},
#define adr11chosts	5
	{	"_dr11chosts"},
#define alsnlist	6
	{	"_Chlsnlist"},
#define ach11xcvr	7
	{	"_ch11xcvr"},
#define arfclist	8
	{	"_Chrfclist"},
#define achhz		9
	{	"_Chhz"},
#define MAXSYMBOLS	10
	{	""},
};
#ifdef vax
char *symfile = "/vmunix";
#else
char *symfile = "/unix";
#endif
char *memfile = "/dev/kmem";
int mem;
struct	{
	caddr_t	kaddr[MAXSYMBOLS];
} info;
struct connection *Chconntab[CHNCONNS];
union {
	char		p_c[CHHEADSIZE + CHMAXDATA];
	struct packet	p_pkt;
} pktbuf;
struct packet	*Chlsnlist;
struct packet	*Chrfclist;
struct chroute Chroutetab[CHNSUBNET];
short Chmyaddr;
unsigned int Chclock;
int Chhz;
#if NCHDR > 0
struct chxcvr dr11cxcvr[NCHDR];
short dr11chosts[NCHDR];
#endif
#if NCHCH > 0
struct chxcvr ch11xcvr[NCHCH];
#endif
char *statename[] = {
	"Closed",
	"Listen",
	"RFCRcd",
	"RFCSnt",
	"Open",
	"Lost",
	"Incomp",
};
#if NCHDR > 0
char *drstate[] = {
	"Idle",
	"Syn1",
	"Syn2",
	"Cnt1",
	"Data",
	"Esc1",
	"Chck",
	"Done",
};
#endif

int vflag;

main(argc, argv)
int argc;
char **argv;
{
	register int i;
	register struct chxcvr *xp;
	register struct host *hp;

	nice (-20);
	while (argc > 1 && argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'v':
			vflag++;
		}
		argc--;
		argv++;
	}

	if ((mem = open(argc > 2 ? argv[2] : memfile, 0)) < 0)
		prexit("%a: can't read memory file\n");
	if ((i = open(argc > 1 ? argv[1] : symfile, 0)) < 0)
		prexit("%a: can't read symbol file\n");
	close(i);
	nlist(argc > 1 ? argv[1] : symfile, namelist);
/*
	for (i = 0; i < MAXSYMBOLS; i++)
		if (namelist[i].n_value == 0)
			fprintf(stderr, "%a: symbol %s not found\n",
				namelist[i].n_name);
 */
	if (namelist[0].n_value == -1)
		prexit("%a: cannot read symbol file: %s\n", symfile);
	for (i = 0; i < MAXSYMBOLS; i++)
		info.kaddr[i] = (caddr_t)namelist[i].n_value;
	lseek(mem, (long)info.kaddr[aconntab], 0);
	read(mem, Chconntab, sizeof(Chconntab));
	lseek(mem, (long)info.kaddr[amyaddr], 0);
	read(mem, &Chmyaddr, sizeof(Chmyaddr));
	lseek(mem, (long)info.kaddr[aroutetab], 0);
	read(mem, Chroutetab, sizeof(Chroutetab));
	lseek(mem, (long)info.kaddr[achhz], 0);
	read(mem, Chhz, sizeof(Chhz));
	if (Chhz == 0)
		Chhz = 60;
#if NCHDR > 0
	lseek(mem, (long)info.kaddr[adr11cxcvr], 0);
	read(mem, dr11cxcvr, sizeof(dr11cxcvr));
	lseek(mem, (long)info.kaddr[adr11chosts], 0);
	read(mem, dr11chosts, sizeof(dr11chosts));
#endif
#if NCHCH > 0
	lseek(mem, (long)info.kaddr[ach11xcvr], 0);
	read(mem, ch11xcvr, sizeof(ch11xcvr));
#endif
	lseek(mem, (long)info.kaddr[alsnlist], 0);
	read(mem, &Chlsnlist, sizeof(Chlsnlist));
	lseek(mem, (long)info.kaddr[arfclist], 0);
	read(mem, &Chrfclist, sizeof(Chrfclist));

	/* Chclock should be read last! */
	lseek(mem, (long)info.kaddr[aclock], 0);
	read(mem, &Chclock, sizeof(Chclock));
	printf("Myaddr is ");
	prhost(Chmyaddr, 14);
	printf("\nConnections:\n # State DestHost Index Idle Tlast Trecd Tackd Tw Rlast Rackd Rread Rw Flags\n");
	for (i = 0; i < CHNCONNS; i++)
		if (Chconntab[i] != NOCONN)
			prconn(Chconntab[i], i);
	if (Chrfclist)
		prrfcs();
#if NCHDR > 0
	prxcvr("DR11-C", dr11cxcvr, NCHDR);
#endif
#if NCHCH > 0
	prxcvr("CH11", ch11xcvr, NCHCH);
#endif
}
static char *xstathead =
" Received Transmitted CRC(xcvr) CRC(buff) Overrun Aborted Length Rejected";

prxcvr(name, xcvr, nx)
char *name;
struct chxcvr *xcvr;
int nx;
{
	register struct chxcvr *xp = xcvr;
	register int i;

	for (i = 0; i < nx; i++, xp++) {
		printf("%-8s %2d Netaddr: ", name, i);
		praddr(xcvr->xc_addr);
		printf(" Devaddr: %x\n", xp->xc_devaddr);
		printf("%s\n%9D%12D%10D%8D%8D%8D%8D%8D\n", xstathead,
			xp->xc_rcvd, xp->xc_xmtd, xp->xc_crcr, xp->xc_crci,
			xp->xc_lost, xp->xc_abrt, xp->xc_leng, xp->xc_rej);
		printf(" Tpacket Ttime Rpacket Rtime");
#if NCHDR > 0
		printf(" Tcnt Tstate  Rcnt Rstate");
#endif
		printf("\n%6x ", xp->xc_tpkt);
		pridle(xp->xc_ttime);
		printf(" %6x ", xp->xc_rpkt);
		pridle(xp->xc_rtime);
#if NCHDR > 0
		printf("%5d %-6s", xp->xc_info.xc_dr11c.dr_tcnt,
			drstate[xp->xc_info.xc_dr11c.dr_tstate]);
		printf("%5d %-6s", xp->xc_info.xc_dr11c.dr_rcnt,
			drstate[xp->xc_info.xc_dr11c.dr_rstate]);
		printf("  %6x", xp->xc_info.xc_dr11c.dr_addr);
		if (xp->xc_info.xc_dr11c.dr_intrup)
			printf(" I");
#endif
		printf("\n");
	}
}
pridle(n)
{
	register int idle;

	idle = Chclock - n;
	if (idle < Chhz)
		printf("%5dt", idle);
	else {
		idle += Chhz/2;
		idle /= Chhz;
		if (idle < 600)
			printf("%5ds", idle);
		else if (idle < 60*60*10)
			printf("%5dm", (idle + 30) / 60);
		else
			printf("%5dh", (idle + 60*30) / (60*60));
	}
}
prhost(h, len)
short h;
{
	register char *cp;
	extern char *chaos_name();

	if (cp = chaos_name(h))
		printf("%*.*s", len, len, cp);
	else
		praddr(h, len);
}
praddr(h, len)
short h;
{
	printf("%02d.%03d%*s", ((chaddr *)&h)->ch_subnet & 0377,
		((chaddr *)&h)->ch_host & 0377, len - 6, "");
}
prconn(conn, i)
struct connection *conn;
{
	register struct packet *pkt;
	struct connection myconn;

	lseek(mem, (long)conn, 0);
	read(mem, &myconn, sizeof(myconn));
	printf("%2d%c%-6s ", i, myconn.cn_mode == CHTTY ? '_' : ' ',
		statename[myconn.cn_state]);
	prhost(myconn.cn_faddr, 6);
	pridx(myconn.cn_fidx);
	pridle(myconn.cn_active);
	printf("%6d%6d%6d%3d", myconn.cn_tlast, myconn.cn_trecvd,
		myconn.cn_tacked, myconn.cn_twsize);
	printf("%6d%6d%6d%3d", myconn.cn_rlast, myconn.cn_racked,
		myconn.cn_rread, myconn.cn_rwsize);
	if (myconn.cn_flags & CHEOFSEEN)
		printf(" ES");
	if (myconn.cn_flags & CHANSWER)
		printf(" ANS ");
	if (myconn.cn_sflags & CHIWAIT)
		printf(" IW");
	if (myconn.cn_sflags & CHOWAIT)
		printf(" OW");
	printf("\n");
	switch(myconn.cn_state) {
	case CSRFCSENT:
		if (myconn.cn_thead) {
			lseek(mem, (long)myconn.cn_thead, 0);
			read(mem, &pktbuf, sizeof(pktbuf));
			printf(" RFC:'%.*s'\n", pktbuf.p_pkt.pk_len,
				pktbuf.p_pkt.pk_cdata);
		} else
			printf("NO RFC!\n");
		break;
	case CSRFCRCVD:
		if (myconn.cn_rhead) {
			lseek(mem, (long)myconn.cn_rhead, 0);
			read(mem, &pktbuf, sizeof(pktbuf));
			printf(" RFC:'%.*s'\n", pktbuf.p_pkt.pk_len,
				pktbuf.p_pkt.pk_cdata);
		} else
			printf("NO RFC!\n");
		break;
	case CSLISTEN:
		for (pkt = Chlsnlist; pkt; pkt = pktbuf.p_pkt.pk_next) {
			lseek(mem, (long)pkt, 0);
			read(mem, &pktbuf, sizeof(pktbuf));
			if (pktbuf.p_pkt.pk_stindex == myconn.cn_ltidx) {
				printf("Contact: '%.*s'\n",
					pktbuf.p_pkt.pk_len,
					pktbuf.p_pkt.pk_cdata);
				break;
			}
		}
		if (pkt == NOPKT)
			printf("No packet on listen list!\n");
		break;
	}
	if (vflag) {
		if (myconn.cn_thead) {
			printf("Transmit queue:\n");
			prq(myconn.cn_thead);
		}
		if (myconn.cn_rhead) {
			printf("Receive queue:\n");
			prq(myconn.cn_rhead);
		}
	}
}
prq(pkt)
register struct packet *pkt;
{
	for (; pkt; pkt = pktbuf.p_pkt.pk_next) {
		lseek(mem, (long)pkt, 0);
		read(mem, &pktbuf, sizeof(pktbuf));
		printf("\top: %3o\tpkn: %3u len:%d", pktbuf.p_pkt.pk_op,
			(unsigned)pktbuf.p_pkt.pk_pkn, pktbuf.p_pkt.pk_len);
		pridle(pktbuf.p_pkt.pk_time);
		printf("\n");
	}
}
prrfcs()
{
	register struct packet *pkt;

	printf("Queued RFC's:\n Sourcehost  Sourceidx RFC string\n");
	for (pkt = Chrfclist; pkt; pkt = pktbuf.p_pkt.pk_next) {
		lseek(mem, (long)pkt, 0);
		read(mem, (char *)&pktbuf, sizeof(pktbuf));
		printf(" ");
		prhost(pktbuf.p_pkt.pk_saddr, 14);
		pridx(pktbuf.p_pkt.pk_sidx);
		printf(" %.*s", pktbuf.p_pkt.pk_len, pktbuf.p_pkt.pk_cdata);
		if (pktbuf.p_pkt.pk_ackn != 0)
			printf(" (skipped)");
		printf("\n");
	}
}
pridx(i)
{
	printf("%2d/%3d", ((chindex *)&i)->ci_tidx & 0377,
		 ((chindex *)&i)->ci_uniq & 0377);
}
prexit(a,b,c,d,e)
char *a;
{
	fprintf(stderr, a, b, c, d, e);
	exit(1);
}
