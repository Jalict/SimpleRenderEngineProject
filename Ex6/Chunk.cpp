#include "Chunk.hpp"

Chunk::Chunk(glm::mat4 chunkTransform){

	//testBlock = new Block();

	//Set the position of the chunk (Upper, front, left corner, hopefully)
	this->chunkTransform = chunkTransform;

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

	//glm::translate(vec3(10.0f, 0.0f, -10.0f));
	//renderpass.draw(floor, floorTransform, floorMat);

	//#IMPORTANT: We asume that the block size is 1.0f

	glm::mat4 chunkTransformX = chunkTransform;
	glm::mat4 chunkTransformY = chunkTransform;
	glm::mat4 chunkTransformZ = chunkTransform;

	for (int x = 0; x < chunkDimensions; x++) {
		for (int y = 0; y < chunkDimensions; y++) {
			for (int z = 0; z < chunkDimensions; z++) {
				renderpass.draw(blocksInChunk[x][y][z].getMesh(), chunkTransform, Wolf3D::getInstance()->blockMaterial);
				chunkTransformZ *= glm::translate(glm::vec3(0.0f, 0.0f, 1.0f));
			}
			chunkTransformZ = chunkTransform;
			chunkTransformY *= glm::translate(glm::vec3(0.0f, 1.0f, 0.0f));
		}
		chunkTransformY = chunkTransform;
		chunkTransformX *= glm::translate(glm::vec3(1.0f, 0.0f, 0.0f));
	}
}