#include "Block.hpp"

Block::Block() {
	type = BlockType::Rock;
}

Block::~Block() {

}

void Block::draw(sre::RenderPass& pass) {
}

void Block::setType(BlockType type) {
	this->type = type;
}