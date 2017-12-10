#include "Chunk.hpp"
#include "Game.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>


Chunk::Chunk() { 
}


Chunk::Chunk(glm::vec3 position){
	//Set the position of the chunk
	this->position = position;
	chunkTransform = glm::translate(position);
	
	// Create the block array for this chunk.
	blocksInChunk = new Block**[chunkSize];
	for (int x = 0; x < chunkSize; x++){
		blocksInChunk[x] = new Block*[chunkSize];
		for (int y = 0; y < chunkSize; y++) {
			blocksInChunk[x][y] = new Block[chunkSize];		
			for (int z = 0; z < chunkSize; z++) {
				// Set blocktype based on height.
				// Bedrock on world Y = 0.
				if (position.y + y == 0) {
					blocksInChunk[x][y][z] = Block(BlockType::Bedrock, glm::vec3(position.x + x, position.y + y, position.z + z));
				} 
				// Rock and ores on world Y for 1 to 2.
				else if (position.y + y <= 2) {
					int p = rand() % 5;

					if(p == 0)
						blocksInChunk[x][y][z] = Block(BlockType::IronOre, glm::vec3(position.x + x, position.y + y, position.z + z));
					else if(p == 1)
						blocksInChunk[x][y][z] = Block(BlockType::CoalOre, glm::vec3(position.x + x, position.y + y, position.z + z));
					else
						blocksInChunk[x][y][z] = Block(BlockType::Rock, glm::vec3(position.x + x, position.y + y, position.z + z));					
				} 
				// At world Y of chunkSize - 1 we place grass.
				else if (position.y + y == chunkSize - 1) {
					blocksInChunk[x][y][z] = Block(BlockType::Grass, glm::vec3(position.x + x, position.y + y, position.z + z));
				} 
				// In all other cases we place dirt and gravel.
				else {
					int p = rand() % 3;

					if (p == 0)
						blocksInChunk[x][y][z] = Block(BlockType::Gravel, glm::vec3(position.x + x, position.y + y, position.z + z));
					else
						blocksInChunk[x][y][z] = Block(BlockType::Dirt, glm::vec3(position.x + x, position.y + y, position.z + z));
				}

				// If this is not the bottom chunk, deactivate the blocks so it acts as air in which we can place blocks.
				if (position.y + y >= chunkSize) {
					blocksInChunk[x][y][z].setActive(false);
				}

				// Initialize the collider pointers for the block.
				blocksInChunk[x][y][z].initCollider();
			}
		}
	}
}


Chunk::~Chunk(){
	for (int i = 0; i < chunkSize; ++i){
		for (int j = 0; j < chunkSize; ++j){
			delete[] blocksInChunk[i][j];
		}
		delete[] blocksInChunk[i];
	}
	delete[] blocksInChunk;
}


void Chunk::update(float dt) {
	// If the flag is raised to recalculate the mesh, generate it.
	if (recalculateMesh)
		generateMesh();
}


void Chunk::draw(sre::RenderPass& renderpass) {
	// If for some reason we do not have a mesh, generate one immediately!
	if (mesh == nullptr) {
		generateMesh();
		std::cout << "Chunk (" << position.x / chunkSize << ", " << position.y / chunkSize << ", " << position.z / chunkSize << ") had no mesh, calculating one now!" << std::endl;
	}
		
	// Draw mesh of this chunk.
	renderpass.draw(mesh, chunkTransform, Game::getInstance()->getBlockMaterial());
}


// # TODO better function name
void Chunk::generateMesh() {
	std::cout << "Recalculating mesh for chunk (" << position.x / chunkSize << ", " << position.y / chunkSize << ", " << position.z / chunkSize << ")." << std::endl;

	std::vector<glm::vec3> vertexPositions;
	std::vector<glm::vec4> uvCoords;
	std::vector<glm::vec3> normals;

	// Calculate vertex positions, UV coordinates and normals.
	calculateMesh(vertexPositions, uvCoords, normals);

	// Create the chunk  mesh.
	mesh = sre::Mesh::create()
				.withPositions(vertexPositions)
				.withUVs(uvCoords)
				.withName("Chunk_" + std::to_string(position.x) + '_' + std::to_string(position.y) + '_' + std::to_string(position.z))
				.withNormals(normals)
				.build();

	// Clean up after we used them.
	vertexPositions.clear();
	uvCoords.clear();
	normals.clear();

	// Lower the flag for recalculation, since we just did that.
	recalculateMesh = false;
}


