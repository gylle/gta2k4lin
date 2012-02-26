#ifndef NETWORK_H
#define NETWORK_H

#include "object.h"

#define NETWORK_MAX_OPPONENTS 64

struct opponent {
	uint32_t id;
	struct object *o;
	int in_use;
};

int network_amsg_send(char *msg);

int network_amsg_recv(char *msg, unsigned long *id, unsigned size);

int network_connect(char *addr, unsigned port, char *nick, int proto);

int network_init(void);

int network_put_position(struct object *o);

int network_get_positions(struct opponent o[]);

const char *network_lookup_id(unsigned long id);

#endif
