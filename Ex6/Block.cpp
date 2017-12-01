#include "Block.hpp"

Block::Block() {
	type = BlockType::Rock; // Default block type

	Block(type);
}

Block::Block(BlockType type) {
	setType(type);

	mesh->create().withCube(1.0f).withUVs(texCoords).build();
}

Block::~Block() {

}

void Block::setType(BlockType type) {
	this->type = type;

	glm::vec4 coords = textureCoordinates((int)type);

	texCoords.clear();

	texCoords.insert(texCoords.end(), {
		glm::vec4(0,0,0,0), glm::vec4(0,0,0,0), glm::vec4(0,0,0,0),
		glm::vec4(0,0,0,0), glm::vec4(0,0,0,0), glm::vec4(0,0,0,0),

		glm::vec4(0,0,0,0), glm::vec4(0,0,0,0), glm::vec4(0,0,0,0),
		glm::vec4(0,0,0,0), glm::vec4(0,0,0,0), glm::vec4(0,0,0,0),

		glm::vec4(0,0,0,0), glm::vec4(0,0,0,0), glm::vec4(0,0,0,0),
		glm::vec4(0,0,0,0), glm::vec4(0,0,0,0), glm::vec4(0,0,0,0),

		glm::vec4(0,0,0,0), glm::vec4(0,0,0,0), glm::vec4(0,0,0,0),
		glm::vec4(0,0,0,0), glm::vec4(0,0,0,0), glm::vec4(0,0,0,0)
	});
}

std::shared_ptr<sre::Mesh> Block::getMesh()
{
	return mesh;
}

//(TODO) Move to somewhere more generic and make it static
// FPL, 1 December 2017
glm::vec4 Block::textureCoordinates(int blockID) {
	glm::vec2 textureSize(1024, 2048);
	glm::vec2 tileSize(128, 128);

	float tileWidth = tileSize.x / textureSize.x;
	float tileHeight = tileSize.y / textureSize.y;

	glm::vec2 min = glm::vec2(0, 16 * tileSize.y) / textureSize;
	glm::vec2 max = min + tileSize / textureSize;

	min.x += (blockID % 8) * tileWidth * 2;
	max.x += (blockID % 8) * tileWidth * 2;

	min.y -= ((blockID - (blockID % 8)) / 8) * tileHeight;
	max.y -= ((blockID - (blockID % 8)) / 8) * tileHeight;

	return glm::vec4(min.x, min.y, max.x, max.y);
}