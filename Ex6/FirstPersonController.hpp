//
// Created by Morten Nobel-Jørgensen on 29/09/2017.
//
#pragma once 

#include <SDL_events.h>
#include "sre/Camera.hpp"
#include "Block.hpp"
#include <BulletDynamics/Dynamics/btRigidBody.h>



class Wolf3D;
class FirstPersonController {
public:
    FirstPersonController(sre::Camera * camera);
	~FirstPersonController();

    void update(float deltaTime);
    void onKey(SDL_Event& event);
    void onMouse(SDL_Event &event);
	void draw(sre::RenderPass& renderpass);

    void setPosition(glm::vec3 position, float rotation);
	glm::vec3 getPosition();
	bool getIsGrounded();


	Block* castRayForBlock(float normalMultiplier); // normalMultiplayer: Allows you to determine whether the normal should be substracted (to get a block), added (to get an empty location) or the border.

	const float MAX_X_LOOK_ROTATION = 45.0f;
	// TODO move this to camera?s
	const float FIELD_OF_FIELD = 45.0f;
	const float NEAR_PLANE = 0.05f;
	const float FAR_PLANE = 1000.0f;

	glm::vec2 lookRotation;

	bool lockRotation = false;

	// TODO TEMP remove
	glm::vec3 fromRay = glm::vec3(0,0,0);
	glm::vec3 toRay = glm::vec3(0, 0, 0);
	glm::vec3 fromRay1 = glm::vec3(0, 0, 0);
	glm::vec3 toRay2 = glm::vec3(0, 0, 0);
private:
	void checkGrounded(btVector3 position);
	void destroyBlock();
	void placeBlock();

    sre::Camera * camera;

	const float ROTATION_SPEED = 0.3f;
	const float MOVEMENT_SPEED = 150.0f;
	const float JUMP_FORCE = 300.0f;
	const float SPRINT_MOVEMENT_INCREASE = 2.0f;
	const float SPRINT_FOV_INCREASE = 1.1f;
	const float Y_CAMERA_OFFSET = 1.0f;
   
    bool fwd = false;
    bool bwd = false;
    bool left = false;
    bool right = false;
	bool isGrounded = false;
	bool isSprinting = false;

	BlockType blockSelected = BlockType::Dirt;
	glm::mat4 transformMatrix;
	glm::mat4 handBlockOffsetMatrix;


	btRigidBody* rigidBody;
//	btDefaultMotionState* motionState;
//	btCollisionShape* controllerShape;
};

