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
				blocksInChunk[x][y][z] = Block(BlockType::Sand, glm::vec3(position.x + x,position.y + y, position.z + z));
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

//void Chunk::draw(sre::RenderPass& renderpass) {
//	//Loop over all cubes, get their information, call render
//	//Offset blocks by a transformation every time
//
//	for (int x = 0; x < chunkDimensions; x++) {
//		for (int y = 0; y < chunkDimensions; y++) {
//			for (int z = 0; z < chunkDimensions; z++) {
//				auto transformMatrix = glm::translate(chunkTransform, glm::vec3(x, y, z));
//
//				//NOTE: Blocks aren't actually ever set to not being active
//				//std::cout << blocksInChunk[x][y][z].getActive() << std::endl;
//
//				//If the block is not active we don't render it
//				if (blocksInChunk[x][y][z].getActive() == false){
//					//std::cout << "Skipped an inactive block" << std::endl;
//					continue;
//				}
//
//				//If the block is at the edge of a chunk we always render it
//				bool onOutsideOfChunk = false;
//				if (!(x > 0) || !(x < chunkDimensions - 1) || !(y > 0) || !(y < chunkDimensions - 1) || !(z > 0) || !(z < chunkDimensions - 1)) {
//					onOutsideOfChunk = true;
//				}
//
//				//If the block is surrounded by active blocks we don't render it
//				bool surrounded = false;
//				if (!onOutsideOfChunk) {
//					if (blocksInChunk[x - 1][y][z].getActive() && 
//						blocksInChunk[x + 1][y][z].getActive() &&
//						blocksInChunk[x][y - 1][z].getActive() && 
//						blocksInChunk[x][y + 1][z].getActive() &&
//						blocksInChunk[x][y][z - 1].getActive() && 
//						blocksInChunk[x][y][z + 1].getActive()) {
//
//						surrounded = true;
//					}
//				}
//
//				if (onOutsideOfChunk || !surrounded) {
//					renderpass.draw(blocksInChunk[x][y][z].getMesh(), transformMatrix, Wolf3D::getInstance()->blockMaterial);
//				}
//			}
//		}
//	}
//}


void Chunk::draw(sre::RenderPass& renderpass) {
	assembleVertexPositionsAndTexturePoints();
	createMesh();
	renderpass.draw(this->mesh, chunkTransform, Wolf3D::getInstance()->blockMaterial);
	vertexPositions.clear();
	texCoords.clear();
}


void Chunk::createMesh() {
	mesh = sre::Mesh::create().withPositions(vertexPositions).withUVs(texCoords).build();
}


void Chunk::assembleVertexPositionsAndTexturePoints() {
	bool Default = true;

	for (int x = 0; x < chunkDimensions; x++){
		for (int y = 0; y < chunkDimensions; y++){
			for (int z = 0; z < chunkDimensions; z++){
				if (blocksInChunk[x][y][z].getActive() == false){
					continue;
				}

				bool XNegative = Default;
				if (x > 0)
					XNegative = blocksInChunk[x - 1][y][z].getActive();

				bool XPositive = Default;
				if (x < chunkDimensions - 1)
					XPositive = blocksInChunk[x + 1][y][z].getActive();

				bool YNegative = Default;
				if (y > 0)
					YNegative = blocksInChunk[x][y - 1][z].getActive();

				bool YPositive = Default;
				if (y < chunkDimensions - 1)
					YPositive = blocksInChunk[x][y + 1][z].getActive();

				bool ZNegative = Default;
				if (z > 0)
					ZNegative = blocksInChunk[x][y][z - 1].getActive();

				bool ZPositive = Default;
				if (z < chunkDimensions - 1)
					ZPositive = blocksInChunk[x][y][z + 1].getActive();

				addToMesh(XNegative, XPositive, YNegative, YPositive, ZNegative, ZPositive, x, y, z, blocksInChunk[x][y][z].getType());
			}
		}
	}
}


