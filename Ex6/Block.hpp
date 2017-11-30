#pragma once

#include "sre/SDLRenderer.hpp"

class Block {
public:
	Block();
	~Block();

	void draw(sre::RenderPass& renderpass);
	void setType(BlockType type);
private:
	BlockType type;

};

enum BlockType {
	Grass,
	Rock
};