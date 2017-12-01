#include "Block.hpp"

Block::Block() {
	type = BlockType::Rock;

	Block(type);
}

Block::Block(BlockType type) {
	this->type = type;

	mesh->create().withCube(1.0f);
}

Block::~Block() {

}

void Block::setType(BlockType type) {
	this->type = type;
}

std::shared_ptr<sre::Mesh> Block::getMesh()
{
	return mesh;
}

std::shared_ptr<sre::Material> Block::getMaterial()
{
	return material;
}

