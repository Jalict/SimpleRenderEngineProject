#pragma once

#include "sre/SDLRenderer.hpp"

/*
Created: 31-11-2017
A block in the world 
(NOTE) Frans calls them Voxels
*/

class Block {
public:
	enum BlockType { Rock, Grass };				// Types of blocks

	Block();
	Block(BlockType type, glm::vec3 position);
	~Block();

	void setType(BlockType type);				// Change type of block
	std::shared_ptr<sre::Mesh> getMesh();		// Get Mesh

	glm::vec3 position;
private:
	BlockType type = BlockType::Rock;			// Current type of block

	std::shared_ptr<sre::Mesh> mesh;			// Mesh of block
	std::vector<glm::vec4> texCoords;	// texCoords for block

	glm::vec4 textureCoordinates(int blockID);
	glm::vec3 getPosition();
};