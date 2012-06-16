#ifndef __MAIN_H__
#define __MAIN_H__

#include "Bullet-C-Api.h"

struct world {
	int nrcubex;
	int nrcubey;
	int nrcubez;
	struct cube *map;
	const char **texture_filenames;
	GLuint *texIDs;
	int ntextures;
};

/* FIXME: Move somewhere */
void draw_quads(float vertices[], int count);

extern plPhysicsSdkHandle physics_sdk;
extern plDynamicsWorldHandle dynamics_world;
extern struct world world;

extern int NoBlend;
extern float blendcolor;

#define BSIZE 5.0f

#endif /* __MAIN_H__ */
