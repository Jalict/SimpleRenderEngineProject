#include "Chunk.hpp"
#include "Wolf3D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Chunk::Chunk() : Chunk(glm::vec3(0, 0, 0)) { // HACK: This doesn't actually set anything 
}

Chunk::Chunk(glm::vec3 position){
	//Set the position of the chunk
	this->position = position;
	chunkTransform = glm::translate(position);
	
	//For outputting a glm::mat4
	//const float *pSource = (const float*)glm::value_ptr(chunkTransform);
	//for (int i = 0; i < 16; ++i) //Output from upper left corner, one collum at a time
	//	std::cout << pSource[i] << std::endl;
	//std::cout << "END OF MATRIX" << std::endl;

	// Create the blocks
	blocksInChunk = new Block** [chunkDimensions];
	for (int i = 0; i < chunkDimensions; i++){
		blocksInChunk[i] = new Block* [chunkDimensions];
		for (int j = 0; j < chunkDimensions; j++){
			blocksInChunk[i][j] = new Block[chunkDimensions];
		}
	}

	for (int x = 0; x < chunkDimensions; x++){
		for (int y = 0; y < chunkDimensions; y++) {
			for (int z = 0; z < chunkDimensions; z++) {
				blocksInChunk[x][y][z] = Block(BlockType::Dirt, glm::vec3(position.x + x,position.y + y, position.z + z));
			}
		}
	}
}

Chunk::~Chunk(){
	// Delete the blocks
	for (int i = 0; i < chunkDimensions; ++i){
		for (int j = 0; j < chunkDimensions; ++j){
			delete[] blocksInChunk[i][j];
		}
		delete[] blocksInChunk[i];
	}
	delete[] blocksInChunk;
}

void Chunk::update(float dt) {
	//Dunno what to do in here yet
}

void Chunk::draw(sre::RenderPass& renderpass) {
	//Loop over all cubes, get their information, call render
	//Offset blocks by a transformation every time


	for (int x = 0; x < chunkDimensions; x++) {
		for (int y = 0; y < chunkDimensions; y++) {
			for (int z = 0; z < chunkDimensions; z++) {
				auto transformMatrix = glm::translate(chunkTransform, glm::vec3(x, y, z));

				//If the block is not active we don't render it
				if (blocksInChunk[x][y][z].getActive() == false){
					continue;
				}

				//If the block is at the edge of a chunk we always render it
				bool onOutsideOfChunk = false;
				if (!(x > 0) || !(x < chunkDimensions - 1) || !(y > 0) || !(y < chunkDimensions - 1) || !(z > 0) || !(z < chunkDimensions - 1)) {
					onOutsideOfChunk = true;
				}

				//If the block is surrounded by active blocks we don't render it
				bool surrounded = false;
				if (!onOutsideOfChunk) {
					surrounded = blocksInChunk[x - 1][y][z].getActive();
					surrounded = blocksInChunk[x + 1][y][z].getActive();
					surrounded = blocksInChunk[x][y - 1][z].getActive();
					surrounded = blocksInChunk[x][y + 1][z].getActive();
					surrounded = blocksInChunk[x][y][z - 1].getActive();
					surrounded = blocksInChunk[x][y][z + 1].getActive();
				}

				if (onOutsideOfChunk && !surrounded) {
					renderpass.draw(blocksInChunk[x][y][z].getMesh(), transformMatrix, Wolf3D::getInstance()->blockMaterial);
				}
			}
		}
	}
}

glm::vec3 Chunk::getPosition() {
	return position;
}

Block* Chunk::getBlock(int x, int y, int z) {
	assert(x >= 0 && x < chunkDimensions);
	assert(y >= 0 && y < chunkDimensions);
	assert(z >= 0 && z < chunkDimensions);

	return &blocksInChunk[x][y][z];
}