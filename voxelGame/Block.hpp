/*
* Block - Created: 31-11-2017
* This class represents a block in the world. All edges of a block are one unit.
*/
#pragma once

#include "btBulletDynamicsCommon.h"
#include "sre/SDLRenderer.hpp"



// Types of blocks
enum BlockType { Stone, Brick, Grass, Dirt, Gravel, Rock, Wood, Planks, Bedrock, Glass, WorkBench, IronOre, CoalOre, DiamondOre, LENGTH }; 

// Faces of a cube
enum BlockSides {Top, Bottom, Left, Right, Front, Back };


class Game;
class Block {
public:
	Block();
	Block(BlockType type, glm::vec3 position);
	~Block();

	// This returns the correct texture index for a block type and the correct side.
	static int getTextureIndex(BlockType type, BlockSides side = BlockSides::Top);

	void addColliderToWorld();			// Adds the rigidbody of this box to the physics world.
	void removeColliderFromWorld();		// Removes the rigidbody of this box from the physics world.
	void initCollider();				// Initializes the collider for this box.

	void setType(BlockType type);				
	void setActive(bool active);
	bool isActive() { return active; }
	BlockType getType() { return type; }
	glm::vec3 getPosition() { return position; }
private:
	btRigidBody* rigidbody = nullptr;	// Rigidbody of this block
	btBoxShape* collider =  nullptr;	// The cube collider of this block

	bool active = true;					// Whether the block is active (displayed / exists in the world).
	bool inPhysicsWorld = false;		// Whether the rigidbody has been added to the physics world.
	BlockType type = BlockType::Dirt;	// Current type of block.
	glm::vec3 position;					// World position of this block
};