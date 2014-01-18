/*
 *
 * Copyright Jonas Eriksson 2012
 * Copyright David Hedberg 2014
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

struct vector {
	float x, y;
};

#define span_test(__o1, __o2, __span, __xyz) \
	((__o1->__xyz + (__span)) >= (__o2->__xyz) && \
		(__o1->__xyz - (__span)) <= (__o2->__xyz))

#define __rotate(__p, __x, __y, __rad) \
	__p.x = __x * cos(__rad) - __y * sin(__rad); \
	__p.y = __x * sin(__rad) + __y * cos(__rad);

#define max(__a, __b) ((__a) > (__b) ? (__a) : (__b))
#define min(__a, __b) ((__a) < (__b) ? (__a) : (__b))

static inline float deg_to_rad(int deg) {
	return ((float)deg) / 360 * M_PI * 2;
}

static inline float scalar_product(struct vector a, struct vector b) {
	return a.x * b.x + a.y * b.y;
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

void get_projection_max_min(const struct vector points[], int npoints,
		const struct vector *edge, float *max, float *min) {
	int i;
	float sp;

	*min = *max = scalar_product(points[0], *edge);

	for (i = 1; i < npoints; i++) {
		sp = scalar_product(points[i], *edge);
		*min = min(*min, sp);
		*max = max(*max, sp);
	}
}

int object_colliding(struct object *o1, struct object *o2) {
	struct vector p1[4];
	struct vector p2[4];
	float angle_rad1;
	float angle_rad2;
	float delta_x;
	float delta_y;
	struct vector edges[4];
	float max1, min1;
	float max2, min2;
	int i;

	/* Do all the z checking we do */
	if (!span_test(o1, o2, max(o1->size_z, o2->size_z) / 2, z))
		return 0;

	/* First tier tests, is there a chance of collision? */
	if (!(span_test(o1, o2, o1->circle + o2->circle, x) &&
			span_test(o1, o2, o1->circle + o2->circle, y)))
		return 0;

	/* Second tier, rotate and get definite result */
	angle_rad1 = deg_to_rad(o1->angle);
	angle_rad2 = deg_to_rad(o2->angle);
	/* TODO: put millions of hours on fixing this up with preprocessor
	 * magic */
	__rotate(p1[0], 0 - o1->size_x / 2, 0 + o1->size_y / 2, angle_rad1);
	__rotate(p1[1], 0 + o1->size_x / 2, 0 + o1->size_y / 2, angle_rad1);
	__rotate(p1[2], 0 + o1->size_x / 2, 0 - o1->size_y / 2, angle_rad1);
	__rotate(p1[3], 0 - o1->size_x / 2, 0 - o1->size_y / 2, angle_rad1);
	__rotate(p2[0], 0 - o2->size_x / 2, 0 + o2->size_y / 2, angle_rad2);
	__rotate(p2[1], 0 + o2->size_x / 2, 0 + o2->size_y / 2, angle_rad2);
	__rotate(p2[2], 0 + o2->size_x / 2, 0 - o2->size_y / 2, angle_rad2);
	__rotate(p2[3], 0 - o2->size_x / 2, 0 - o2->size_y / 2, angle_rad2);

	/* Now, this may seem a bit strange, but move one center to the delta
	 * between the two objects' coordinates in the plane. It will make
	 * for a simpler comparison in the end. This instead of moving both
	 * points to their actual position in the plane. */
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
	 * to what we lose with the cache misses. */

	/* Alright, time for the actual collision detection. We will project
	 * both objects on lines that are perpendicular to the bodies edges,
	 * and if the ranges on these lines are overlapping for all lines, the
	 * objects are intersecting. */

	/* Gotchas:
	 * - The algorithm works only for monotone polygons.
	 * - Another gotcha: We assume we have squares (as visible in the
	 *   struct object). But more dimensions should be straight-forward to
	 *   add by changing the edge creation. */

	/* Create edges, that is vectors between two corners of the figures.
	 * Since opposing sides are parallel, only create two per figure.
	 * Also, since neighbouring edges are perpendicular, we need not twist
	 * each edge 90 degrees to get a perpendicular projection area! Beeing
	 * lazy is fun.
	 * Also also, the lines for the two objects will be basically the same
	 * if the objects' rotation angles are 0, 90, 180 or 270 apart. This
	 * opens up for more optimizations, but let's leave it as a TODO. */

	/* TODO: This should be done to completion, from edge creation to
	 * overlap check for each edge, so that we do not calculate things
	 * unneccessarily! */
	edges[0].x = p1[0].x - p1[1].x;
	edges[0].y = p1[0].y - p1[1].y;
	edges[1].x = p1[1].x - p1[2].x;
	edges[1].y = p1[1].y - p1[2].y;
	edges[2].x = p2[0].x - p2[1].x;
	edges[2].y = p2[0].y - p2[1].y;
	edges[3].x = p2[1].x - p2[2].x;
	edges[3].y = p2[1].y - p2[2].y;

	for (i = 0; i < 4; i++) {
		/* Project the figures on the edge vectors */
		get_projection_max_min(p1, 4, &(edges[i]), &max1, &min1);
		get_projection_max_min(p2, 4, &(edges[i]), &max2, &min2);

		/* Do overlap testing of the projections */
		if (max1 < min2)
			return 0;
		if (max2 < min1)
			return 0;
	}

	return 1;
}

void object_update_circle(struct object *o) {
	o->circle = ceilf(sqrtf(pow(o->size_x / 2, 2) + pow(o->size_y / 2, 2)));
}
