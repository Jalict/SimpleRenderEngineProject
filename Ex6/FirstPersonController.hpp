//
// Created by Morten Nobel-Jørgensen on 29/09/2017.
//
#pragma once 

#include <SDL_events.h>
#include "sre/Camera.hpp"
#include <BulletDynamics/Dynamics/btRigidBody.h>



class Wolf3D;
class FirstPersonController {
public:
    FirstPersonController(sre::Camera * camera);
	~FirstPersonController();

    void update(float deltaTime);
    void onKey(SDL_Event& event);
    void onMouse(SDL_Event &event);

    void setInitialPosition(glm::vec2 position, float rotation);
	bool getIsGrounded();

	const float MAX_X_LOOK_ROTATION = 30.0f;
	// TODO move this to camera?s
	const float FIELD_OF_FIELD = 45.0f;
	const float NEAR_PLANE = 0.1f;
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
	const float JUMP_FORCE = 250.0f;
	const float SPRINT_MOVEMENT_INCREASE = 2.0f;
	const float SPRINT_FOV_INCREASE = 1.1f;
   
    bool fwd = false;
    bool bwd = false;
    bool left = false;
    bool right = false;
	bool isGrounded = false;
	bool isSprinting = false;

	btRigidBody* rigidBody;
//	btDefaultMotionState* motionState;
//	btCollisionShape* controllerShape;
};

