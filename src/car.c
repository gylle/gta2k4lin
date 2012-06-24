#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>

#include "main.h"
#include "car.h"
#include "stl.h"

#define CARSIZE_X 3.0f
#define CARSIZE_Y 5.0f
#define CARSIZE_Z 2.0f

static void draw_stl_model(struct stl_model *m) {
    static const int stride = 3 + 3 + 2;

    glBegin(GL_TRIANGLES);
        int i;
        for(i = 0; i < m->nr_of_vertices; i++) {
            int offset = i*stride;
            glTexCoord2f(m->data[offset+6], m->data[offset+7]);
            glNormal3f(m->data[offset+3], m->data[offset+4], m->data[offset+5]);
            glVertex3f(m->data[offset], m->data[offset+1], m->data[offset+2]);
        }
    glEnd();
}

static void car_get_world_transform(void *data, plVector3 v, plReal *m) {
    /* printf("get_world_transform (%p, %p, %p)\n", data, v, m); */

    struct car *c = (struct car*)data;

    v[0] = c->o.x;
    v[1] = c->o.y;
    v[2] = c->o.z;

    memcpy(m, c->rotation, sizeof(float)*16);
}

static void car_set_world_transform(void *data, plVector3 v, plReal *m) {
    printf("set_world_transform (%p, %p, %p)\n", data, v, m);

    printf("v: %f, %f, %f\n", v[0], v[1], v[2]);
    printf("m: %f, %f, %f %f\n %f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
           m[0], m[1], m[2], m[3],
           m[4], m[5], m[6], m[7],
           m[8], m[9], m[10], m[11],
           m[12], m[13], m[14], m[15]);

    struct car *c = (struct car*)data;
    c->o.x = v[0];
    c->o.y = v[1];
    c->o.z = v[2];

    memcpy(c->rotation, m, sizeof(float)*16);

    /* FIXME? */
    c->rotation[15] = 1.0f;
}

void init_car(struct car *bil) {
    // Ladda en standardbil...

    printf("init_car: %p\n", bil);

    car_set_model(bil);

    /* FIXME: CARSIZE can (should?) theoretically be replaced by values
     *  calculated from the model's min/max values.
     */
    bil->o.size_x=CARSIZE_X;
    bil->o.size_y=CARSIZE_Y;
    bil->o.size_z=CARSIZE_Z;
    /* object_update_circle(&(bil->o)); */

    bil->helhet=100;

    bil->o.x=10;
    bil->o.y=10;
    bil->o.z=BSIZE + CARSIZE_Z / 2 + 1.0f; // Ska nog inte initialiseras här..

    bil->t1=1;
    bil->t2=1;
    bil->t3=1;
    bil->t4=1;

    bil->o.angle=0;

    bil->Points=0;

    float bt_weight = 50.0f;
    void *user_data = NULL; /* ? */
    bil->bt_shape = plNewBoxShape(CARSIZE_X/2.0f, CARSIZE_Y/2.0f, CARSIZE_Z/2.0f);

    bil->engineForce = bil->brakeForce = bil->steering = 0;


    bil->rotation[0] = 1.0f; bil->rotation[1] = 0.0f; bil->rotation[2] = 0.0f; bil->rotation[3] = 0.0f;
    bil->rotation[4] = 0.0f; bil->rotation[5] = 1.0f; bil->rotation[6] = 0.0f; bil->rotation[7] = 0.0f;
    bil->rotation[8] = 0.0f; bil->rotation[9] = 0.0f; bil->rotation[10] = 1.0f; bil->rotation[11] = 0.0f;
    bil->rotation[12] = 0.0f; bil->rotation[13] = 0.0f; bil->rotation[14] = 0.0f; bil->rotation[15] = 1.0f;

    bil->bt_rbody = plCreateRigidBodyWithCallback(user_data, bt_weight, bil->bt_shape,
                                                  &car_get_world_transform,
                                                  &car_set_world_transform,
                                                  bil);

    plRigidBody_SetActivationState(bil->bt_rbody, DISABLE_DEACTIVATION);

    /* Make it into a proper vehicle */
    plVehicleTuningHandle vehicle_tuning = plNewVehicleTuning();
    plVehicleRayCasterHandle vehicle_raycaster = plNewDefaultVehicleRaycaster(dynamics_world);
    /* plRaycastVehicleHandle */ bil->bt_vehicle = plNewRaycastVehicle(vehicle_tuning, bil->bt_rbody, vehicle_raycaster);

    plDynamicsWorld_AddVehicle(dynamics_world, bil->bt_vehicle);

    /* Shared wheel data */
    plVector3 wheelDirection = {0, 0, -1};
    plVector3 wheelAxle = {1, 0, 0};
    plReal suspensionRestLength = 0.5f;
    plReal wheelRadius = 0.5f;

    /* Add wheels */
    plVector3 cPoint0 = {-(CARSIZE_X/2.0f - 0.1f), (CARSIZE_Y/2.0f - 0.1f), -(CARSIZE_Z/2.0f - 0.1f)}; /* Front */
    plVector3 cPoint1 = {(CARSIZE_X/2.0f - 0.1f), (CARSIZE_Y/2.0f - 0.1f), -(CARSIZE_Z/2.0f - 0.1f)};
    plVector3 cPoint2 = {-(CARSIZE_X/2.0f - 0.1f), -(CARSIZE_Y/2.0f - 0.1f), -(CARSIZE_Z/2.0f - 0.1f)}; /* Back */
    plVector3 cPoint3 = {(CARSIZE_X/2.0f - 0.1f), -(CARSIZE_Y/2.0f - 0.1f), -(CARSIZE_Z/2.0f - 0.1f)};


    plRaycastVehicle_AddWheel(bil->bt_vehicle, cPoint0, wheelDirection, wheelAxle, suspensionRestLength, wheelRadius, vehicle_tuning, 1 /* frontWheel */);
    plRaycastVehicle_AddWheel(bil->bt_vehicle, cPoint1, wheelDirection, wheelAxle, suspensionRestLength, wheelRadius, vehicle_tuning, 1 /* frontWheel */);
    plRaycastVehicle_AddWheel(bil->bt_vehicle, cPoint2, wheelDirection, wheelAxle, suspensionRestLength, wheelRadius, vehicle_tuning, 0 /* backWheel */);
    plRaycastVehicle_AddWheel(bil->bt_vehicle, cPoint3, wheelDirection, wheelAxle, suspensionRestLength, wheelRadius, vehicle_tuning, 0 /* backWheel */);

    plAddRigidBody(dynamics_world, bil->bt_rbody);
    printf("Added %p\n", bil->bt_rbody);
}

void car_render(struct car *bil) {

    glPushMatrix();

    glTranslatef(bil->o.x, bil->o.y, bil->o.z);

    /* glRotatef((float)bil->o.angle,0.0f,0.0f,1.0f); */
    glMultMatrixf(bil->rotation);

    glBindTexture(GL_TEXTURE_2D,world.texIDs[bil->t1]);
    draw_stl_model(bil->model);

    glPopMatrix();
}

void car_set_model(struct car *bil) {
    static struct stl_model *car_model;

    if(car_model == NULL) {
        car_model = load_stl_model("data/gta2kcar.stl");
    }
    bil->model = car_model;
}
