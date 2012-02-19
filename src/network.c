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

#define PROTO_NICK		0
#define PROTO_REG_COMMIT	90
#define PROTO_REG_ACK		91
#define PROTO_MOVE		100
#define PROTO_AMSG		110
#define PROTO_SMSG		111
#define PROTO_NEW_USER		120
#define PROTO_DROPPED_USER	121


float pos[3];
int angle;
int pos_changed;
pthread_t thread;
pthread_mutex_t position_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_OPPONENTS 64
int used[MAX_OPPONENTS];
float opos[MAX_OPPONENTS][3];
unsigned long ids[MAX_OPPONENTS];

struct loop_args {
	int sock;
};

struct timeval last_position_update;

extern int errno;

int sock;

struct proto_move {
	float x, y, z;
	int angle;
} __attribute__((packed));

ssize_t send_all(int sock, void *b, ssize_t len, int flags) {
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

ssize_t recv_exact(int sock, void *b, ssize_t len, int flags) {
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

void *loop(void *args) {
	int sock = ((struct loop_args*)args)->sock;

	unsigned char sbuf[1024];
	unsigned char rbuf[1024];
	uint16_t u16;
	int16_t s16;
	uint32_t u32;
	int32_t s32;
	struct proto_move pmove;



	for (;;) {
		/* Assume that this is atomic enough */
		if (pos_changed) {
			u16 = htons(PROTO_MOVE);
			memcpy(&sbuf[0], &u16, 2);

			pthread_mutex_lock(&position_mutex);
			pos_changed = 0;
			pmove.x = pos[0];
			pmove.y = pos[1];
			pmove.z = pos[2];
			pmove.angle = htons(angle);
			pthread_mutex_unlock(&position_mutex);

			memcpy(&sbuf[2], &pmove, sizeof(pmove));

			send_all(sock, sbuf, sizeof(pmove) + 2, 0);
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
		hints.ai_flags = AF_INET;
	if (proto == 6)
		hints.ai_flags = AF_INET6;
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
		fprintf(stderr, "failed when sending nick! %s\n",
				strerror(errno));
		goto end;
	}

	/* Commit */
	u16 = htons(PROTO_REG_COMMIT);
	if (send_all(sock, &u16, 2, 0) < 0) {
		fprintf(stderr, "failed when committing registration! %s\n",
				strerror(errno));
		goto end;
	}

	/* Wait for ack */
	if (recv_exact(sock, &u16, 2, 0) < 0 || ntohs(u16) != PROTO_REG_ACK) {
		fprintf(stderr, "failed registration (no ack)! %s\n",
				strerror(errno));
		goto end;
	}

	fprintf(stderr, "done!\n");

	pthread_create(&thread, NULL, loop, (void *)&sock);

	ret = 0;

end:
	freeaddrinfo(server_addr_info_list);

	return ret;
}

int network_init(void) {
	return 0;
}

int network_put_position(float x, float y, float z, int a) {
	pthread_mutex_lock(&position_mutex);
	pos_changed = 1;
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
	angle = a;
	pthread_mutex_unlock(&position_mutex);

	return 0;
}

int network_get_positions(float *x, float *y, float *z, int *angle, int size) {
	return 0;
}
