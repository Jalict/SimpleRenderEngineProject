#include "Chunk.hpp"
#include "Wolf3D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

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
	if(recalculateMesh)
		createMesh();

	renderpass.draw(this->mesh, chunkTransform, Wolf3D::getInstance()->blockMaterial);
}


void Chunk::createMesh() {
	std::cout << "recal" << std::endl; 

	calculateMesh();

	// Create the chunk
	std::string chunkID = "Chunk_" + std::to_string(position.x) + '_' + std::to_string(position.y) + '_' + std::to_string(position.z);
	mesh = sre::Mesh::create().withPositions(vertexPositions).withUVs(texCoords).withName(chunkID).withNormals(normals).build();

	// Clean up after we used them
	vertexPositions.clear();
	texCoords.clear();
	normals.clear();

	// Lower the flag for recalculation
	recalculateMesh = false;
}


void Chunk::calculateMesh() {
	for (int x = 0; x < chunkDimensions; x++){
		for (int y = 0; y < chunkDimensions; y++){
			for (int z = 0; z < chunkDimensions; z++){
				if (blocksInChunk[x][y][z].getActive() == false){
					continue;
				}

				bool XNegative = true;
				if (x > 0)
					XNegative = !blocksInChunk[x - 1][y][z].getActive();

				bool XPositive = true;
				if (x < chunkDimensions - 1)
					XPositive = !blocksInChunk[x + 1][y][z].getActive();

				bool YNegative = true;
				if (y > 0)
					YNegative = !blocksInChunk[x][y - 1][z].getActive();

				bool YPositive = true;
				if (y < chunkDimensions - 1)
					YPositive = !blocksInChunk[x][y + 1][z].getActive();

				bool ZNegative = true;
				if (z > 0)
					ZNegative = !blocksInChunk[x][y][z - 1].getActive();

				bool ZPositive = true;
				if (z < chunkDimensions - 1)
					ZPositive = !blocksInChunk[x][y][z + 1].getActive();

				addToMesh(XNegative, XPositive, YNegative, YPositive, ZNegative, ZPositive, x, y, z, blocksInChunk[x][y][z].getType());
			}
		}
	}
}


void Chunk::addToMesh(bool XNegative, bool XPositive, bool YNegative, bool YPositive, bool ZNegative, bool ZPositive, float x, float y, float z, BlockType type) {
	glm::vec3 p1 = glm::vec3(x - 0.5, y - 0.5, z + 0.5);
	glm::vec3 p2 = glm::vec3(x + 0.5, y - 0.5, z + 0.5);
	glm::vec3 p3 = glm::vec3(x + 0.5, y + 0.5, z + 0.5);
	glm::vec3 p4 = glm::vec3(x - 0.5, y + 0.5, z + 0.5);
	glm::vec3 p5 = glm::vec3(x + 0.5, y - 0.5, z - 0.5);
	glm::vec3 p6 = glm::vec3(x - 0.5, y - 0.5, z - 0.5);
	glm::vec3 p7 = glm::vec3(x - 0.5, y + 0.5, z - 0.5);
	glm::vec3 p8 = glm::vec3(x + 0.5, y + 0.5, z - 0.5);

	glm::vec4 coords;

	//Left
	if (XNegative) {
		vertexPositions.insert(vertexPositions.end(), {
			p6,p1,p4,
			p6,p4,p7
		});

		coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Left));
		texCoords.insert(texCoords.end(), {
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
		});

		//Normal for this face
		normals.insert(normals.end(), {
			glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0),
			glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0)
		});
	}
	//Right
	if (XPositive) {
		//Vertex positions for this face
		vertexPositions.insert(vertexPositions.end(), {
			p2,p5,p8,
			p2,p8,p3
		});

		//Texture coordinates for this face
		coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Right));
		texCoords.insert(texCoords.end(), {
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
		});

		//Normal for this face
		normals.insert(normals.end(), {
			glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0),
			glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)
		});
	}

	//Bottom
	if (YNegative) {
		vertexPositions.insert(vertexPositions.end(), {
			p6,p5,p2,
			p6,p2,p1
		});

		coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Bottom));
		texCoords.insert(texCoords.end(), {
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
		});

		//Normal for this face
		normals.insert(normals.end(), {
			glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0),
			glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0)
		});
	}
	//Top
	if (YPositive) {
		vertexPositions.insert(vertexPositions.end(), {
			p4,p3,p8,
			p4,p8,p7
		});

		coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Top));
		texCoords.insert(texCoords.end(), {
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
		});

		//Normal for this face
		normals.insert(normals.end(), {
			glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
			glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)
		});
	}

	//Back
	if (ZNegative) {
		vertexPositions.insert(vertexPositions.end(), {
			p5, p6, p7,
			p5, p7, p8
		});

		coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Back));
		texCoords.insert(texCoords.end(), {
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
		});

		//Normal for this face
		normals.insert(normals.end(), {
			glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), 
			glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), glm::vec3(0, 0, -1)
		});
	}

	//Front
	if (ZPositive) {
		vertexPositions.insert(vertexPositions.end(), {
			p1,p2,p3,
			p1,p3,p4
		});

		glm::vec4 coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Front));
		texCoords.insert(texCoords.end(), {
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
			glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0)
		});

		//Normal for this face
		normals.insert(normals.end(), {
			glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1),
			glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)
		});
	}
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
	// If the block is not within bounds of this chunk, return null pointer
	if ((x < 0 && x >= chunkDimensions) || (y < 0 && y >= chunkDimensions) || (z < 0 && z >= chunkDimensions)) {
		std::cout << "block doesnt exist" << std::endl;
		return nullptr;
	}

	// If an block was requested, changes probably occurred. Therefore recalculate.
	// Eventhough this is not 100% accurate it saves us from having to loop over each block and check for flags
	// Or store a reference to the parent chunk in each block
	recalculateMesh = true;

	// Else return the block
	return &blocksInChunk[x][y][z];
}