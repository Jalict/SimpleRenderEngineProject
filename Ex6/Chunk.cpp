#include "Chunk.hpp"
#include "Wolf3D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>




Chunk::Chunk() { // Default constructor. This doesn't do anything.
}


Chunk::Chunk(glm::vec3 position){
	//Set the position of the chunk
	this->position = position;
	chunkTransform = glm::translate(position);
	
	// Create the block array for this chunk.
	blocksInChunk = new Block**[chunkDimensions];
	for (int x = 0; x < chunkDimensions; x++){
		blocksInChunk[x] = new Block*[chunkDimensions];

		for (int y = 0; y < chunkDimensions; y++) {
			blocksInChunk[x][y] = new Block[chunkDimensions];

			// Set blocktype based on height.
			for (int z = 0; z < chunkDimensions; z++) {

				// Bedrock on y == 0
				if (position.y + y == 0) {
					blocksInChunk[x][y][z] = Block(BlockType::Bedrock, glm::vec3(position.x + x, position.y + y, position.z + z));

				//Random ores
				} else if (position.y + y <= 2) {
					int p = rand() % 5;

					if(p == 0)
						blocksInChunk[x][y][z] = Block(BlockType::IronOre, glm::vec3(position.x + x, position.y + y, position.z + z));
					else if(p == 1)
						blocksInChunk[x][y][z] = Block(BlockType::CoalOre, glm::vec3(position.x + x, position.y + y, position.z + z));
					else
						blocksInChunk[x][y][z] = Block(BlockType::Rock, glm::vec3(position.x + x, position.y + y, position.z + z));
					
				//On top of the chunks we want grass
				} else if (position.y + y == chunkDimensions - 1) {
					blocksInChunk[x][y][z] = Block(BlockType::Grass, glm::vec3(position.x + x, position.y + y, position.z + z));

				//Otherwise we want random gravel and dirt
				} else {
					int p = rand() % 3;

					if (p == 0)
						blocksInChunk[x][y][z] = Block(BlockType::Gravel, glm::vec3(position.x + x, position.y + y, position.z + z));
					else
						blocksInChunk[x][y][z] = Block(BlockType::Dirt, glm::vec3(position.x + x, position.y + y, position.z + z));
				}

				// If this is the top chunk, set it to deactive so it can act as air.
				if (position.y + y >= chunkDimensions) {
					blocksInChunk[x][y][z].setActive(false);
				}

				// Init the collider pointers
				blocksInChunk[x][y][z].initCollider();
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
}


void Chunk::draw(sre::RenderPass& renderpass) {
	// Generate Mesh if we are told to recalculate one
	if(recalculateMesh)
		createMesh();

	// Draw Mesh Chunk
	renderpass.draw(this->mesh, chunkTransform, Wolf3D::getInstance()->blockMaterial);
}


void Chunk::createMesh() {
	std::cout << " recalculate chunk" << std::endl;

	// Calculate vertex positions, UV coordinates and normals.
	calculateMesh();

	// Create the chunk  mesh.
	mesh = sre::Mesh::create()
				.withPositions(vertexPositions)
				.withUVs(texCoords)
				.withName("Chunk_" + std::to_string(position.x) + '_' + std::to_string(position.y) + '_' + std::to_string(position.z))
				.withNormals(normals)
				.build();

	// Clean up after we used them.
	vertexPositions.clear();
	texCoords.clear();
	normals.clear();

	// Lower the flag for recalculation, since we just did that.
	recalculateMesh = false;
}


void Chunk::calculateMesh() {
	// Flags for what sides should be drawn, by default faces should be drawn unless told otherwise.
	bool left = true;  
	bool right = true; 
	bool bottom = true; 
	bool top = true; 
	bool front = true;
	bool back = true;

	// Loop over all blocks in this chunk
	for (int x = 0; x < chunkDimensions; x++){
		for (int y = 0; y < chunkDimensions; y++){
			for (int z = 0; z < chunkDimensions; z++){

				// If the block is not activated, it not displayed so skip it alltogether.
				if (blocksInChunk[x][y][z].isActive() == false)
					continue;

				if (x > 0) {
					left = !blocksInChunk[x - 1][y][z].isActive();
				} else {
					auto b = Wolf3D::getInstance()->locationToBlock(position.x + x - 1, position.y + y, position.z + z, true);
					if (b != nullptr){
						left = !b->isActive();
					}
				}

				if (x < chunkDimensions - 1){
					right = !blocksInChunk[x + 1][y][z].isActive();
				} else {
					auto b = Wolf3D::getInstance()->locationToBlock(position.x + x + 1, position.y + y, position.z + z, true);
					if (b != nullptr){
						right = !b->isActive();
					}
				}

				if (y > 0) {
					bottom = !blocksInChunk[x][y - 1][z].isActive();
				} else {
					auto b = Wolf3D::getInstance()->locationToBlock(position.x + x, position.y + y - 1, position.z + z, true);
					if (b != nullptr){
						bottom = !b->isActive();
					}
				}
					
				if (y < chunkDimensions - 1) {
					top = !blocksInChunk[x][y + 1][z].isActive();
				} else {
					auto b = Wolf3D::getInstance()->locationToBlock(position.x + x, position.y + y + 1, position.z + z, true);
					if (b != nullptr){
						top = !b->isActive();
					}
				}
				
				if (z > 0) {
					front = !blocksInChunk[x][y][z - 1].isActive();
				} else {
					auto b = Wolf3D::getInstance()->locationToBlock(position.x + x, position.y + y, position.z + z - 1, true);
					if (b != nullptr) {
						front = !b->isActive();
					}
				}
					
				
				if (z < chunkDimensions - 1) {
					back = !blocksInChunk[x][y][z + 1].isActive();
				} else {
					auto b = Wolf3D::getInstance()->locationToBlock(position.x + x, position.y + y, position.z + z + 1, true);
					if(b != nullptr){
						back = !b->isActive();
					}
				} 
					
				addToMesh(x, y, z, blocksInChunk[x][y][z].getType(), left, right, bottom, top, front, back);

				// Reset the flags for the next loop
				left = true;
				right = true;
				bottom = true;
				top = true;
				front = true;
				back = true;
			}
		}
	}
}


void Chunk::addToMesh(float x, float y, float z, BlockType type, bool left, bool right, bool bottom, bool top, bool front, bool back) {
	// All corners of a cube
	glm::vec3 p1 = glm::vec3(x - 0.5, y - 0.5, z + 0.5);
	glm::vec3 p2 = glm::vec3(x + 0.5, y - 0.5, z + 0.5);
	glm::vec3 p3 = glm::vec3(x + 0.5, y + 0.5, z + 0.5);
	glm::vec3 p4 = glm::vec3(x - 0.5, y + 0.5, z + 0.5);
	glm::vec3 p5 = glm::vec3(x + 0.5, y - 0.5, z - 0.5);
	glm::vec3 p6 = glm::vec3(x - 0.5, y - 0.5, z - 0.5);
	glm::vec3 p7 = glm::vec3(x - 0.5, y + 0.5, z - 0.5);
	glm::vec3 p8 = glm::vec3(x + 0.5, y + 0.5, z - 0.5);

	// #TODO make all list consistent in one place???
	glm::vec4 uvs;


	if (left) {
		vertexPositions.insert(vertexPositions.end(), {
			p6,p1,p4,
			p6,p4,p7
		});

		uvs = textureCoordinates(Block::getTextureIndex(type, BlockSides::Left));
		texCoords.insert(texCoords.end(), {
			glm::vec4(uvs.x,uvs.w,0,0), glm::vec4(uvs.z,uvs.w,0,0), glm::vec4(uvs.z,uvs.y,0,0),
			glm::vec4(uvs.x,uvs.w,0,0), glm::vec4(uvs.z,uvs.y,0,0), glm::vec4(uvs.x,uvs.y,0,0),
		});

		normals.insert(normals.end(), {
			glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0),
			glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0)
		});
	}


	if (right) {
		vertexPositions.insert(vertexPositions.end(), {
			p2,p5,p8,
			p2,p8,p3
		});

		uvs = textureCoordinates(Block::getTextureIndex(type, BlockSides::Right));
		texCoords.insert(texCoords.end(), {
			glm::vec4(uvs.x,uvs.w,0,0), glm::vec4(uvs.z,uvs.w,0,0), glm::vec4(uvs.z,uvs.y,0,0),
			glm::vec4(uvs.x,uvs.w,0,0), glm::vec4(uvs.z,uvs.y,0,0), glm::vec4(uvs.x,uvs.y,0,0),
		});

		normals.insert(normals.end(), {
			glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0),
			glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)
		});
	}

	
	if (bottom) {
		vertexPositions.insert(vertexPositions.end(), {
			p6,p5,p2,
			p6,p2,p1
		});

		uvs = textureCoordinates(Block::getTextureIndex(type, BlockSides::Bottom));
		texCoords.insert(texCoords.end(), {
			glm::vec4(uvs.x,uvs.y,0,0), glm::vec4(uvs.z,uvs.y,0,0), glm::vec4(uvs.z,uvs.w,0,0),
			glm::vec4(uvs.x,uvs.y,0,0), glm::vec4(uvs.z,uvs.w,0,0), glm::vec4(uvs.x,uvs.w,0,0),
		});

		normals.insert(normals.end(), {
			glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0),
			glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0)
		});
	}


	if (top) {
		vertexPositions.insert(vertexPositions.end(), {
			p4,p3,p8,
			p4,p8,p7
		});

		uvs = textureCoordinates(Block::getTextureIndex(type, BlockSides::Top));
		texCoords.insert(texCoords.end(), {
			glm::vec4(uvs.x,uvs.y,0,0), glm::vec4(uvs.z,uvs.y,0,0), glm::vec4(uvs.z,uvs.w,0,0),
			glm::vec4(uvs.x,uvs.y,0,0), glm::vec4(uvs.z,uvs.w,0,0), glm::vec4(uvs.x,uvs.w,0,0),
		});

		normals.insert(normals.end(), {
			glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
			glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)
		});
	}


	// TODO RENAMED BACK TO FRONT
	if (front) {
		vertexPositions.insert(vertexPositions.end(), {
			p5, p6, p7,
			p5, p7, p8
		});

		uvs = textureCoordinates(Block::getTextureIndex(type, BlockSides::Back));
		texCoords.insert(texCoords.end(), {
			glm::vec4(uvs.x,uvs.w,0,0), glm::vec4(uvs.z,uvs.w,0,0), glm::vec4(uvs.z,uvs.y,0,0),
			glm::vec4(uvs.x,uvs.w,0,0), glm::vec4(uvs.z,uvs.y,0,0), glm::vec4(uvs.x,uvs.y,0,0),
		});

		normals.insert(normals.end(), {
			glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), 
			glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), glm::vec3(0, 0, -1)
		});
	}

	
	if (back) {
		vertexPositions.insert(vertexPositions.end(), {
			p1,p2,p3,
			p1,p3,p4
		});

		glm::vec4 coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Front));
		texCoords.insert(texCoords.end(), {
			glm::vec4(coords.x,coords.w,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.z,coords.y,0,0),
			glm::vec4(coords.x,coords.w,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.x,coords.y,0,0)
		});

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


