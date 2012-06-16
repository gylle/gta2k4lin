/*
 * Extend the not-that-amazing bullet C-Api a bit.
 *
 * Copyright David Hedberg  2001,2012
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

#include "btBulletDynamicsCommon.h"

#include "Bullet-C-Api.h"
#include "btwrap.h"

// virtual void 	getWorldTransform (btTransform &worldTrans) const =0
// virtual void 	setWorldTransform (const btTransform &worldTrans)=0


class MotionStateCallback : public btMotionState {
private:
    get_world_transform_cb _gcb;
    set_world_transform_cb _scb;
    void *_ptr;

public:
    MotionStateCallback(get_world_transform_cb gcb, set_world_transform_cb scb, void *ptr) {
        _gcb = gcb;
        _scb = scb;
        _ptr = ptr;
    };

    void getWorldTransform(btTransform &worldTrans) const {
        plVector3 origin;
        plReal basis[16];
        memset(basis, 0, sizeof(basis));

        _gcb(_ptr, origin, basis);

        printf("got [%f, %f, %f]\n", origin[0], origin[1], origin[2]);

        btMatrix3x3 newBasis;
        newBasis.setIdentity();
        newBasis.setFromOpenGLSubMatrix(basis);
        worldTrans.setBasis(newBasis);

        btVector3 newOrigin(origin[0], origin[1], origin[2]);
        worldTrans.setOrigin(newOrigin);
    }

    void setWorldTransform(const btTransform &worldTrans) {
        btMatrix3x3 newBasis = worldTrans.getBasis();
        btVector3 newOrigin = worldTrans.getOrigin();

        plVector3 origin;
        origin[0] = newOrigin[0];
        origin[1] = newOrigin[1];
        origin[2] = newOrigin[2];

        plReal glbasis[16];
        memset(glbasis, 0, sizeof(glbasis)); /* ? */
        newBasis.getOpenGLSubMatrix(glbasis);

        _scb(_ptr, origin, glbasis);
    };
};


plRigidBodyHandle plCreateRigidBodyWithCallback(void* user_data, float mass, plCollisionShapeHandle cshape,
                                                get_world_transform_cb gcb, set_world_transform_cb scb,
                                                void *ptr)
{
	btVector3 localInertia(0,0,0);
	btCollisionShape* shape = reinterpret_cast<btCollisionShape*>( cshape);
	btAssert(shape);
	if (mass)
	{
		shape->calculateLocalInertia(mass,localInertia);
	}

        /* btMotionState callback */
        MotionStateCallback *motionStateCallback;
        motionStateCallback = new MotionStateCallback(gcb, scb, ptr);

	void* mem = btAlignedAlloc(sizeof(btRigidBody),16);
	btRigidBody::btRigidBodyConstructionInfo rbci(mass, motionStateCallback, shape, localInertia);
	btRigidBody* body = new (mem)btRigidBody(rbci);
	body->setUserPointer(user_data);
	return (plRigidBodyHandle) body;
}

void plRigidBody_ApplyForce(plRigidBodyHandle object, plVector3 force, plVector3 rel_pos)
{
    btRigidBody *body = reinterpret_cast<btRigidBody*>(object);
    btAssert(body);

    body->applyForce(btVector3(force[0], force[1], force[2]),
                     btVector3(rel_pos[0], rel_pos[1], rel_pos[2]));
}

void plDynamicsWorld_SetGravity(plDynamicsWorldHandle handle, plReal x, plReal y, plReal z) {
    btDynamicsWorld* dynamicsWorld = reinterpret_cast< btDynamicsWorld* >(handle);

    dynamicsWorld->setGravity(btVector3(x, y, z));
}
