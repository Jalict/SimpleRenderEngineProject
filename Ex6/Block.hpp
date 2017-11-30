#pragma once

#include "sre/SDLRenderer.hpp"

/*
Created: 31-11-2017
A block in the world 
(NOTE) Frans calls them Voxels
*/

class Block {
private:
	enum BlockType {Grass, Rock};
	BlockType type;

public:
	Block();
	~Block();

	void draw(sre::RenderPass& renderpass);
	void setType(BlockType type);
};

enum BlockType {
	Grass,
	Rock
};