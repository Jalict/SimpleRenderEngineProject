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
	// Setup  Camera projection
    camera->setPerspectiveProjection(FIELD_OF_FIELD, NEAR_PLANE, FAR_PLANE);

	// Create Capsule collider
	btCollisionShape* controllerShape = new btCapsuleShape(0.1f, 1.0f); // CHECKGROUND is linked to these values 1.2 / 2 = 0.6f;
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	controllerShape->calculateLocalInertia(mass, fallInertia);

	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, motionState, controllerShape, fallInertia);
	rigidBody = new btRigidBody(fallRigidBodyCI);
	rigidBody->setActivationState(DISABLE_DEACTIVATION);

	// Add rigidbody to world
	Wolf3D::getInstance()->physics.addRigidBody(rigidBody);

	// Only allow for rotations around the Y-axis
	rigidBody->setAngularFactor(btVector3(0, 1, 0));

	// Set initial look rotation to 0, 0
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
		if(fwd)
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

		if(isSprinting)
			movement *= SPRINT_MOVEMENT_INCREASE;	

		// Multiply by time that has passed to compensate for FPS
		movement *= deltaTime;


		// TODO use collider rotations? Is this necessary?
		// Translate local movement to relative world movement 
		float x = cos(radians(lookRotation.x)) * movement.x - sin(radians(lookRotation.x)) * movement.z;
		float z = cos(radians(lookRotation.x)) * movement.z + sin(radians(lookRotation.x)) * movement.x;

		// Apply movmement
		btVector3 velocity = rigidBody->getLinearVelocity();
		velocity = btVector3(x * MOVEMENT_SPEED, velocity.getY(), z * MOVEMENT_SPEED); // Carry falling speed to our current movement
		rigidBody->setLinearVelocity(velocity);
	}
	
	// Get our position from physics
	btTransform transform;
	rigidBody->getMotionState()->getWorldTransform(transform);
	btVector3 position = transform.getOrigin();

	// Check if the controller is grounded
	checkGrounded(position);
	
	// Update our tranform matrix, pass it on to the camera
	auto transformMatrix = mat4();
	transformMatrix = translate(transformMatrix, glm::vec3(position.getX(), position.getY(), position.getZ())); 
	transformMatrix = rotate(transformMatrix, radians(lookRotation.x), vec3(0, -1, 0));
	transformMatrix = rotate(transformMatrix, radians(lookRotation.y), vec3(-1, 0, 0));
	camera->setViewTransform(glm::inverse(transformMatrix));
}

glm::vec3 FirstPersonController::getPosition() {
	btTransform transform;
	rigidBody->getMotionState()->getWorldTransform(transform);
	btVector3 position = transform.getOrigin();
	return glm::vec3(position.getX(), position.getY(), position.getZ());
}


void FirstPersonController::checkGrounded(btVector3 position) {
	// Cast a ray from our position to a location far below it
	btVector3 down = position + btVector3(0, -100000, 0);
	btCollisionWorld::ClosestRayResultCallback res(position, down);

	Wolf3D::getInstance()->physics.raycast(&position, &down, &res);

	// If we hit something, check the distance to the ground
	// TODO distance check - expansive calculation, change to something more efficient.
	if (res.hasHit()) {
		isGrounded = !(position.distance(res.m_hitPointWorld) > 0.6f); // 0l.6f is the height of capsule collider
	}
	else {
		isGrounded = false;
	}
}


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
			case SDLK_LSHIFT:
				if(!isGrounded)
					return;

				isSprinting = true;
				camera->setPerspectiveProjection(FIELD_OF_FIELD * SPRINT_FOV_INCREASE, NEAR_PLANE, FAR_PLANE);
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
			case SDLK_LSHIFT:
				camera->setPerspectiveProjection(FIELD_OF_FIELD, NEAR_PLANE, FAR_PLANE);
				isSprinting = false;
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
	
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		if (event.button.button == SDL_BUTTON_LEFT) {
			destroyBlock();
		}
		else if(event.button.button == SDL_BUTTON_RIGHT) {
			placeBlock();
		}
	}
}

void FirstPersonController::destroyBlock() {
	std::cout << "destroying" << std::endl;

	// TODO make sure this below is correct
	Physics* physics = &Wolf3D::getInstance()->physics;

	btVector3 position = rigidBody->getWorldTransform().getOrigin();
	btVector3 forward = btVector3(sin(radians(lookRotation.x)), sin(radians(lookRotation.y)) * -1, cos(radians(lookRotation.x)) * -1);
	btVector3 to = position + forward * 10.0f;

	btCollisionWorld::ClosestRayResultCallback res(position, to);

	physics->raycast(&position, &to, &res);
	
	// TODO TEMP prob remove below
	fromRay = vec3(position.getX(), position.getY(), position.getZ());
	//toRay = vec3(to.getX(), to.getY(), to.getZ());

	if (res.hasHit()) {
		btVector3 hit = res.m_hitPointWorld;

		toRay = vec3(hit.getX(), hit.getY(), hit.getZ());
		toRay2 = toRay + vec3(res.m_hitNormalWorld.getX(), res.m_hitNormalWorld.getY(), res.m_hitNormalWorld.getZ()); //res.m_hitNormalWorld;
		fromRay1 = toRay;

		hit -= res.m_hitNormalWorld * 0.2f;

		std::cout << hit.getX() << " - " << hit.getY() << " - " << hit.getZ() << std::endl;
		auto t = vec3((int)hit.getX(), (int)hit.getY(), (int)hit.getZ());
		auto block = Wolf3D::getInstance()->locationToBlock(t);
		block->setActive(false);
	}
	
}


void FirstPersonController::placeBlock() {
	std::cout << "placing" << std::endl;
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