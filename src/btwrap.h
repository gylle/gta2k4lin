#include "Bullet-C-Api.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef void (*get_world_transform_cb)(void*, plVector3 v, plReal *m);
    typedef void (*set_world_transform_cb)(void*, plVector3 v, plReal *m);

    plRigidBodyHandle plCreateRigidBodyWithCallback(void* user_data, float mass,
                                                    plCollisionShapeHandle cshape,
                                                    get_world_transform_cb gcb,
                                                    set_world_transform_cb scb,
                                                    void *ptr);

    void plRigidBody_ApplyForce(plRigidBodyHandle object, plVector3 force, plVector3 rel_pos);

    void plDynamicsWorld_SetGravity(plDynamicsWorldHandle handle, plReal x, plReal y, plReal z);


#ifdef __cplusplus
}
#endif
