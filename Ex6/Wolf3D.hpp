/*
* Initially Created by Morten Nobel-Jørgensen on 29/09/2017.
*
* FirstPersonController - included in project from: 30-11-2017
* Wrapper for all the bullet physics.
*/
#pragma once

#include "sre/SDLRenderer.hpp"
#include "sre/Material.hpp"
#include "FirstPersonController.hpp"
#include "ParticleSystem.hpp"
#include "Physics.hpp"
#include "Chunk.hpp"
#include "Block.hpp"

class Wolf3D {
public:
    Wolf3D();
	~Wolf3D();

	static Wolf3D* getInstance(); 

	// Pass in a world block location and it flags the chunk for recalculation.
	// Furthermore, it flags neighbouring chunks to recalculate if the said block is on a chunk edge.
	void flagNeighboursForRecalculateIfNecessary(int x, int y, int z);	

	// Pass in a world position and the function returns a pointer to the block on that location.
	// When ghostInspect is set to true no mesh recalculation flag will be raised. 
	// Set it to false and chunks and possible neighbours will be recalculated when necessary.
	Block* locationToBlock(int x, int y, int z, bool ghostInspect);

	// Particle systemm
	void placeParticleSystem(glm::vec3 pos);
	void updateApperance();
	void updateEmit();

	std::shared_ptr<sre::Material> getBlockMaterial() { return blockMaterial; }					// Returns the material shared between all blocks
	std::shared_ptr<sre::Mesh> getBlockMesh(BlockType type) { return blockMeshes[(int)type]; }	// Returns a cube mesh for a block type
	std::shared_ptr<Chunk> getChunk(int x, int y, int z);										// Returns a chunk at chunk coordinates x, y, z

	Physics* getPhysics() { return &physics; }	// Returns the physics wrapper for the game
private:
    void init();
    void update(float deltaTime);
    void render();
	void onKey(SDL_Event& e);

	void drawChunks(sre::RenderPass & renderPass);	// Loops through all chunks and tells them to draw
	void drawGUI();									// Draws the GUI

	void loadColliders(int xPos, int yPos, int zPos);	// Add colliders to the world of the chunk xPos, yPos, zPos and its neighbours. Unloads all others.

	std::shared_ptr<sre::Mesh> createBlockMesh(BlockType type);	// Creates a block mesh for the blockType. These are used to display blocks in hand
	glm::vec4 textureCoordinates(int textureId);				// Translates a texture index to UV coordinates

	// Singleton pattern
	static bool instanceFlag;
	static Wolf3D* instance;

	// World elements and controllers we need
	FirstPersonController* fpsController;
	sre::WorldLights worldLights;
    sre::SDLRenderer renderer;
    sre::Camera camera;
	Physics physics;

	// Togglles for various debug modes
	bool physicsDebugDraw = false;	// Whether we should allow the physics debug drawer to draw
	bool debugProfiler = false;		// Whether we should show the profiler
	bool mouseLock = true;			// Whether the mouse is locked in the window

	// Array for all chunks, and how many chunks we have in each axis.
	const int chunkArrayX = 34;
	const int chunkArrayY = 2; 
	const int chunkArrayZ = 34;
	std::shared_ptr<Chunk>*** chunkArray;

	// List of all block meshes, these are used to be hold in hand by the player.
	std::shared_ptr<sre::Mesh>* blockMeshes;

	// Material used to draw blocks, used by both block meshes and chunk meshes
	std::shared_ptr<sre::Material> blockMaterial;

	// Particles
	std::shared_ptr<sre::Texture> particleTexture;
	std::shared_ptr<ParticleSystem> particleSystem;

	// Particle setting
	float sizeFrom = 50;
	float sizeTo = 0;
	float elapsedParticleTime = 0;

	glm::vec3 emitPosition = { 0,0,0 };
	float emitVelocity = 1;
	float emitRotation = 10;
	float emitAngularVelocity = 10;
};
