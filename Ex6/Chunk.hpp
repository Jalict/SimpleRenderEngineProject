#pragma once
#include "Block.hpp"
#include "sre/SDLRenderer.hpp"
#include <glm\gtx\rotate_vector.hpp>
#include "sre/SDLRenderer.hpp"
#include "sre/Material.hpp"
#include "ParticleSystem.hpp"

/*
Created: 01-12-2017
Holds a list of blocks that lie in 5x5x5 cube. It can draw these blocks by taking information from the 
blocks and (some other class that contains the mesh/material of the blocks). It can also update which
blocks are active and which are not.
*/

class Chunk {
public:
	Chunk();
	Chunk(glm::vec3 position);
	~Chunk();

	void update(float dt);
	void draw(sre::RenderPass& renderpass);
	
	// When this function is called the mesh for the chunk will be recalculated.
	void flagRecalculateMesh();
	void addCollidersToWorld();
	void removeCollidersFromWorld();
	bool isCollidersActive();

	void placeParticleSystem(glm::vec3 pos);
	void updateApperance();
	void updateEmit();

	glm::vec3 getPosition();
	Block* getBlock(int x, int y, int z);
	static int getChunkDimensions() { return chunkDimensions; } // #TODO just public constant?
private:
	void calculateMesh();
	void createMesh();
	void addToMesh(float x, float y, float z, BlockType type, bool left, bool right, bool bottom, bool top, bool front, bool back);
	glm::vec4 textureCoordinates(int blockID);


	const static int chunkDimensions = 8;

	glm::vec3 position;
	glm::mat4 chunkTransform;

	// List of positions for the Mesh 
	// # TODO move?
	std::vector<glm::vec3> vertexPositions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> texCoords;

	// Flag to see if we need to recalculate our mesh
	bool recalculateMesh = true; 
	std::shared_ptr<sre::Mesh> mesh;

	// Whether colliders are active on this chunk
	bool collidersActive = false;

	// The actual blocks in this chunk
	Block*** blocksInChunk;
	
	// Particles
	std::shared_ptr<sre::Texture> particleTexture;
	std::shared_ptr<ParticleSystem> particleSystem;

	// Particle setting
	float sizeFrom = 50;
	float sizeTo = 0;

	glm::vec3 emitPosition = { 0,0,0 };
	float emitVelocity = 1;
	float emitRotation = 10;
	float emitAngularVelocity = 10;
};