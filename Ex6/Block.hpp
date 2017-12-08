#pragma once

#include "sre/SDLRenderer.hpp"
#include "btBulletDynamicsCommon.h"


/*
Created: 31-11-2017
A block in the world 
(NOTE) Frans calls them Voxels
*/

// Types of blocks
enum BlockType { Stone, Brick, Grass, Dirt, Gravel, Rock, Wood, Planks, Bedrock, Glass, WorkBench, IronOre, CoalOre, DiamondOre, LENGTH }; 
// Faces of a cube
enum BlockSides {Top, Bottom, Left, Right, Front, Back };

class Wolf3D;
class Block {
public:
	Block();
	Block(BlockType type, glm::vec3 position);
	~Block();

	// Returns the texture coordinates depending on type and side requested
	static int getTextureIndex(BlockType type, BlockSides side = BlockSides::Top);

	void addColliderToWorld();
	void removeColliderFromWorld();

	// Sets the type of this block
	void setType(BlockType type);				
	void setActive(bool active);
	bool isActive();
	BlockType getType() { return type; }
	glm::vec3 getPosition();
private:
	btRigidBody* createRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape);
	
	btRigidBody* rigidbody;
	btBoxShape* collider;

	// Current type of block
	BlockType type = BlockType::Dirt;			
	
	// Wether the block exists or not
	bool active = true;

	// World position of the block
	glm::vec3 position;
};


