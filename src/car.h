#ifndef __CAR_H__
#define __CAR_H__

#include "object.h"

#include "Bullet-C-Api.h"
#include "btwrap.h"
#include "linmath.h"

struct car {
	struct object o;

	// Texturer, nr1: tak. nr2: sidor. nr3: fram. nr4: bak:
	int t1,t2,t3,t4;
	// Hur hel bilen är(%). 100 är helhel, 0 är heltrasig.
	int helhet;
	// Poäng. I Multiplayer spel hur många "frags" man har...
	int Points;

	struct stl_model *model;

	plCollisionShapeHandle bt_shape;
	plRigidBodyHandle bt_rbody;
	plRaycastVehicleHandle bt_vehicle;

	float engineForce, brakeForce, steering;
};

void init_car(struct car *bil);
void car_render(struct car *bil);
void car_set_model(struct car *bil);

#endif /* __CAR_H__ */
