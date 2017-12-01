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
	void addRigidBody(btRigidBody* rigidbody);
	void setDebugDrawMode(btIDebugDraw::DebugDrawModes mode);
	void raycast(btVector3* from, btVector3* to, btCollisionWorld::ClosestRayResultCallback* result);
private:
	btBroadphaseInterface*	broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	
	btDebugDrawer debugDrawer;
};

