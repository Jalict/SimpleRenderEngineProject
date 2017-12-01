#include "Chunk.hpp"

Chunk::Chunk(glm::mat4 chunkTransform){

	testBlock = new Block();

	/*
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
	*/
}

Chunk::~Chunk(){
	/*
	// Delete the blocks
	for (int i = 0; i < chunkDimensions; ++i){
		for (int j = 0; j < chunkDimensions; ++j){
			delete[] blocksInChunk[i][j];
		}
		delete[] blocksInChunk[i];
	}
	delete[] blocksInChunk;
	*/
}

void Chunk::update(float dt) {
	//Dunno what to do in here yet
}

void Chunk::draw(sre::RenderPass& renderpass) {
	//Loop over all cubes, get their information, call render
	//Offset blocks by a transformation every time

	for (int i = 0; i < chunkDimensions; i++) {
		for (int j = 0; j < chunkDimensions; j++) {
			for (int k = 0; k < chunkDimensions; k++) {

			}
		}
	}




}