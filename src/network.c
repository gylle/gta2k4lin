/*
 * gta2k4lin
 *
 * Copyright David Hedberg  2001,2012
 * Copyright Jonas Eriksson 2012
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <sys/ioctl.h>

#include "object.h"
#include "network.h"
#include "lmq.h"

#define PROTO_NICK		0
#define PROTO_REG_COMMIT	90
#define PROTO_REG_ACK		91
#define PROTO_MOVE		100
#define PROTO_RESIZE		101
#define PROTO_AMSG		110
#define PROTO_SMSG		111
#define PROTO_NEW_USER		120
#define PROTO_DROPPED_USER	121

/* Since this is an interface towards the ui-part, ingress is things that we
 * get from the local user, and outgress is what is going to be delivered to
 * the local user. */
lmq_t amsg_ingress;
lmq_t amsg_outgress;
lmq_t smsg_outgress;

pthread_mutex_t position_mutex = PTHREAD_MUTEX_INITIALIZER;
struct object local_object;
int local_object_pos_changed;
int local_object_size_changed;
pthread_t thread;

int socktransferunit; /* damp */

struct opponent {
	unsigned long id;
	int used;
	char *nick;
	struct object o;
};

struct opponent opponents[NETWORK_MAX_OPPONENTS];
pthread_mutex_t opponents_mutex = PTHREAD_MUTEX_INITIALIZER;

struct timeval last_position_update;

extern int errno;

int sock;

struct proto_move_send {
	float x, y, z;
	int angle;
} __attribute__((packed));

struct proto_resize_send {
	float x, y, z;
} __attribute__((packed));

struct proto_move_recv {
	uint32_t id;
	float x, y, z;
	int angle;
} __attribute__((packed));

struct proto_resize_recv {
	uint32_t id;
	float x, y, z;
} __attribute__((packed));

int network_amsg_send(char *msg) {
	return lmq_send(&amsg_ingress, msg, strlen(msg), 0);
}

#define min(x,y) (x) < (y) ? (x) : (y)
int network_amsg_recv(char *msg, unsigned long *id, unsigned size) {
	/* TODO: detta är racy som fan, inte säkert både msg och id finns i
	 * kön */
	return lmq_recv(&amsg_outgress, msg, size, NULL) +
		lmq_recv(&amsg_outgress, id, sizeof(unsigned long), NULL);
}

static int register_id(unsigned long id, char *nick) {
	int ret = -1;
	int i;

	pthread_mutex_lock(&opponents_mutex);
	for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
		if (!opponents[i].used)
			break;
	}

	if (i >= NETWORK_MAX_OPPONENTS)
		return -1;

	opponents[i].id = id;
	opponents[i].nick = malloc(strlen(nick) + 1);
	if (opponents[i].nick == NULL)
		goto out;
	strcpy(opponents[i].nick, nick);

	ret = 0;
out:
	pthread_mutex_unlock(&opponents_mutex);

	return ret;
}

static int remove_id(unsigned long id) {
	int ret = -1;
	int i;

	pthread_mutex_lock(&opponents_mutex);
	for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
		if (opponents[i].id == id) {
			opponents[i].used = 0;
			free(opponents[i].nick);
			opponents[i].nick = NULL;
			ret = 1;
			break;
		}
	}
	pthread_mutex_unlock(&opponents_mutex);
	return ret;
}

const char *network_lookup_id(unsigned long id) {
	char *ret = NULL;
	int i;

	pthread_mutex_lock(&opponents_mutex);

	for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
		if (opponents[i].id == id) {
			/* TODO: här finns ett race, nicket kan bli free:at
			 * innan det används klar i andra ändan */
			ret = opponents[i].nick;
			break;
		}
	}

	pthread_mutex_unlock(&opponents_mutex);

	return ret;
}

static ssize_t send_all(int sock, void *b, ssize_t len, int flags) {
	unsigned char *buf = (unsigned char *)b;
	ssize_t sent = 0;
	ssize_t r;
	while (sent < len) {
		r = send(sock, &buf[sent], len - sent, flags);
		if (r < 0) {
			if (r == EAGAIN || r == EWOULDBLOCK || r == EINTR)
				continue;

			return r;
		}
		sent += r;
	}

	return sent;
}

