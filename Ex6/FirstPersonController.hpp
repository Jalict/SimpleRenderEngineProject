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

	glm::vec2 lookRotation;
	bool lockRotation = false;
private:
	void checkGrounded(btVector3 position);

    sre::Camera * camera;

	const float ROTATION_SPEED = 0.3f;
	const float MOVEMENT_SPEED = 100.0f;
	const float JUMP_FORCE = 250.0f;
   
    bool fwd = false;
    bool bwd = false;
    bool left = false;
    bool right = false;
	bool isGrounded = false;

	btRigidBody* rigidBody;
//	btDefaultMotionState* motionState;
//	btCollisionShape* controllerShape;
};

