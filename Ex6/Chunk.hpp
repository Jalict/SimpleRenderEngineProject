/*
* Chunk - Created: 01-12-2017
* Holds a three dimensional array of blocks that are contained in this chunk.
*/
#pragma once

#include <glm\gtx\rotate_vector.hpp>
#include "sre/SDLRenderer.hpp"
#include "sre/Material.hpp"
#include "ParticleSystem.hpp"
#include "Block.hpp"



class Chunk {
public:
	Chunk();
	Chunk(glm::vec3 position);
	~Chunk();

	void update(float dt);
	void draw(sre::RenderPass& renderpass);

	Block* getBlock(int x, int y, int z);	// Returns a block with the passed in coordinates. These are local chunk coordinates!
	
	void flagRecalculateMesh();			// Raises the flag to recalculate the mesh.
	void addCollidersToWorld();			// Tells all the blocks in this chunk to add their rigidbodies to the world.
	void removeCollidersFromWorld();	// Tells all the blocks to remove their rigidbodies from the world.

	bool isCollidersActive() { return collidersActive; }
	glm::vec3 getPosition() { return position; }		

	const static int chunkSize = 8;		// Size of the chunk in all dimensions, e.g. when 8 the chunk is 8x8x8.
private:
	void generateMesh();
	void calculateMesh(std::vector<glm::vec3>& vertexPositions, std::vector<glm::vec4>& uvCoords, std::vector<glm::vec3>& normals);
	void addToMesh(	glm::vec3 position, BlockType type, bool left, bool right, bool bottom, bool top, bool front, bool back, 
					std::vector<glm::vec3>& vertexPositions, std::vector<glm::vec4>& uvCoords, std::vector<glm::vec3>& normals);
	glm::vec4 textureCoordinates(int blockID);


	glm::vec3 position;			// The position of this chunk
	glm::mat4 chunkTransform;	// Transform matrix of this chunk

	// Flag to see if we need to recalculate our mesh
	bool recalculateMesh = true; 
	std::shared_ptr<sre::Mesh> mesh;

	// Whether colliders are active on this chunk
	bool collidersActive = false;

	// The actual blocks in this chunk
	Block*** blocksInChunk;
};