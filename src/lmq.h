/*
 * lmq - process-local messaging queues
 *
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

#ifndef LMQ_H
#define LMQ_H


#define LMQ_NPRIORITIES 1

struct lmq_queue {
	struct lmq_msg *priorities_head[LMQ_NPRIORITIES];
	struct lmq_msg *priorities_tail[LMQ_NPRIORITIES];
	pthread_mutex_t mutex;
	int notify_w;
	int notify_r;
};

typedef struct lmq_queue lmq_t;

int lmq_init(lmq_t *q);
int lmq_free(lmq_t *q);
int lmq_send(lmq_t *q, void *msg, size_t len, int prio);
int lmq_recv(lmq_t *q, void *msg, size_t len, int *prio);
int lmq_getfd(lmq_t *q);

#endif
