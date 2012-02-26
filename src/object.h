#ifndef OBJECT_H
#define OBJECT_H

struct object {
	union {
		float x;
		uint32_t xi;
	};
	union {
		float y;
		uint32_t yi;
	};
	union {
		float z;
		uint32_t zi;
	};
	float speed;
	int angle;
	union {
		float size_x;
		uint32_t size_xi;
	};
	union {
		float size_y;
		uint32_t size_yi;
	};
	union {
		float size_z;
		uint32_t size_zi;
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
