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

#include <math.h>

#include "object.h"

enum directions {
	north,
	south,
	east,
	west
};

struct square {
	struct square *neighbours[4];
	struct object *objects;
};

struct point {
	float x;
	float y;
};

#define __interval_test(__o1, __o2, __interval, __xyz) \
	((__o1->__xyz + __interval) >= __o2->__xyz && \
		(__o1->__xyz - __interval) <= __o2->__xyz)

#define __rotate(__p, __x, __y, __rad) \
	__p.x = __x * cos(__rad) - __y * sin(__rad); \
	__p.y = __x * sin(__rad) + __y * cos(__rad);

#define __max(__a, __b) (__a > __b ? __a : __b)

static inline float deg_to_rad(int deg) {
	return ((float)deg) / 360 * M_PI * 2;
}

static inline void move_object(struct object *o, int factor) {
	float angle = deg_to_rad(360 - o->angle);
	o->x += o->speed * sin(angle) * factor;
	o->y += o->speed * cos(angle) * factor;
}

void object_forward(struct object *o) {
	move_object(o, 1);
}

void object_backward(struct object *o) {
	move_object(o, -1);
}

int object_colliding(struct object *o1, struct object *o2) {
	struct point p1[4];
	struct point p2[4];
	float angle_rad1;
	float angle_rad2;
	float delta_x;
	float delta_y;

	/* First tier tests, is there a chance of collision? */
	if (o1->circle > o2->circle) {
		if (!(__interval_test(o1, o2, o1->circle, x) &&
				__interval_test(o1, o2, o1->circle, y)))
			return 0;
	}
	else {
		if (!(__interval_test(o2, o1, o2->circle, x) &&
				__interval_test(o2, o1, o2->circle, y)))
			return 0;
	}

	/* Do all the z checking we do, not within 1 unit => no collision! */
	if (!__interval_test(o1, o2, __max(o1->size_z, o2->size_z) / 2 - 1, z))
		return 0;

	/* Second tier, rotate and get definite result */
	angle_rad1 = deg_to_rad(o1->angle);
	angle_rad2 = deg_to_rad(o2->angle);
	/* TODO: put millions of hours on fixing this up with preprocessor
	 * magic */
	__rotate(p1[0], o1->x - o1->size_x / 2, o1->y + o1->size_y / 2,
			angle_rad1);
	__rotate(p1[1], o1->x + o1->size_x / 2, o1->y + o1->size_y / 2,
			angle_rad1);
	__rotate(p1[2], o1->x + o1->size_x / 2, o1->y - o1->size_y / 2,
			angle_rad1);
	__rotate(p1[3], o1->x - o1->size_x / 2, o1->y - o1->size_y / 2,
			angle_rad1);
	__rotate(p2[0], o2->x - o2->size_x / 2, o2->y + o2->size_y / 2,
			angle_rad2);
	__rotate(p2[1], o2->x + o2->size_x / 2, o2->y + o2->size_y / 2,
			angle_rad2);
	__rotate(p2[2], o2->x + o2->size_x / 2, o2->y - o2->size_y / 2,
			angle_rad2);
	__rotate(p2[3], o2->x - o2->size_x / 2, o2->y - o2->size_y / 2,
			angle_rad2);

	/* Now, this may seem a bit strange, but move one center to the delta
	 * between the two objects' coordinates in the plane. It will make
	 * for a simpler comparison in the end. */
	delta_x = o1->x - o2->x;
	delta_y = o1->y - o2->y;
#define __move(__p, __n, __xy) __p[__n].__xy += delta_ ## __xy;
	/* TODO: same thing here with wasti^Winvesting some hours in
	 * beautification */
	__move(p1, 0, x);
	__move(p1, 0, y);
	__move(p1, 1, x);
	__move(p1, 1, y);
	__move(p1, 2, x);
	__move(p1, 2, y);
	__move(p1, 3, x);
	__move(p1, 3, y);

	/* TODO, see if we can gain anything from cache:ing these values. I
	 * would believe not, since turning happens quite frequently compared
	 * to what we lose with the cache misses */

	/* Alright, time for the actual collision detection. We will project
	 * both objects on lines that are perpendicular to the bodies edges,
	 * and if the ranges on these lines are overlapping for all lines, the
	 * objects are intersecting. This means two lines per object, which
	 * will be the same if the objects angles are 0, 90, 180 or 270 apart */
	/* Gotchas: The algorithm works for monotone polygons, but we assume
	 * that we only have squares, meaning that using the edges as
	 * projection lines is possible (as they are perpendicular with the
	 * neighbouring edges). */

	/* Left as an exercise for the reader. Just kidding, will add it soon. */

	return 1;
}

void object_update_circle(struct object *o) {
	o->circle = ceilf(sqrtf(pow(o->size_x, 2) + pow(o->size_y, 2)) / 2);
}