static ssize_t recv_exact(int sock, void *b, ssize_t len, int flags) {
	unsigned char *buf = (unsigned char *)b;
	ssize_t recvd = 0;
	ssize_t r;
	while (recvd < len) {
		r = recv(sock, &buf[recvd], len - recvd, flags);
		if (r < 0) {
			if (r == EAGAIN || r == EWOULDBLOCK || r == EINTR)
				continue;

			return r;
		}
		recvd += r;
	}

	return recvd;
}

static int handle_data(char *buf, size_t size) {
	uint16_t u16;
	uint32_t u32;
	uint16_t event;
	size_t consumed = 0;
	struct proto_move_recv pmove;
	struct proto_resize_recv psize;
	char mbuf[2048];
	unsigned long id;
	unsigned len;

	if (size < 2)
		return 0;

	/*
	int i;
	for (i=0; i<size; i++)
		printf("0x%02x ", buf[i]);
	printf("\n");
	*/
	
	memcpy(&u16, buf, 2);
	event = ntohs(u16);

	switch (event) {
	case PROTO_MOVE:
		if (size < sizeof(pmove) + 2)
			return 0;

		memcpy(&pmove, &buf[2], sizeof(pmove));

		consumed = sizeof(pmove) + 2;
		break;
	case PROTO_RESIZE:
		if (size < sizeof(psize) + 2)
			return 0;

		memcpy(&psize, &buf[2], sizeof(psize));

		consumed = sizeof(psize) + 2;
		break;
	case PROTO_AMSG:
		if (size < 6 + 2)
			return 0;

		memcpy(&u32, &buf[2], 4);
		id = ntohl(u32);
		memcpy(&u16, &buf[6], 2);
		len = ntohs(u16);

		if (size < (8 + len))
			return 0;

		memcpy(mbuf, &buf[8], len);
		mbuf[len] = 0;

		printf("User with id %lu just said: '%s'\n", id, mbuf);
		lmq_send(&amsg_outgress, mbuf, len + 1, 0);
		lmq_send(&amsg_outgress, &id, sizeof(id), 0);

		consumed = len + 8;
		break;
	case PROTO_SMSG:
		if (size < 2 + 2)
			return 0;

		memcpy(&u16, &buf[2], 2);
		len = ntohs(u16);

		if (size < (4 + len))
			return 0;

		memcpy(mbuf, &buf[4], len);
		mbuf[len] = 0;
		printf("Server just said: '%s'\n", mbuf);
		lmq_send(&smsg_outgress, mbuf, len + 1, 0);

		consumed = len + 4;
		break;
	case PROTO_NEW_USER:
		if (size < 5 + 2)
			return 0;

		memcpy(&u32, &buf[2], 4);
		id = ntohl(u32);
		len = buf[6];

		if (size < (7 + len))
			return 0;

		memcpy(mbuf, &buf[7], len);
		mbuf[len] = 0;

		register_id(id, mbuf);
		printf("User %s with id %lu just joined\n", mbuf, id);

		consumed = len + 7;
		break;
	case PROTO_DROPPED_USER:
		if (size < 4 + 2)
			return 0;

		memcpy(&u32, &buf[2], 4);
		id = ntohl(u32);
		
		remove_id(id);
		printf("User id %lu just left\n", id);

		consumed = 6;
		break;
	default:
		fprintf(stderr, "Konstig data, här. { 0x%02x, 0x%02x }\n",
				buf[0], buf[1]);
	}

	return consumed;
}

/* This should be large enough to hold the biggest message size, which I
 * believe is amsg with a 1023-byte msg, so 1027 bytes */