void Chunk::addToMesh(bool XNegative, bool XPositive, bool YNegative, bool YPositive, bool ZNegative, bool ZPositive, float x, float y, float z, BlockType type) {

	if (ZNegative) {
		vertexPositions.insert(vertexPositions.end(), {
			glm::vec3(x + -0.5, y + -0.5, z + 0.5), glm::vec3(x + 0.5, y + -0.5, z + 0.5), glm::vec3(x + -0.5, y + 0.5, z + 0.5),
			glm::vec3(x + 0.5, y + 0.5, z + 0.5), glm::vec3(x + -0.5, y + 0.5, z + 0.5), glm::vec3(x + 0.5, y + -0.5, z + 0.5)
		});
	}

	//This is probably placed wrong
	if (YNegative) {
		vertexPositions.insert(vertexPositions.end(), {
			glm::vec3(x + -0.5, y + 0.5, z + -0.5), glm::vec3(x + -0.5, y + 0.5, z + 0.5),  glm::vec3(x + 0.5, y + 0.5, z + -0.5),
			glm::vec3(x + 0.5, y + 0.5, z + 0.5), glm::vec3(x + 0.5, y + 0.5, z + -0.5),  glm::vec3(x + -0.5, y + 0.5, z + 0.5)
		});
	}

	if (XNegative) {
		vertexPositions.insert(vertexPositions.end(), {
			glm::vec3(x + 0.5, y + -0.5, z + 0.5), glm::vec3(x + 0.5, y + 0.5, z + -0.5), glm::vec3(x + 0.5, y + 0.5, z + 0.5),
			glm::vec3(x + 0.5, y + -0.5, z + 0.5), glm::vec3(x + 0.5, y + -0.5, z + -0.5), glm::vec3(x + 0.5, y + 0.5, z + -0.5)
		});
	}

	if (ZPositive) {
		vertexPositions.insert(vertexPositions.end(), {
			glm::vec3(x + 0.5, y + -0.5, z + -0.5), glm::vec3(x + -0.5, y + -0.5, z + -0.5),  glm::vec3(x + 0.5, y + 0.5, z + -0.5),
			glm::vec3(x + -0.5, y + 0.5, z + -0.5), glm::vec3(x + 0.5, y + 0.5, z + -0.5), glm::vec3(x + -0.5, y + -0.5, z + -0.5)
		});
	}

	//This is probably placed wrong
	if (YPositive) {
		vertexPositions.insert(vertexPositions.end(), {
			glm::vec3(x + -0.5, y + -0.5, z + 0.5), glm::vec3(x + -0.5, y + -0.5, z + -0.5),  glm::vec3(x + 0.5, y + -0.5, z + 0.5),
			glm::vec3(x + 0.5, y + -0.5, z + -0.5), glm::vec3(x + 0.5, y + -0.5, z + 0.5),  glm::vec3(x + -0.5, y + -0.5, z + -0.5)
		});
	}

	if (XPositive) {
		vertexPositions.insert(vertexPositions.end(), {
			glm::vec3(x + -0.5, y + 0.5, z + -0.5), glm::vec3(x + -0.5, y + -0.5, z + -0.5),  glm::vec3(x + -0.5, y + 0.5, z + 0.5),
			glm::vec3(x + -0.5, y + 0.5, z + 0.5), glm::vec3(x + -0.5, y + -0.5, z + -0.5),  glm::vec3(x + -0.5, y + -0.5, z + 0.5)
		});
	}

	const glm::vec4 coords = textureCoordinates((int)type);
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


glm::vec4 Chunk::textureCoordinates(int blockID) {
	glm::vec2 textureSize(1024, 2048);
	glm::vec2 tileSize(128, 128);

	float tileWidth = tileSize.x / textureSize.x;
	float tileHeight = tileSize.y / textureSize.y;

	glm::vec2 min = glm::vec2(0, 1.0f);							// Start at top left
	glm::vec2 max = min + glm::vec2(tileWidth, -tileHeight);	// Move max to bottom right corner of this block

	int tilesX = textureSize.x / tileSize.x;

	float xOffset = (blockID % tilesX) * tileWidth;
	float yOffset = ((blockID - (blockID % tilesX)) / tilesX) * tileHeight;

	min.x += xOffset;
	max.x += xOffset;

	min.y -= yOffset;
	max.y -= yOffset;

	return glm::vec4(min.x, min.y, max.x, max.y);
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