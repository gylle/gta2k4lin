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
int lmq_send(lmq_t *q, char *msg, size_t len, int prio);
int lmq_recv(lmq_t *q, char *msg, size_t len, int *prio);
int lmq_getfd(lmq_t *q);

#endif
