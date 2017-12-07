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
	static int getChunkDimensions() { return chunkDimensions; }

	void update(float dt);
	void draw(sre::RenderPass& renderpass);
	glm::vec3 getPosition();
	Block* getBlock(int x, int y, int z);

	void placeParticleSystem(glm::vec3 pos);
	void updateApperance();
	void updateEmit();
	
private:
	const static int chunkDimensions = 5;
	glm::vec3 position;
	
	glm::mat4 chunkTransform;
	std::vector<glm::vec3> vertexPositions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> texCoords;

	Block*** blocksInChunk;
	glm::vec4 textureCoordinates(int blockID);

	void addToMesh(bool XNegative, bool XPositive, bool YNegative, bool YPositive, bool ZNegative, bool ZPositive, float x, float y, float z, BlockType type);
	std::shared_ptr<sre::Mesh> mesh;
	void assembleVertexPositionsAndTexturePoints();
	void createMesh();

	// Particles
	std::shared_ptr<sre::Texture> particleTexture;
	std::vector<std::shared_ptr<ParticleSystem>> particleSystems;

	// Particle setting
	glm::vec4 colorFrom = { 1,1,1,1 };
	glm::vec4 colorTo = { 1,1,1,0 };
	float sizeFrom = 50;
	float sizeTo = 50;

	glm::vec3 emitPosition = { 0,0,0 };
	float emitVelocity = 1;
	float emitRotation = 10;
	float emitAngularVelocity = 10;
};