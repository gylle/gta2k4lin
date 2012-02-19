#ifndef NETWORK_H
#define NETWORK_H

int network_amsg_send(char *msg);

int network_amsg_recv(char *msg, unsigned size);

int network_connect(char *addr, unsigned port, char *nick, int proto);

int network_init(void);

int network_put_position(float x, float y, float z, int a);

int network_get_positions(float *x, float *y, float *z, int *angle, int size);

#endif
