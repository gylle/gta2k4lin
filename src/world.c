#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>

#include "Bullet-C-Api.h"
#include "btwrap.h"

#include "world.h"

struct world world;

int map_load() {
    world.map.cubes = (struct cube *)calloc(world.map.nrcubex * world.map.nrcubey * world.map.nrcubez, sizeof(struct cube));
    if (world.map.cubes == NULL)  {
        return 0;
    }
    memset(world.map.cubes, 0, world.map.nrcubex * world.map.nrcubey * world.map.nrcubez *
           sizeof(struct cube));

    int loop1, loop2, loop3;

    for(loop1=0;loop1<world.map.nrcubex;loop1++) {
        for(loop2=0;loop2<world.map.nrcubey;loop2++) {
            for(loop3=0;loop3<world.map.nrcubez;loop3++) {
                map_cube(world, loop1, loop2, loop3).o.size_x=BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.size_y=BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.size_z=BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.x=loop1 * BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.y=loop2 * BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.z= -1 * loop3 * BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).texturenr=0;

                object_update_circle(&(map_cube(world, loop1, loop2, loop3).o));
            }
        }
    }

    map_cube(world, 0, 0, 0).o.z = BSIZE;
    map_cube(world, 0, 0, 0).texturenr=1;

    map_cube(world, 0, 1, 0).o.z = BSIZE * 2;
    map_cube(world, 0, 1, 0).texturenr=1;

    // Vägen -------------------------------
    for(loop1=0;loop1<world.map.nrcubey;loop1++)
        map_cube(world, 1, loop1, 0).texturenr=2;

    for(loop1=0;loop1<world.map.nrcubey;loop1++)
        map_cube(world, 2, loop1, 0).texturenr=3;

    for(loop1=0;loop1<world.map.nrcubex;loop1++)
        map_cube(world, loop1, world.map.nrcubey-2, 0).texturenr=8;

    for(loop1=2;loop1<world.map.nrcubex;loop1++)
        map_cube(world, loop1, world.map.nrcubey-3, 0).texturenr=9;

    map_cube(world, 1, world.map.nrcubey-2, 0).texturenr=5;
    map_cube(world, 2, world.map.nrcubey-2, 0).texturenr=6;
    map_cube(world, 2, world.map.nrcubey-3, 0).texturenr=7;


    // "Väggen" runtomkring
    for(loop1=0;loop1<world.map.nrcubey;loop1++) {
        map_cube(world, 0, loop1, 0).texturenr=4;
        map_cube(world, 0, loop1, 0).o.z = BSIZE * 2;
    }

    for(loop1=0;loop1<world.map.nrcubey;loop1++) {
        map_cube(world, world.map.nrcubex-1, loop1, 0).texturenr=4;
        map_cube(world, world.map.nrcubex-1, loop1, 0).o.z = BSIZE * 2;
    }

    for(loop1=0;loop1<world.map.nrcubex;loop1++) {
        map_cube(world, loop1, 0, 0).texturenr=4;
        map_cube(world, loop1, 0, 0).o.z = BSIZE * 2;
    }
    for(loop1=0;loop1<world.map.nrcubex;loop1++) {
        map_cube(world, loop1, world.map.nrcubey-1, 0).texturenr=4;
        map_cube(world, loop1, world.map.nrcubey-1, 0).o.z = BSIZE * 2;
    }

    // Vi lägger in lite buskar
    for(loop1=1;loop1<(world.map.nrcubey/2-1);loop1+=2) {
        map_cube(world, world.map.nrcubex/2, loop1, 0).texturenr=15;
        map_cube(world, world.map.nrcubex/2, loop1, 0).o.z = BSIZE * 2;
    }

    // Vägen in till mitten och den fina credits saken där.
    for(loop1=3;loop1<=world.map.nrcubex/2;loop1++)
        map_cube(world, loop1, world.map.nrcubey/2, 0).texturenr=7;

    map_cube(world, world.map.nrcubex/2, world.map.nrcubey/2, 1).texturenr=10;
    map_cube(world, world.map.nrcubex/2, world.map.nrcubey/2, 1).o.z = BSIZE * 2 * 2;

    /* FIXME: Fult */
    for(loop1=0;loop1<world.map.nrcubex;loop1++) {
        for(loop2=0;loop2<world.map.nrcubey;loop2++) {
            for(loop3=0;loop3<world.map.nrcubez;loop3++) {
                /* Bullet */
                plCollisionShapeHandle cube_shape;
                plRigidBodyHandle cube_rbody;
                void *user_data = NULL;

                cube_shape = plNewBoxShape(BSIZE, BSIZE, BSIZE);
                cube_rbody = plCreateRigidBody(user_data, 0.0f, cube_shape);

                float cube_pos[3];
                cube_pos[0] = map_cube(world, loop1, loop2, loop3).o.x;
                cube_pos[1] = map_cube(world, loop1, loop2, loop3).o.y;
                cube_pos[2] = map_cube(world, loop1, loop2, loop3).o.z;

                /* plVector3 == float[3] */
                plSetPosition(cube_rbody, cube_pos);

                plAddRigidBody(world.dynamics_world, cube_rbody);
            }
        }
    }

    return 1;
}

void camera_init() {
    world.camera.x = 0.0f;
    world.camera.y = 0.0f;
    world.camera.z = -30.0f;
}

void camera_move_for_car(struct car *car) {
    world.camera.x = car->o.x;
    world.camera.y = car->o.y;

    // Få kameran att höjas och sänkas beroende på hastigheten...
    GLfloat tmpSpeedVar=car->o.speed*5;
    if(tmpSpeedVar>0)
        tmpSpeedVar=-tmpSpeedVar;

    if(tmpSpeedVar>world.camera.SpeedVar)
        world.camera.SpeedVar+=0.4f;

    if(tmpSpeedVar<world.camera.SpeedVar)
        world.camera.SpeedVar-=0.4f;
}

void opponents_init() {
    int i;

    for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
        world.opponents[i].o = &(world.opponent_cars[i].o);
        world.opponent_cars[i].t1=1;
        world.opponent_cars[i].t2=1;
        world.opponent_cars[i].t3=1;
        world.opponent_cars[i].t4=1;
        car_set_model(&world.opponent_cars[i]);
    }
}

int cars_init() {
    camera_init(&world.camera);

    init_car(&world.bil);

    return 1;
}

int gubbar_init() {
    world.gubbar = malloc(sizeof(struct gubbe)*nrgubbar);
    memset(world.gubbar, 0, sizeof(struct gubbe)*nrgubbar);

    int i;
    for(i = 0; i < nrgubbar; i++) {
        init_gubbe(&world.gubbar[i]);
        printf("init_gubbe: %p:\n", &world.gubbar[i]);
    }

    return 1;
}

int world_init() {
    /* Bullet */
    world.physics_sdk = plNewBulletSdk();
    world.dynamics_world = plCreateDynamicsWorld(world.physics_sdk);
    plDynamicsWorld_SetGravity(world.dynamics_world, 0, 0, -30.0f);

    world.map.nrcubex = 20;
    world.map.nrcubey = 20;
    world.map.nrcubez = 2;

    map_load();

    camera_init();
    opponents_init();
    cars_init();
    gubbar_init();

    return 1;
}
