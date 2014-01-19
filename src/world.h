#ifndef __WORLD_H__
#define __WORLD_H__

#include "object.h"
#include "gubbe.h"
#include "car.h"
#include "network.h"
#include "linmath.h"

#define BSIZE 5.0f
#define nrgubbar 100
#define map_cube(world, x, y, z) world.map.cubes[((x) * (world).map.nrcubey + (y)) * (world).map.nrcubez + (z)]

struct camera {
    GLfloat x, y, z;
    GLfloat SpeedVar;

    mat4x4 projection;
    mat4x4 view;
};

struct spelare {
    int points;
    int krockar;
    int runovers;
};

struct cube {
    int texturenr;
    int in_use;

    struct object o;
};

struct map {
    int nrcubex;
    int nrcubey;
    int nrcubez;
    struct cube *cubes;

    /* FIXME: Using this only to store
     * rendering stuff
     */
    struct object r_o;
};

struct world {
    struct map map;

    plPhysicsSdkHandle physics_sdk;
    plDynamicsWorldHandle dynamics_world;
    struct gubbe *gubbar;
    struct camera camera;
    struct car bil;
    struct car opponent_cars[NETWORK_MAX_OPPONENTS];
    struct opponent opponents[NETWORK_MAX_OPPONENTS];
    struct spelare player;

    /* FIXME: Move somewhere else */
    const char **texture_filenames;
    GLuint *texIDs;
    int ntextures;
};

extern struct world world;

int world_init();
void map_draw();
void camera_move_for_car(struct car *car);

#endif
