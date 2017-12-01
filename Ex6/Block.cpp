#include <iostream>
#include "Block.hpp"

Block::Block() : Block(type) {
}

Block::Block(BlockType type) {
	setType(type);

	mesh = sre::Mesh::create()
		.withCube(1.0f)
		.withUVs(texCoords)
		.build();
}

Block::~Block() {

}

void Block::setType(BlockType type) {
	this->type = type;

	const glm::vec4 coords = textureCoordinates((int)1);
	std::cout << coords.x << " " << coords.y << " " << coords.z << " " << coords.w << std::endl;

	texCoords.clear();

	texCoords.insert(texCoords.end(), {
		// +z
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

		// ?
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

		// ?
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

		// ?
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

		// top
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

		// bottom
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});
}

std::shared_ptr<sre::Mesh> Block::getMesh()
{
	return mesh;
}

//(TODO) Move to somewhere more generic
// FPL, 1 December 2017
glm::vec4 Block::textureCoordinates(int blockID) {
	glm::vec2 textureSize(1024, 2048);
	glm::vec2 tileSize(128, 128);

	float tileWidth = tileSize.x / textureSize.x;
	float tileHeight = tileSize.y / textureSize.y;

	glm::vec2 min = glm::vec2(0, 16 * tileSize.y) / textureSize;
	glm::vec2 max = min - tileSize / textureSize;

	min.x += (blockID % 8) * tileWidth * 2;
	max.x += (blockID % 8) * tileWidth * 2;

	min.y -= ((blockID - (blockID % 8)) / 8) * tileHeight;
	max.y -= ((blockID - (blockID % 8)) / 8) * tileHeight;

	return glm::vec4(min.x, min.y, max.x, max.y);
}