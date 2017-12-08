#pragma once

#include "sre/SDLRenderer.hpp"
#include "sre/Material.hpp"
#include "FirstPersonController.hpp"
#include "Physics.hpp"
#include "Chunk.hpp"
#include "Block.hpp"

class Wolf3D {
public:
    Wolf3D();

	static Wolf3D* getInstance();

	Block* locationToBlock(int x, int y, int z, bool ghostInspect);

	// Material used for everything
	std::shared_ptr<sre::Material> blockMaterial; 
	// Physics of this world
	Physics physics; 

	// Returns the material of on complete block
	std::shared_ptr<sre::Mesh> getBlockMesh(BlockType type);
	std::shared_ptr<Chunk> getChunk(int x, int y, int z);
private:
    void init();
    void update(float deltaTime);
    void render();
	void renderFloor(sre::RenderPass & renderPass);
	void renderChunk(sre::RenderPass & renderPass);
	void drawGUI();
	void handleDebugKeys(SDL_Event& e);	

	glm::vec4 textureCoordinates(int blockID);
	std::shared_ptr<sre::Mesh> createBlockMesh(BlockType type);


	static bool instanceFlag;
	static Wolf3D* instance;

    sre::SDLRenderer renderer;
    sre::Camera camera;
	sre::WorldLights worldLights;
	FirstPersonController* fpsController;

	bool physicsDebugDraw = false;
	bool mouseLock = false;
	bool debugProfiler = false;
	
	// The red falling ball
	btRigidBody* fallRigidBody;
	glm::mat4 sphereTransform;
	std::shared_ptr<sre::Mesh> sphere;
	std::shared_ptr<sre::Material> sphereMaterial;
	btTransform sphereTrans;

	// Floor for the whole world
	glm::mat4 floorTransform;
	glm::vec4 floorColor;
	std::shared_ptr<sre::Mesh> floor;
	std::shared_ptr<sre::Material> floorMat;

	// Array for all chunks
	//const int chunkArraySize = 2;
	//std::shared_ptr<Chunk>** chunkArray;

	const int chunkArrayX = 1;
	const int chunkArrayY = 2;
	const int chunkArrayZ = 3;
	std::shared_ptr<Chunk>*** chunkArray;

	// List of all block meshes, these are used to be hold in hand by the player
	std::shared_ptr<sre::Mesh>* blockMeshes;
};