// # TODO better function name
void Chunk::calculateMesh(std::vector<glm::vec3>& vertexPositions, std::vector<glm::vec4>& uvCoords, std::vector<glm::vec3>& normals) {
	// Flags for what sides should be drawn, by default faces should be drawn unless told otherwise.
	bool left = true;  
	bool right = true; 
	bool bottom = true; 
	bool top = true; 
	bool front = true;
	bool back = true;

	// Loop over all blocks in this chunk, and see what sides should be added to  the mesh
	for (int x = 0; x < chunkSize; x++){
		for (int y = 0; y < chunkSize; y++){
			for (int z = 0; z < chunkSize; z++){

				// If the block is not activated, it not displayed so skip it alltogether.
				if (blocksInChunk[x][y][z].isActive() == false)
					continue;

				// Check if there is a block left of us (either in this chunk or the chunk next to us if were on an edge), ifso we do not need to show our left side.
				if (x > 0) {
					left = !blocksInChunk[x - 1][y][z].isActive();
				} else {
					auto b = Game::getInstance()->locationToBlock(position.x + x - 1, position.y + y, position.z + z, true);
					if (b != nullptr){
						left = !b->isActive();
					}
				}

				// Check if there is a block right of us (either in this chunk or the chunk next to us if were on an edge), ifso we do not need to show our left side.
				if (x < chunkSize - 1){
					right = !blocksInChunk[x + 1][y][z].isActive();
				} else {
					auto b = Game::getInstance()->locationToBlock(position.x + x + 1, position.y + y, position.z + z, true);
					if (b != nullptr){
						right = !b->isActive();
					}
				}

				// Check if there is a block below us (either in this chunk or the chunk next to us if were on an edge), ifso we do not need to show our left side.
				if (y > 0) {
					bottom = !blocksInChunk[x][y - 1][z].isActive();
				} else {
					auto b = Game::getInstance()->locationToBlock(position.x + x, position.y + y - 1, position.z + z, true);
					if (b != nullptr){
						bottom = !b->isActive();
					}
				}
					
				// Check if there is a block above us (either in this chunk or the chunk next to us if were on an edge), ifso we do not need to show our left side.
				if (y < chunkSize - 1) {
					top = !blocksInChunk[x][y + 1][z].isActive();
				} else {
					auto b = Game::getInstance()->locationToBlock(position.x + x, position.y + y + 1, position.z + z, true);
					if (b != nullptr){
						top = !b->isActive();
					}
				}
				
				// Check if there is a block in front of us (either in this chunk or the chunk next to us if were on an edge), ifso we do not need to show our left side.
				if (z > 0) {
					front = !blocksInChunk[x][y][z - 1].isActive();
				} else {
					auto b = Game::getInstance()->locationToBlock(position.x + x, position.y + y, position.z + z - 1, true);
					if (b != nullptr) {
						front = !b->isActive();
					}
				}
					
				// Check if there is a block behind us (either in this chunk or the chunk next to us if were on an edge), ifso we do not need to show our left side.
				if (z < chunkSize - 1) {
					back = !blocksInChunk[x][y][z + 1].isActive();
				} else {
					auto b = Game::getInstance()->locationToBlock(position.x + x, position.y + y, position.z + z + 1, true);
					if(b != nullptr){
						back = !b->isActive();
					}
				} 
					
				// Now that we have determined which sides should be added to the mesh, call the function which addes them.
				addToMesh(glm::vec3(x, y, z), blocksInChunk[x][y][z].getType(), left, right, bottom, top, front, back, vertexPositions, uvCoords, normals);

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


// # TODO find proper name for function
void Chunk::addToMesh(	glm::vec3 position, BlockType type, bool left, bool right, bool bottom, bool top, bool front, bool back, 
						std::vector<glm::vec3>& vertexPositions, std::vector<glm::vec4>& uvCoords, std::vector<glm::vec3>& normals) {

	// Store points for all corners of the cube
	glm::vec3 p1 = glm::vec3(position.x - 0.5, position.y - 0.5, position.z + 0.5);
	glm::vec3 p2 = glm::vec3(position.x + 0.5, position.y - 0.5, position.z + 0.5);
	glm::vec3 p3 = glm::vec3(position.x + 0.5, position.y + 0.5, position.z + 0.5);
	glm::vec3 p4 = glm::vec3(position.x - 0.5, position.y + 0.5, position.z + 0.5);
	glm::vec3 p5 = glm::vec3(position.x + 0.5, position.y - 0.5, position.z - 0.5);
	glm::vec3 p6 = glm::vec3(position.x - 0.5, position.y - 0.5, position.z - 0.5);
	glm::vec3 p7 = glm::vec3(position.x - 0.5, position.y + 0.5, position.z - 0.5);
	glm::vec3 p8 = glm::vec3(position.x + 0.5, position.y + 0.5, position.z - 0.5);

	// Used to store the UV coordinates for this side.
	glm::vec4 uvs;

	// Check al sides and add vertex positions, uv coordinates and normals where necessary.
	if (left) {
		vertexPositions.insert(vertexPositions.end(), {
			p6,p1,p4,
			p6,p4,p7
		});

		uvs = textureCoordinates(Block::getTextureIndex(type, BlockSides::Left));
		uvCoords.insert(uvCoords.end(), {
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
		uvCoords.insert(uvCoords.end(), {
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
		uvCoords.insert(uvCoords.end(), {
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
		uvCoords.insert(uvCoords.end(), {
			glm::vec4(uvs.x,uvs.y,0,0), glm::vec4(uvs.z,uvs.y,0,0), glm::vec4(uvs.z,uvs.w,0,0),
			glm::vec4(uvs.x,uvs.y,0,0), glm::vec4(uvs.z,uvs.w,0,0), glm::vec4(uvs.x,uvs.w,0,0),
		});

		normals.insert(normals.end(), {
			glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
			glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)
		});
	}


	if (front) {
		vertexPositions.insert(vertexPositions.end(), {
			p5, p6, p7,
			p5, p7, p8
		});

		uvs = textureCoordinates(Block::getTextureIndex(type, BlockSides::Back));
		uvCoords.insert(uvCoords.end(), {
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
		uvCoords.insert(uvCoords.end(), {
			glm::vec4(coords.x,coords.w,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.z,coords.y,0,0),
			glm::vec4(coords.x,coords.w,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.x,coords.y,0,0)
		});

		normals.insert(normals.end(), {
			glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1),
			glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)
		});
	}
}


glm::vec4 Chunk::textureCoordinates(int textureID) {
	glm::vec2 textureSize(1024, 2048);
	glm::vec2 tileSize(128, 128);

	float tileWidth = tileSize.x / textureSize.x;
	float tileHeight = tileSize.y / textureSize.y;

	glm::vec2 min = glm::vec2(0, 1.0f);							// Start at top left
	glm::vec2 max = min + glm::vec2(tileWidth, -tileHeight);	// Move max to bottom right corner of this block

	int tilesX = textureSize.x / tileSize.x;

	float xOffset = (textureID % tilesX) * tileWidth;
	float yOffset = ((textureID - (textureID % tilesX)) / tilesX) * tileHeight;

	min.x += xOffset;
	max.x += xOffset;

	min.y -= yOffset;
	max.y -= yOffset;

	return glm::vec4(min.x, min.y, max.x, max.y);
}


void Chunk::flagRecalculateMesh() {
	recalculateMesh = true;
}


void Chunk::addCollidersToWorld() {
	// If colliders are already active, we do not have to do anything
	if(collidersActive)
		return;

	for (int x = 0; x < chunkSize; x++)	{
		for (int y = 0; y < chunkSize; y++) {
			for (int z = 0; z < chunkSize; z++) {
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
	for (int x = 0; x < chunkSize; x++) {
		for (int y = 0; y < chunkSize; y++) {
			for (int z = 0; z < chunkSize; z++) {
				blocksInChunk[x][y][z].removeColliderFromWorld();
			}
		}
	}

	collidersActive = false;
}


Block* Chunk::getBlock(int x, int y, int z) {	
	// If the requested block is without bounds return a nullpointer
	if (x < 0 || x >= chunkSize || y < 0 || y >= chunkSize || z < 0 || z >= chunkSize) {
		return nullptr;
	}

	// Else return the block
	return &blocksInChunk[x][y][z];
}