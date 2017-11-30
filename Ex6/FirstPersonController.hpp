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

    float rotation = 0;
private:
    sre::Camera * camera;

	const float ROTATION_SPEED = 0.3f;
	const float MOVEMENT_SPEED = 100.0f;
   
    bool fwd = false;
    bool bwd = false;
    bool left = false;
    bool right = false;

	btRigidBody* rigidBody;
//	btDefaultMotionState* motionState;
//	btCollisionShape* controllerShape;
};

