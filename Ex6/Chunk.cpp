#include "Chunk.hpp"

Chunk::Chunk(){
	// Create the blocks
	blocksInChunk = new Block** [chunkDimension];
	for (int i = 0; i < chunkDimension; i++){
		blocksInChunk[i] = new Block* [chunkDimension];

		for (int j = 0; j < chunkDimension; j++){
			blocksInChunk[i][j] = new Block[chunkDimension];
		}
	}
}

Chunk::~Chunk(){
	// Delete the blocks
	for (int i = 0; i < chunkDimension; ++i){
		for (int j = 0; j < chunkDimension; ++j){
			delete[] blocksInChunk[i][j];
		}
		delete[] blocksInChunk[i];
	}
	delete[] blocksInChunk;
}

void Chunk::update(float dt) {

}

void Chunk::draw(sre::RenderPass& renderpass) {

}