#define RBUF_SIZE 2048
static void *loop(void *arg) {
	int sock = *((int *)arg);
	char sbuf[1024];
	char rbuf[RBUF_SIZE];
	char buf[1024];
	size_t rbufpos = 0;
	uint16_t u16;
	struct proto_move_send pmove;
	struct proto_resize_send psize;
	struct pollfd fds[8];
	int nfds;
	int r;

	fds[0].fd = sock;
	fds[0].events = POLLIN;
	fds[1].fd = lmq_getfd(&amsg_ingress);
	fds[1].events = POLLIN;
	nfds = 2;

	for (;;) {

		r = poll(fds, nfds, 100);
		if (r < 0) {
			fprintf(stderr, "Poll failed: %s\n", strerror(errno));
			return NULL;
		}

		/* amsg to send */
		if (fds[1].revents & POLLIN) {
			r = lmq_recv(&amsg_ingress, buf, 1024, NULL);
			if (r >= 0) {
				u16 = htons(PROTO_AMSG);
				memcpy(&sbuf[0], &u16, 2);
				u16 = htons(r);
				memcpy(&sbuf[2], &u16, 2);
				memcpy(&sbuf[4], buf, r);
				if (send_all(sock, sbuf, r + 4, 0) < 0) {
					fprintf(stderr, "Unable to send amsg: %s\n",
							strerror(errno));
				}
			}
			else {
				fprintf(stderr, "Unable to retrieve local amsg: %s\n",
						strerror(errno));
			}
		}

		/* data to recv */
		if (fds[0].revents & POLLIN) {
			/* Write to the buffer rbuf. This is where we keep
			 * messages while waiting for them to be completely
			 * recieved. */
			r = recv(sock, &rbuf[rbufpos], RBUF_SIZE - rbufpos, 0);
			if (r < 0 && !(r == EAGAIN || r == EWOULDBLOCK ||
						r == EINTR)) {
				fprintf(stderr, "Fault during recv: %s\n",
						strerror(r));
				return NULL;
			}

			if (r > 0) {
				rbufpos += r;

				/* Alright, let's see if we have any parseable
				 * data */
				int c = 0;
				while ((r = handle_data(&rbuf[c], rbufpos - c)) > 0) {
					c += r;
				}

				/* Remove the things we have handled */
				if (c != 0 && c != rbufpos) {
					memmove(rbuf, &rbuf[c], rbufpos - c);
				}
				rbufpos -= c;
			}
		}

		/* position to send */
		if (local_object_pos_changed) {
			u16 = htons(PROTO_MOVE);
			memcpy(&sbuf[0], &u16, 2);

			pthread_mutex_lock(&position_mutex);
			local_object_pos_changed = 0;
			pmove.x = local_object.x;
			pmove.y = local_object.y;
			pmove.z = local_object.z;
			pmove.angle = htons(local_object.angle);
			pthread_mutex_unlock(&position_mutex);

			memcpy(&sbuf[2], &pmove, sizeof(pmove));

			send_all(sock, sbuf, sizeof(pmove) + 2, 0);
		}

		/* Size to send */
		if (local_object_size_changed) {
			u16 = htons(PROTO_RESIZE);
			memcpy(&sbuf[0], &u16, 2);

			pthread_mutex_lock(&position_mutex);
			local_object_pos_changed = 0;
			psize.x = local_object.size_x;
			psize.y = local_object.size_y;
			psize.z = local_object.size_z;
			pthread_mutex_unlock(&position_mutex);

			memcpy(&sbuf[2], &psize, sizeof(psize));

			send_all(sock, sbuf, sizeof(psize) + 2, 0);
		}
	}

	return NULL;
}

