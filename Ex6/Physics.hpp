/*
* Physics - Created: 30-11-2017
* Wrapper for all the bullet physics.
*/
#pragma once

#include <btBulletDynamicsCommon.h>
#include "btDebugDrawer.hpp"



class Physics {
public:
	Physics();
	~Physics();

	void init();
	void drawDebug(sre::RenderPass* renderPass);
	void update();

	void addRigidBody(btRigidBody* rigidbody);		// Adds the rigidbody to the physics world.
	void removeRigidBody(btRigidBody* rigidbody);	// Removes the rigidbody form the physics world.

	void setDebugDrawMode(btIDebugDraw::DebugDrawModes mode);	// Set the debug mode, so you can debug draw colliders.

	void raycast(btVector3* from, btVector3* to, btCollisionWorld::ClosestRayResultCallback* result);	// Regular raycast.
private:
	btBroadphaseInterface*	broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	
	btDebugDrawer debugDrawer;
};

