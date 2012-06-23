#ifndef __CAR_H__
#define __CAR_H__

#include "object.h"

#include "Bullet-C-Api.h"
#include "btwrap.h"

struct car {
	// Texturer, nr1: tak. nr2: sidor. nr3: fram. nr4: bak:
	int t1,t2,t3,t4;
	// Fartsaker
	float maxspeed,maxbspeed, accspeed, bromsspeed;
	int turnspeed;
	// Hur fort bilen stannar om man inte gasar
	float speeddown;
	// Hur hel bilen är(%). 100 är helhel, 0 är heltrasig.
	int helhet;
	// Poäng. I Multiplayer spel hur många "frags" man har...
	int Points;

	struct object o;
	struct stl_model *model;

	plCollisionShapeHandle bt_shape;
	plRigidBodyHandle bt_rbody;
	plRaycastVehicleHandle bt_vehicle;

	float rotation[16];
	float engineForce, brakeForce, steering;
};

void init_car(struct car *bil);
void car_render(struct car *bil);
void car_set_model(struct car *bil);

#endif /* __CAR_H__ */
