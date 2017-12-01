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
	btCollisionShape* controllerShape = new btCapsuleShape(0.1f, 1.0f); // CHECKGROUND is linked to these values 1.2 / 2 = 0.6f;
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

	lookRotation = vec2(0,0);
}


FirstPersonController::~FirstPersonController(){
	// TODO check if done correctly
//	delete rigidBody;
//	delete motionState;
//	delete controllerShape;
}



void FirstPersonController::update(float deltaTime){
	// Determine local movement
	vec3 movement = vec3(0, 0, 0);

	// Only handle movement if we are grounded
	if(isGrounded){
		if(fwd )
			movement += vec3(0, 0, -1);
		if(left)
			movement += vec3(-1, 0, 0);
		if(bwd)
			movement += vec3(0, 0, 1);
		if(right)
			movement += vec3(1, 0, 0);

		// Only normalize if we have some movement.
		// Crashes if we try to normalize (0, 0, 0).
		if (movement != vec3(0, 0, 0))
			movement = glm::normalize(movement);

		movement *= deltaTime;


		// TODO use collider rotations
		// Translate local movement to relative world movement 
		float x = cos(radians(lookRotation.x)) * movement.x - sin(radians(lookRotation.x)) * movement.z;
		float z = cos(radians(lookRotation.x)) * movement.z + sin(radians(lookRotation.x)) * movement.x;

		// Apply movmement
		btVector3 velocity = rigidBody->getLinearVelocity();
		velocity = btVector3(1, 0, 0) * x * MOVEMENT_SPEED + btVector3(0, velocity.getY(), 0) + btVector3(0, 0, 1) * z * MOVEMENT_SPEED;
		rigidBody->setLinearVelocity(velocity);
	}
	
	// Get our position from physics
	btTransform transform;
	rigidBody->getMotionState()->getWorldTransform(transform);
	btVector3 position = transform.getOrigin();

	checkGrounded(position);
	
	auto transformMatrix = mat4();
	transformMatrix = translate(transformMatrix, glm::vec3(position.getX(), position.getY(), position.getZ())); 
	transformMatrix = rotate(transformMatrix, radians(lookRotation.x), vec3(0, -1, 0));
	transformMatrix = rotate(transformMatrix, radians(lookRotation.y), vec3(-1, 0, 0));
	camera->setViewTransform(glm::inverse(transformMatrix));
}


void FirstPersonController::checkGrounded(btVector3 position) {
	btVector3 btTo = position + btVector3(0, -100000, 0);
	btCollisionWorld::ClosestRayResultCallback res(position, btTo);

	Wolf3D::getInstance()->physics.raycast(&position, &btTo, &res);

	// TODO distance check - expansive calculation, change to something more efficient.
	if (res.hasHit()) {
//		std::cout << position.distance(res.m_hitPointWorld) << std::endl;
		isGrounded = !(position.distance(res.m_hitPointWorld) > 0.6f);
	}
//	std::cout << isGrounded << std::endl;
}


// TODO handle two keys pressed (diagonal speed increase) -> normalize movement
void FirstPersonController::onKey(SDL_Event &event) {
	// Capture Jump
	if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE && isGrounded) {
		rigidBody->applyCentralForce(btVector3(0,JUMP_FORCE,0));
	}

	// Capture movement keys down
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
	// Capture movement keys released
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
   if(event.type == SDL_MOUSEMOTION && !lockRotation) {
	   lookRotation.x += event.motion.xrel * ROTATION_SPEED;
	   lookRotation.y += event.motion.yrel * ROTATION_SPEED;
	   lookRotation.y = clamp(lookRotation.y, -MAX_X_LOOK_ROTATION, MAX_X_LOOK_ROTATION);
   }
}


// Set Spawn Position
void FirstPersonController::setInitialPosition(glm::vec2 position, float rotation) {
    this->lookRotation.x = rotation;
	this->lookRotation.y = 0;
	rigidBody->translate(btVector3(position.x, 10.0f, position.y));
}

bool FirstPersonController::getIsGrounded() {
	return isGrounded;
}