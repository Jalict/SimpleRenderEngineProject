#pragma once

#include "sre/SDLRenderer.hpp"

/*
Created: 31-11-2017
A block in the world 
(NOTE) Frans calls them Voxels
*/

class Block {
private:
	enum BlockType { Rock, Grass };
	BlockType type;

	std::shared_ptr<sre::Mesh> mesh;
	std::shared_ptr<sre::Material> material;
public:
	Block();
	Block(BlockType type);
	~Block();

	void setType(BlockType type);

	std::shared_ptr<sre::Mesh> getMesh();
	std::shared_ptr<sre::Material> getMaterial();
};