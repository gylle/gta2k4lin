#ifndef __GUBBE_H__
#define __GUBBE_H__

#include "object.h"

#include "Bullet-C-Api.h"
#include "btwrap.h"

struct gubbe {
	// Texturer. ltexture2=huvudet. ltexture=resten. dtexturer=texture då gubben dött...
	int ltexture, ltexture2,  dtexture;
	// Fartsaker...
	float maxspeed,maxbspeed,accspeed;
	// Lever?
	int alive;
	// Tid tills han lever igen...
	int atimer;

	struct object o;

	plCollisionShapeHandle bt_shape;
	plRigidBodyHandle bt_rbody;
	float rotation[16];
};

void init_gubbe(struct gubbe *g);
void gubbe_move(struct gubbe *g);
void gubbe_render(struct gubbe *g);

#define GUBBSIZE_X 1.0f
#define GUBBSIZE_Y 1.0f
#define GUBBSIZE_Z 1.9f

#endif /* __GUBBE_H__ */
