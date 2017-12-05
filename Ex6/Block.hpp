#pragma once

#include "sre/SDLRenderer.hpp"
#include "btBulletDynamicsCommon.h"

/*
Created: 31-11-2017
A block in the world 
(NOTE) Frans calls them Voxels
*/

class Wolf3D;
class Block {
public:
	enum BlockType { Stone = 0, Brick = 1, Grass = 4, WoolBlue = 5, Sand = 8, Dirt = 9, Gravel = 25, Rock = 50, Wood = 74, Planks = 83 }; // Types of blocks

	Block();
	Block(BlockType type, glm::vec3 position);
	~Block();

	void setType(BlockType type);				// Change type of block
	std::shared_ptr<sre::Mesh> getMesh();		// Get Mesh

	glm::vec3 position;
private:
	btRigidBody* createRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape);

	BlockType type = BlockType::Dirt;			// Current type of block

	std::shared_ptr<sre::Mesh> mesh;			// Mesh of block
	std::vector<glm::vec4> texCoords;	// texCoords for block

	glm::vec4 textureCoordinates(int blockID);
	glm::vec3 getPosition();
};