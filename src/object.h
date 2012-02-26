#ifndef OBJECT_H
#define OBJECT_H

struct object {
	union {
		float x;
		unsigned long xi;
	};
	union {
		float y;
		unsigned long yi;
	};
	union {
		float z;
		unsigned long zi;
	};
	float speed;
	int angle;
	union {
		float size_x;
		unsigned long size_xi;
	};
	union {
		float size_y;
		unsigned long size_yi;
	};
	union {
		float size_z;
		unsigned long size_zi;
	};
	float circle; /* used for rough estimations */
	struct square *square;
	struct object *next;
};

int object_colliding(struct object *o1, struct object *o2);

void object_forward(struct object *o);

void object_backward(struct object *o);

void object_update_circle(struct object *o);

#endif
