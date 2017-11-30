//
// Created by Morten Nobel-Jørgensen on 29/09/2017.
//
#include "FirstPersonController.hpp"
#include <glm/gtx/rotate_vector.hpp>
#include "Wolf3D.hpp"

using namespace sre;
using namespace glm;



FirstPersonController::FirstPersonController(sre::Camera * camera)
:camera(camera) {
    camera->setPerspectiveProjection(45,0.1f,1000);

	// Create Capsule collider
	btCollisionShape* controllerShape = new btCapsuleShape(0.1f, 1.0f);
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(2, 50, 0)));

	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	controllerShape->calculateLocalInertia(mass, fallInertia);

	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, motionState, controllerShape, fallInertia);
	rigidBody = new btRigidBody(fallRigidBodyCI);

	// Add rigidbody to world
	Wolf3D::getInstance()->physics.addRigidBody(rigidBody);

	// Only allow for rotations around the Y-axis
	rigidBody->setAngularFactor(btVector3(0, 1, 0));
}


FirstPersonController::~FirstPersonController(){
	// #TODO check if done correctly
//	delete rigidBody;
//	delete motionState;
//	delete controllerShape;
}



void FirstPersonController::update(float deltaTime){
	// Determine local movement
	vec3 movement = vec3(0, 0, 0); 

	if(fwd)
		movement += vec3(0, 0, -1);
	if(left)
		movement += vec3(-1, 0, 0);
	if(bwd)
		movement += vec3(0, 0, 1);
	if(right)
		movement += vec3(1, 0, 0);

	movement *= deltaTime;

	// #TODO use collider rotations
	// Translate local movement to relative world movement 
	float x = cos(radians(rotation)) * movement.x - sin(radians(rotation)) * movement.z;
	float z = cos(radians(rotation)) * movement.z + sin(radians(rotation)) * movement.x;

	// Apply movmement
	btVector3 velocity = rigidBody->getLinearVelocity();
	velocity = btVector3(1,0,0) * x * MOVEMENT_SPEED + btVector3(0, velocity.getY(),0) + btVector3(0,0,1) * z * MOVEMENT_SPEED;
	rigidBody->setLinearVelocity(velocity);
	
	// Get our position from physics
	btTransform transform;
	rigidBody->getMotionState()->getWorldTransform(transform);
	btVector3 position = transform.getOrigin();

	auto transformMatrix = mat4();
	transformMatrix = translate(transformMatrix, glm::vec3(position.getX(), position.getY(), position.getZ())); 
	transformMatrix = rotate(transformMatrix, radians(rotation), vec3(0, -1, 0));
	camera->setViewTransform(glm::inverse(transformMatrix));
}


// #TODO handle two keys pressed (diagonal speed increase) -> normalize movement
void FirstPersonController::onKey(SDL_Event &event) {
    if(event.type == SDL_KEYDOWN ){
		switch (event.key.keysym.sym){
			case SDLK_w:
				fwd = true;
				break;
			case SDLK_a:
				left = true;
				break;
			case SDLK_s:
				bwd = true;
				break;
			case SDLK_d:
				right = true;
				break;
		}
	}

	if (event.type == SDL_KEYUP) {
		switch (event.key.keysym.sym) {
			case SDLK_w:
				fwd = false;
				break;
			case SDLK_a:
				left = false;
				break;
			case SDLK_s:
				bwd = false;
				break;
			case SDLK_d:
				right = false;
				break;
			}
	}
}


// Handle Mouse Events
void FirstPersonController::onMouse(SDL_Event &event) {
   if(event.type == SDL_MOUSEMOTION) {
	   rotation += event.motion.xrel * ROTATION_SPEED;
   }
}


// Set Spawn Position
void FirstPersonController::setInitialPosition(glm::vec2 position, float rotation) {
    this->rotation = rotation;
	rigidBody->translate(btVector3(position.x, 10.0f, position.y));
}
