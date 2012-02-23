#ifndef OBJECT_H
#define OBJECT_H

struct object {
	float x, y, z;
	float speed;
	int angle;
	float size_x, size_y, size_z;
	float circle; /* used for rough estimations */
	struct square *square;
	struct object *next;
};

int object_colliding(struct object *o1, struct object *o2);

void object_forward(struct object *o);

void object_backward(struct object *o);

void object_update_circle(struct object *o);

#endif
