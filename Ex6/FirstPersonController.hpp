/*
* Initially Created by Morten Nobel-Jørgensen on 29/09/2017.
* 
* FirstPersonController - included in project from: 30-11-2017
* Wrapper for all the bullet physics.
*/
#pragma once 

#include "Block.hpp"
#include "sre/Camera.hpp"
#include <SDL_events.h>
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

    void setLockRotation(bool lockRotation);
	void translateController(glm::vec3 position, float rotation);	// Translates the rigidbody with the position

	glm::vec3 getPosition();							// Get the position based on the collider
	glm::vec3 getLookAt() { return toRay; }				// Get the position the camera is currently looking at
	bool getIsGrounded() { return isGrounded; }	
	float getMinedAmount() { return minedAmount; }	
private:
	void checkGrounded(btVector3 position); // Checks whether the controller is grounded.
	void destroyBlock(Block* block);		// Destroys the block
	void placeBlock();						// Places a block

	// Does a raycast from the controller to the center of the screen and returns the block the FPS controller is looking at.
	// normalMultiplier: Allows you to determine whether the normal should be substracted (to get a block), added (to get an empty location) or the border.
	Block* castRayForBlock(float normalMultiplier); 

    sre::Camera * camera;				// Camera that the FPScontroller is attached to
	btRigidBody* rigidBody;				// Rigidbody of the FPS controller
	btCollisionShape* controllerShape;	// Collider of the controller

	const float ROTATION_SPEED = 0.3f;				// Value mouse movement is multiplied by to rotate the controller
	const float MAX_X_LOOK_UP_ROTATION = 45.0f;		// Max angle the controller can look up
	const float MAX_X_LOOK_DOWN_ROTATION = 80.0f;	// Max angle the controller can look down

	const float MOVEMENT_SPEED = 200.0f;			// Value the movement is multiplied by per frame
	const float JUMP_FORCE = 320.0f;				// Force applied when the controller jumps
	const float JUMP_MOVEMENT_MULTIPLIER = 0.8f;	// Percentage of movement speed the character has whilst mid air
	const bool NEEDS_GROUNDED_TO_MOVE = false;		// When enabled the controller cannot move mid air
	const float SPRINT_MOVEMENT_INCREASE = 2.0f;	// Multiplier of movement speed when the controller is sprinting
	const float SPRINT_FOV_INCREASE = 1.1f;			// Multiplier of FOV increase of when the character starts sprinting

	const float COLLIDER_HEIGHT = 1.7f;	// Height of the character controller capsule
	const float COLLIDER_RADIUS = .1f;	// Radius of the character controller capsule.

	const float MINE_RANGE = 10.0f;	// Length the raycast should check, essentially the distance that a block can be mined from or placed.

	const float Y_CAMERA_OFFSET = 0.7f;	// Offset the camera has from the center origin of the capsule collider
	const float FIELD_OF_VIEW = 45.0f;	// Field of view of the camera
	const float NEAR_PLANE = 0.05f;		// Distance to near plane for the camera
	const float FAR_PLANE = 1000.0f;	// Distance to the far plane for the camera
   
	// Movement
    bool fwd = false;
    bool bwd = false;
    bool left = false;
    bool right = false;
	bool up = false;
	bool down = false;

	// States
	bool isGrounded = false;	// Wether the controller is grounded
	bool isSprinting = false;	// Wether the character is sprinting
	bool isMining = false;		// Whether the controller is mining

	// Toggle modes
	bool replaceBlock = false;  // When enabled, replaces block instead of placing new blocks
	bool ghostMode = false;		// When enabled the controller can walk through collidables
	bool flyMode = false;		// When enabled the controller is no longer affected by gravity and can fly	
	bool lockRotation = false;	// When enabled the character cannot rotate
	bool drawLookRays = true;	// When enabled the raycasts for detemining the block you are looking at are drawn.
	bool instantMining = false;	// When enabled blocks will be mined instantly

	// Positional
	glm::mat4 transformMatrix;	// Transform matrix for the location and rotations of this controller
	glm::vec2 lookRotation;		// Look rotations of the controller, in euler angles in degrees

	// Variables for the block that the controller holds in its hand.
	BlockType blockSelected = BlockType::Dirt;	// The type currently selected
	glm::mat4 handBlockOffsetMatrix;			// Matrix to go from the FPS transform matrix to the hand

	// Delays on mining
	float minedAmount = 0;	// Duration the player has been mining this block
	Block* lastBlock;		// Last block that was attempted to be mined

	// Used to store the locations for debug drawing the lines of the last raycast
	glm::vec3 fromRay = glm::vec3(0, 0, 0);
	glm::vec3 toRay = glm::vec3(0, 0, 0);
	glm::vec3 fromRayNormal = glm::vec3(0, 0, 0);
	glm::vec3 toRayNormal = glm::vec3(0, 0, 0);
};

