#include "Physics.hpp"



Physics::Physics() {
}


Physics::~Physics() {
	// #TODO Check if done correctly
	delete &broadphase;
	delete &collisionConfiguration;
	delete &dispatcher;
	delete &solver;
	delete &dynamicsWorld;
}


void Physics::init() {
	// Create the physics world
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;
	//	btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);


	// Set gravity for the world
	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	// Attach debug drawer
	dynamicsWorld->setDebugDrawer(&debugDrawer);
}


void Physics::update() {
	dynamicsWorld->stepSimulation(1 / 60.f, 10);
}


void Physics::drawDebug(sre::RenderPass* renderPass) {
	debugDrawer.renderPass = renderPass;
	dynamicsWorld->debugDrawWorld();
}


void Physics::addRigidBody(btRigidBody* rigidbody) {
	dynamicsWorld->addRigidBody(rigidbody);
}


void Physics::setDebugDrawMode(btIDebugDraw::DebugDrawModes mode){
	debugDrawer.setDebugMode(mode);
}


void Physics::raycast(btVector3* from, btVector3* to, btCollisionWorld::ClosestRayResultCallback* result){
	 dynamicsWorld->rayTest(*from, *to, *result);
}


