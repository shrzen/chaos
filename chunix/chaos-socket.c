#include "../h/chaos.h"
#include "chsys.h"
#include "chconf.h"
#include "../chncp/chncp.h"

#include "log.h"

#define E2BIG	10
#define ENOBUFS	11
#define ENXIO	12
#define EIO	13

#ifdef DEBUG_CHAOS
#define ASSERT(x,y)	if(!(x)) printf("%s: Assertion failure\n",y);
#else
#define ASSERT(x,y)
#endif

/*
 * Return a connection or return NULL and set *errnop to any error.
 */
struct connection *
chopen_conn(struct chopen *c, int wflag,int *errnop)
{
	struct connection *conn;
	struct packet *pkt;
	int rwsize, length;
        struct chopen cho;

        tracef(TRACE_LOW, "chopen_conn(wflag=%d)", wflag);

	length = c->co_clength + c->co_length + (c->co_length ? 1 : 0);
	if (length > CHMAXPKT ||
	    c->co_clength <= 0) {
		*errnop = -E2BIG;
		return NOCONN;
	}

	debugf(DBG_LOW,
	       "chopen_conn: c->co_length %d, c->co_clength %d, length %d",
	       c->co_length, c->co_clength, length);

	pkt = ch_alloc(length, 0);
	if (pkt == NOPKT) {
		*errnop = -ENOBUFS;
		return NOCONN;
	}

	if (c->co_length)
		pkt->pk_cdata[c->co_clength] = ' ';

	memcpy(pkt->pk_cdata, c->co_contact, c->co_clength);
	if (c->co_length)
		memcpy(&pkt->pk_cdata[c->co_clength + 1], c->co_data,
		       c->co_length);

	rwsize = c->co_rwsize ? c->co_rwsize : CHDRWSIZE;
	SET_PH_LEN(pkt->pk_phead,length);

	conn = c->co_host ?
		ch_open(c->co_host, rwsize, pkt) :
		ch_listen(pkt, rwsize);

	if (conn == NOCONN) {
		debugf(DBG_LOW, "chopen_conn: NOCONN");
		*errnop = -ENXIO;
		return NOCONN;
	}

	debugf(DBG_LOW, "chopen_conn: c->co_async %d", c->co_async);
	debugf(DBG_LOW, "chopen_conn: conn %p", conn);

#if 0
	if (!c->co_async) {
		/*
		 * We should hang until the connection changes from
		 * its initial state.
		 * If interrupted, flush the connection.
		 */

//		current->timeout = (unsigned long) -1;

		*errnop = chwaitfornotstate(conn, c->co_host ?
                                            CSRFCSENT : CSLISTEN);
		if (*errnop) {
			rlsconn(conn);
			return NOCONN;
		}

		/*
		 * If the connection is not open, the open failed.
		 * Unless is got an ANS back.
		 */
		if (conn->cn_state != CSOPEN &&
		    (conn->cn_state != CSCLOSED ||
		     (pkt = conn->cn_rhead) == NOPKT ||
		     pkt->pk_op != ANSOP))
		{
			debugf(DBG_LOW,
			       "chopen_conn: open failed; cn_state %d",
			       conn->cn_state);
			rlsconn(conn);
			*errnop = -EIO;
			return NOCONN;
		}
	}
#endif

//	if (wflag)
//		conn->cn_sflags |= CHFWRITE;
//	conn->cn_sflags |= CHRAW;

	conn->cn_mode = CHSTREAM;
        debugf(DBG_LOW, "chopen_conn() done");

	return conn;
}