// TODO recalculate on the spot instead of during render thread?
void Chunk::flagRecalculateMesh() {
	recalculateMesh = true;
}


bool Chunk::isCollidersActive() {
	return collidersActive;
}

void Chunk::addCollidersToWorld() {
	// If colliders are already active, we do not have to do anything
	if(collidersActive)
		return;

	for (int x = 0; x < chunkDimensions; x++)	{
		for (int y = 0; y < chunkDimensions; y++) {
			for (int z = 0; z < chunkDimensions; z++) {
				blocksInChunk[x][y][z].addColliderToWorld();
			}
		}
	}

	collidersActive = true;
}


void Chunk::removeCollidersFromWorld() {
	// If colliders are already active, we do not have to do anything
	if(!collidersActive)
		return;

	// If colliders are already not active, we do not have to do anything
	for (int x = 0; x < chunkDimensions; x++) {
		for (int y = 0; y < chunkDimensions; y++) {
			for (int z = 0; z < chunkDimensions; z++) {
				blocksInChunk[x][y][z].removeColliderFromWorld();
			}
		}
	}

	collidersActive = false;
}


glm::vec3 Chunk::getPosition() {
	return position;
}


Block* Chunk::getBlock(int x, int y, int z) {	
	// If the requested block is without bounds return a nullpointer
	if (x < 0 || x > chunkDimensions - 1 || y < 0 || y > chunkDimensions - 1 || z < 0 || z > chunkDimensions - 1) {
		return nullptr;
	}

	// Else return the block
	return &blocksInChunk[x][y][z];
}