int network_connect(char *addr, unsigned port, char *nick, int proto) {
	struct addrinfo *server_addr_info_list, *i;
	struct addrinfo hints;

	int ret = -1;
	int sock;
	int r;
	char on = 1;
	unsigned char nicklen;
	uint16_t u16;
	unsigned char sbuf[1024];
	char buf[1024];


	/* Lookup host, use hints to limit ipv6/ipv6 if needed */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; /* We need these to to avoid that */
	hints.ai_protocol = IPPROTO_TCP; /* getaddrinfo create combinations of
					    all possible combinations */
	/* Force protocol? */
	if (proto == 4)
		hints.ai_family = AF_INET;
	if (proto == 6)
		hints.ai_family = AF_INET6;
	hints.ai_flags = AI_IDN | AI_CANONIDN;
	r = getaddrinfo(addr, NULL, &hints, &server_addr_info_list);
	if (r) {
		fprintf(stderr, "Unable to look up '%s': %s\n", addr,
				gai_strerror(errno));
		return 1;
	}
	
	/* Connect! */
	for (i = server_addr_info_list; i != NULL; i = i->ai_next) {

		/* This proved to be unneccessarily tricky.. */
		struct sockaddr_in *sin = (struct sockaddr_in *)(i->ai_addr);
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)(i->ai_addr);
		switch (i->ai_family) {
		case AF_INET:
			inet_ntop(i->ai_family,
					&sin->sin_addr, buf, 1024);
			sin->sin_port = htons(port);
			break;
		case AF_INET6:
			inet_ntop(i->ai_family,
					&sin6->sin6_addr, buf, 1024);
			sin6->sin6_port = htons(port);
			break;
		}

		fprintf(stderr, "Trying %s, port %u...", buf, port);
		fflush(stderr);

		if ((sock = socket(i->ai_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			fprintf(stderr, "failed. %s\n",
					strerror(errno));
			continue;
		}

		if (connect(sock, i->ai_addr, i->ai_addrlen) != -1) {
			break; /* Alright, connected! */
		}

		fprintf(stderr, "failed. %s\n", strerror(errno));

		close(sock);
	}
	if (i == NULL) {
		goto end;
	}

	fprintf(stderr, "connected, registering..");
	fflush(stderr);

	/* Send nick */
	u16 = htons(PROTO_NICK);
	memcpy(&sbuf[0], &u16, 2);
	nicklen = strlen(nick);
	sbuf[2] = nicklen;
	memcpy(&sbuf[3], nick, nicklen);
	if (send_all(sock, sbuf, nicklen + 3, 0) < 0) {
		fprintf(stderr, "failed when sending nick: %s\n",
				strerror(errno));
		goto end;
	}

	/* Commit */
	u16 = htons(PROTO_REG_COMMIT);
	if (send_all(sock, &u16, 2, 0) < 0) {
		fprintf(stderr, "failed when committing registration: %s\n",
				strerror(errno));
		goto end;
	}

	/* Wait for ack */
	if (recv_exact(sock, &u16, 2, 0) < 0 || ntohs(u16) != PROTO_REG_ACK) {
		fprintf(stderr, "failed registration (no ack): %s\n",
				strerror(errno));
		goto end;
	}

	fprintf(stderr, "done!\n");

	if (ioctl(sock, FIONBIO, &on) < 0) {
		fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
	}

	socktransferunit = sock;
	pthread_create(&thread, NULL, loop, (void *)&socktransferunit);

	ret = 0;

end:
	freeaddrinfo(server_addr_info_list);

	return ret;
}

int network_init(void) {
	lmq_init(&amsg_ingress);
	lmq_init(&amsg_outgress);
	lmq_init(&smsg_outgress);

	return 0;
}

void network_deinit(void) {
	lmq_free(&amsg_ingress);
	lmq_free(&amsg_outgress);
	lmq_free(&smsg_outgress);
}

int network_put_position(struct object *o) {
	pthread_mutex_lock(&position_mutex);

	if (local_object.x != o->x || local_object.y != o->y ||
			local_object.z != o->z ||
			local_object.angle != o->angle) {
		local_object_pos_changed = 1;
		local_object.x = o->x;
		local_object.y = o->y;
		local_object.z = o->z;
		local_object.angle = o->angle;
	}
	if (local_object.size_x != o->size_x ||
			local_object.size_y != o->size_y ||
			local_object.size_z != o->size_z) {
		local_object_size_changed = 1;
		local_object.size_x = o->size_x;
		local_object.size_y = o->size_y;
		local_object.size_z = o->size_z;
	}
	pthread_mutex_unlock(&position_mutex);

	return 0;
}

int network_get_positions(struct object *o[]) {
	return 0;
}
