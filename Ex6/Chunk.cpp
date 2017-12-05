#include "Chunk.hpp"
#include "Wolf3D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Chunk::Chunk(glm::mat4 chunkTransform){
	//Set the position of the chunk
	this->chunkTransform = chunkTransform;
	
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
	//#IMPORTANT: We asume that the block size is 2.0f

	for (int x = 0; x < chunkDimensions; x++) {
		for (int y = 0; y < chunkDimensions; y++) {
			for (int z = 0; z < chunkDimensions; z++) {
				auto transformMatrix = glm::translate(chunkTransform, glm::vec3(x, y, z)); 

				renderpass.draw(blocksInChunk[x][y][z].getMesh(), transformMatrix, Wolf3D::getInstance()->blockMaterial);
			}
		}
	}
}