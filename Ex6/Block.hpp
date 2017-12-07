#pragma once

#include "sre/SDLRenderer.hpp"
#include "btBulletDynamicsCommon.h"


/*
Created: 31-11-2017
A block in the world 
(NOTE) Frans calls them Voxels
*/

// enum BlockType { Stone = 0, Brick = 1, Grass = 4, WoolBlue = 5, Sand = 8, Dirt = 9, Gravel = 25, Rock = 50, Wood = 74, Planks = 83 }; // Types of blocks
enum BlockType { Stone, Brick, Grass, WoolBlue, Sand, Dirt, Gravel, Rock, Wood, Planks, LENGTH }; // Types of blocks
enum BlockSides {Top, Bottom, Left, Right, Front, Back };

class Wolf3D;
class Block {
public:

	Block();
	Block(BlockType type, glm::vec3 position);
	~Block();

	void setType(BlockType type);				// Change type of block
	std::shared_ptr<sre::Mesh> getMesh();		// Get Mesh
//	static std::string typeToString(BlockType type);
	static int getTextureIndex(BlockType type, BlockSides side = BlockSides::Top);

	glm::vec3 getPosition();
	bool getActive();
	void setActive(bool active);
private:
	btRigidBody* createRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape);
	
	btRigidBody* rigidbody;
	btBoxShape* collider;

	void setMesh(BlockType type);
	BlockType type = BlockType::Dirt;			// Current type of block

	bool active = true;

	std::shared_ptr<sre::Mesh> mesh;			// Mesh of block
	glm::vec3 position;
};


