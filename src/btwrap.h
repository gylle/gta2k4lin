#ifndef __BTWRAP_H__
#define __BTWRAP_H__

#include "Bullet-C-Api.h"

#ifdef __cplusplus
extern "C" {
#endif

    PL_DECLARE_HANDLE(plVehicleRayCasterHandle);
    PL_DECLARE_HANDLE(plVehicleTuningHandle);
    PL_DECLARE_HANDLE(plRaycastVehicleHandle);

    /* btCollisionObject.h */
    #define DISABLE_DEACTIVATION 4

    typedef void (*get_world_transform_cb)(void*, plVector3 v, plReal *m);
    typedef void (*set_world_transform_cb)(void*, plVector3 v, plReal *m);

    plRigidBodyHandle plCreateRigidBodyWithCallback(void* user_data, float mass,
                                                    plCollisionShapeHandle cshape,
                                                    get_world_transform_cb gcb,
                                                    set_world_transform_cb scb,
                                                    void *ptr);

    void plRigidBody_SetActivationState(plRigidBodyHandle rbHandle, int state);
    void plRigidBody_ApplyForce(plRigidBodyHandle object, plVector3 force, plVector3 rel_pos);

    void plDynamicsWorld_SetGravity(plDynamicsWorldHandle handle, plReal x, plReal y, plReal z);
    void plDynamicsWorld_AddVehicle(plDynamicsWorldHandle wHandle, plRaycastVehicleHandle vHandle);

    plVehicleRayCasterHandle plNewDefaultVehicleRaycaster(plDynamicsWorldHandle handle);
    plVehicleTuningHandle plNewVehicleTuning();
    plRaycastVehicleHandle plNewRaycastVehicle(plVehicleTuningHandle tHandle,
                                               plRigidBodyHandle rHandle,
                                               plVehicleRayCasterHandle rcHandle);
    void plRaycastVehicle_SetCoordinateSystem(plRaycastVehicleHandle vHandle,
                                              int rightIndex, int upIndex, int forwardIndex);
    void plRaycastVehicle_AddWheel(plRaycastVehicleHandle vHandle, plVector3 cPoint, plVector3 wDirection,
                                   plVector3 wAxleCS, plReal suspensionRestLength, plReal wheelRadius,
                                   plVehicleTuningHandle tHandle, int isFrontWheel);
    void plRaycastVehicle_ApplyEngineForce(plRaycastVehicleHandle vHandle, plReal engineForce, int wheelIndex);
    void plRaycastVehicle_SetBrake(plRaycastVehicleHandle vHandle, plReal breakingForce, int wheelIndex);
    void plRaycastVehicle_SetSteeringValue(plRaycastVehicleHandle vHandle, plReal steering, int wheelIndex);

#ifdef __cplusplus
}
#endif

#endif
