#pragma once
#include "Block.hpp"
#include "sre/SDLRenderer.hpp"
#include <glm\gtx\rotate_vector.hpp>

/*
Created: 01-12-2017
Holds a list of blocks that lie in 5x5x5 cube. It can draw these blocks by taking information from the 
blocks and (some other class that contains the mesh/material of the blocks). It can also update which
blocks are active and which are not.
*/

class Chunk {
public:
	Chunk(glm::mat4 chunkTransform);
	~Chunk();
	int getChunkDimensions() { return chunkDimensions; }

	void update(float dt);
	void draw(sre::RenderPass& renderpass);
	
private:
	//(Upper, front, left corner of chunk)
	glm::mat4 chunkTransform;

	Block* testBlock;
	Block*** blocksInChunk;
	const int chunkDimensions = 5;
};