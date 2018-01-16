/*
 * declarations for host table
 */

#define HOSTTAB	"/etc/hosttab"
#define MAXHOST	20		/* Maximum size of host name */

struct net_entry {
	char *net_name;
	int net_number;
	int net_type;
};
#define NT_CHAOS	1	/* Net is a chaos net */

struct host_entry {
	char *host_name;
	struct net_address *host_address;
	int host_server;	/* if SERVER, else USER */
	char *host_system;
	char *host_machine;
	char **host_nicnames;
};

struct net_address {
	short addr_net;
	short addr_host;
};

struct ip_address {	/* Internet Protocol address */
	char ip_net;	/* network */
	char ip_hhost;	/* most significant byte of address (often subnet) */
	char ip_mhost;	/* middle byte of address */
	char ip_lhost;	/* least significant byte of address */
};

struct host_data	{
	struct net_entry	*ht_nets;	/* Pointer to net table */
	struct host_entry	*ht_hosts;	/* Pointer to host table */
	struct host_entry	*ht_me;		/* This host */
	int			ht_nsize;	/* Size of net table */
	int			ht_hsize;	/* Size of host table */
};
extern struct host_data *host_data;	/* Filled inside the library */

extern struct host_entry *host_info(char *name);
extern char *host_name(char *name);
extern char *host_system(char *name);
extern char *host_machine(char *name);
extern char *chaos_name(short addr);
extern int net_number(char *name);
extern unsigned short chaos_addr(char *name, int subnet);
extern unsigned short chaos_host(struct host_entry *h, int subnet);
extern int arpa_addr(char *name);
extern int ip_addr(char *name, int net, int subnet, struct ip_address *ip);
extern struct host_entry *host_here(void);	/* This host's entry */
extern char *host_me(void);			/* Name of this host */
extern int host_start(void);
extern struct host_entry *host_next(void);
extern int readhosts(void);
extern int nicnames(struct host_entry *h, char *p);
extern int arpa_host(struct host_entry *h);
