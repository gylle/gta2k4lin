#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "lmq.h"

struct lmq_msg {
	unsigned len;
	char *data;
	struct lmq_msg *next;
};

#define LMQ_INIT_LOCK(q) pthread_mutex_init(&q->mutex, NULL);
#define LMQ_LOCK(q) pthread_mutex_lock(&q->mutex);
#define LMQ_UNLOCK(q) pthread_mutex_unlock(&q->mutex);

int lmq_init(struct lmq_queue *q) {
	int fds[2];

	pipe(fds);

	memset(q, 0, sizeof(struct lmq_queue));
	q->notify_r = fds[0];
	q->notify_w = fds[1];

	LMQ_INIT_LOCK(q);

	return 0;
}

int lmq_free(struct lmq_queue *q) {
	struct lmq_msg *qm, *qmf;
	int i;

	LMQ_LOCK(q);
	for (i = 0; i < LMQ_NPRIORITIES; i++) {
		qm = q->priorities_head[i];
		while (qm != NULL) {
			qmf = qm;
			qm = qm->next;
			free(qmf->data);
			free(qmf);
		}
	}
	LMQ_UNLOCK(q);

	return 0;
}

int lmq_send(struct lmq_queue *q, char *msg, size_t len, int prio) {
	struct lmq_msg *qm;
	int r;

	/* Prepare msg node */
	qm = (struct lmq_msg *)malloc(sizeof(struct lmq_msg));
	if (qm == NULL) {
		errno = ENOMEM;
		return -1;
	}

	memset(qm, 0, sizeof(struct lmq_msg));

	qm->data = (char *)malloc(len);
	if (qm == NULL) {
		errno = ENOMEM;
		return -1;
	}

	memcpy(qm->data, msg, len);
	qm->len = len;

	/* Insert */
	LMQ_LOCK(q);
	if (q->priorities_tail[prio] == NULL) {
		/* If tail is zero, head is also zero and queue is empty */
		q->priorities_tail[prio] = q->priorities_head[prio] = qm;
	}
	else {
		q->priorities_tail[prio]->next = qm;
		q->priorities_tail[prio] = qm;
	}
	LMQ_UNLOCK(q);

	/* Notify */
	r = 0;
	while (r != 1) {
		r = write(q->notify_w, "x", 1);
		if (r < 0 && r != EAGAIN)
			break;
	}

	return len;
}

int lmq_recv(struct lmq_queue *q, char *msg, size_t len, int *prio) {
	char b;
	int i, r;
	int ret;
	struct lmq_msg *qm = NULL;

	/* Remove from queue */
	LMQ_LOCK(q);
	for (i = 0; i < LMQ_NPRIORITIES; i++) {
		if (q->priorities_head[i] != NULL) {
			qm = q->priorities_head[i];
			if (qm->len > len) {
				LMQ_UNLOCK(q);
				errno = EMSGSIZE;
				return -1;
			}
			q->priorities_head[i] = qm->next;
			if (q->priorities_head[i] == NULL)
				q->priorities_tail[i] = NULL;
			break;
		}
	}
	LMQ_UNLOCK(q);

	if (qm == NULL) {
		errno = EAGAIN;
		return -1;
	}

	memcpy(msg, qm->data, qm->len);
	ret = qm->len;
	if (prio != NULL)
		*prio = i;
	free(qm->data);
	free(qm);

	r = 0;
	while (r != 1) {
		r = read(q->notify_r, &b, 1);
		if (r < 0 && r != EAGAIN)
			break;
	}

	return ret;
}

int lmq_getfd(struct lmq_queue *q) {
	return q->notify_r;
}
