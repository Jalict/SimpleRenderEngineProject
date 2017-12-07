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
	// # TODO PRIORITY fix controller shape size without getting stuck..
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

	// Set local block matrix to display the block in the bottom right corner in front of the camera
	handBlockOffsetMatrix = translate(mat4(), vec3(0.5f, -.5f, -1));
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
	transformMatrix = mat4();
	transformMatrix = translate(transformMatrix, glm::vec3(position.getX(), position.getY() + Y_CAMERA_OFFSET, position.getZ())); 
	transformMatrix = rotate(transformMatrix, radians(lookRotation.x), vec3(0, -1, 0));
	transformMatrix = rotate(transformMatrix, radians(lookRotation.y), vec3(-1, 0, 0));
	camera->setViewTransform(glm::inverse(transformMatrix));
}


void FirstPersonController::draw(sre::RenderPass& renderpass) {
	// Translate to  the correct position ( controller position + the block offset for the hand)
	auto matrix = transformMatrix * handBlockOffsetMatrix;
	// Scale the block to 35%
	matrix = scale(matrix, vec3(0.35f, 0.35f, 0.35f));
	// Rotate it 45 degrees around y
	matrix = rotate(matrix, radians(45.0f), vec3(0, -1, 0));
	
	// Draw the block we currently have selected
	renderpass.draw(Wolf3D::getInstance()->getMesh(blockSelected), matrix, Wolf3D::getInstance()->blockMaterial);
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
		isGrounded = !(position.distance(res.m_hitPointWorld) > 0.9f); // 0l.6f is the height of capsule collider
	}
	else {
		isGrounded = false;
	}
}


void FirstPersonController::onKey(SDL_Event &event) {
	// Reset character position if HOME is pressed
	if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_HOME) {
		setPosition(glm::vec3(3, 8, 3), 0);
	}

	// Capture Jump
	if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE && isGrounded) {
		rigidBody->applyCentralForce(btVector3(0,JUMP_FORCE,0));
	}

	if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_RIGHT && isGrounded) {
		// Increase block selected
		blockSelected = (BlockType)(blockSelected + 1);

		// If we have the last block selected, go back to the start
		if (blockSelected == BlockType::LENGTH)
			blockSelected = BlockType::Stone;
	}

	if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LEFT && isGrounded) {
		// If we are at the end, go back to the start
		if (blockSelected == BlockType::Stone)
			blockSelected = BlockType::LENGTH;

		// Decrease block selected
		blockSelected = (BlockType)(blockSelected - 1);
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



// TODO clean up
void FirstPersonController::destroyBlock() {
	std::cout << "destroying" << std::endl;
	auto block = castRayForBlock(-0.2f);

	if(block != nullptr)
		block->setActive(false);
}


void FirstPersonController::placeBlock() {
	std::cout << "placing" << std::endl;

	auto block = castRayForBlock(0.2f);

	if (block != nullptr) {
		block->setType(blockSelected);
		block->setActive(true);
	}
	
}


Block* FirstPersonController::castRayForBlock(float normalMultiplier) {
	btVector3 start = rigidBody->getWorldTransform().getOrigin();
	start.setY(start.getY() + Y_CAMERA_OFFSET);
	btVector3 direction = btVector3(sin(radians(lookRotation.x)), sin(radians(lookRotation.y)) * -1, cos(radians(lookRotation.x)) * -1);
	btVector3 end = start + direction * 10.0f;

	btCollisionWorld::ClosestRayResultCallback res(start, end);

	// Cast ray
	Wolf3D::getInstance()->physics.raycast(&start, &end, &res);

	// If we have an hit handle it, else return null
	if (res.hasHit()) {
		// Store hit location
		btVector3 hit = res.m_hitPointWorld;

		// TODO TEMP prob remove below -- added for debug purposes
		toRay = vec3(hit.getX(), hit.getY(), hit.getZ());
		toRay2 = toRay + vec3(res.m_hitNormalWorld.getX(), res.m_hitNormalWorld.getY(), res.m_hitNormalWorld.getZ()); //res.m_hitNormalWorld;
		fromRay1 = toRay;
		fromRay = vec3(start.getX(), start.getY(), start.getZ());

		// Compensate for origin of block, collider origin is in center, though for the math we want it to be on a corner
		hit += btVector3(0.501f, 0.501f, 0.501f);

		// Every block is above or at 0. If we hit the bedrock it could be that we get below 0. So make really sure we always grab blocks on atleast 0
		if (hit.getY() < 0)
			hit.setY(0);

		// Add some of  the hit normal. The raycast always hits the edge of the collider. This increases accuracy since we move into the collider or out.
		hit += res.m_hitNormalWorld * normalMultiplier;

		// Grab the block, and set it to not active - all numbers are floored since blocks take up a whole unit
		return Wolf3D::getInstance()->locationToBlock((int)hit.getX(), (int)hit.getY(), (int)hit.getZ());
	} else{
		return nullptr;
	}
}


// Set Spawn Position
void FirstPersonController::setPosition(glm::vec3 position, float rotation) {
    this->lookRotation.x = rotation;
	this->lookRotation.y = 0;
	rigidBody->translate(btVector3(position.x, position.y, position.z));
}


bool FirstPersonController::getIsGrounded() {
	return isGrounded;
}