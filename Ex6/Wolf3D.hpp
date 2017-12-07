#pragma once

#include "sre/SDLRenderer.hpp"
#include "sre/Material.hpp"
#include "FirstPersonController.hpp"
#include "Physics.hpp"
#include "sre/SpriteAtlas.hpp"
#include "ParticleSystem.hpp"
#include "Chunk.hpp"
#include "Block.hpp"

class Wolf3D {
public:
    Wolf3D();
	static Wolf3D* getInstance();
	Block* locationToBlock(int x, int y, int z);

	Physics physics;

	std::shared_ptr<sre::Material> blockMaterial; // #WIP blockTextures
	//glm::vec4 textureCoordinates(int blockID); //I didn't know if we still used this, so I just commented it out

	std::shared_ptr<sre::Mesh> getMesh(BlockType type);

private:
    void init();
    void update(float deltaTime);
    void render();
	void renderFloor(sre::RenderPass & renderPass);
	void renderChunk(sre::RenderPass & renderPass);
	void drawGUI();
	void handleDebugKeys(SDL_Event& e);	
	void updateApperance();
	void updateEmit();
//	void loadBlocks(std::string fromFile);

	glm::vec4 textureCoordinates(int blockID);
	std::shared_ptr<sre::Mesh> initializeMesh(BlockType type);

	static bool instanceFlag;
	static Wolf3D* instance;

    sre::SDLRenderer renderer;
    sre::Camera camera;
	FirstPersonController* fpsController;
	sre::WorldLights worldLights;

    std::shared_ptr<sre::Mesh> walls;

	bool physicsDebugDraw = false;
	bool mouseLock = false;
	bool debugProfiler = false;
	
	btRigidBody* fallRigidBody;
	glm::mat4 sphereTransform;
	std::shared_ptr<sre::Mesh> sphere;
	std::shared_ptr<sre::Material> sphereMaterial;
	btTransform sphereTrans;

	std::shared_ptr<sre::Texture> particleTexture;
	std::shared_ptr<ParticleSystem> particleSystem;
	std::shared_ptr<sre::Material> particleMaterial;

	glm::mat4 floorTransform;
	glm::vec4 floorColor;
	std::shared_ptr<sre::Mesh> floor;
	std::shared_ptr<sre::Material> floorMat;

	// Array for all chunks
	const int chunkArraySize = 3;
	std::shared_ptr<Chunk>** chunkArray;

	// List of all block meshes, these are used to be hold in hand by the player
	std::shared_ptr<sre::Mesh>* blockMeshes;

	glm::vec4 colorFrom = { 1,1,1,1 };
	glm::vec4 colorTo = { 1,1,1,0 };
	float sizeFrom = 50;
	float sizeTo = 50;

	glm::vec3 emitPosition = { 0,3,0 };
	float emitVelocity = 1;
	float emitRotation = 10;
	float emitAngularVelocity = 10;
};
