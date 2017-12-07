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

	std::shared_ptr<Texture> particleTexture;
	std::shared_ptr<ParticleSystem> particleSystem;
	std::shared_ptr<sre::Material> particleMaterial;

	glm::mat4 floorTransform;
	glm::vec4 floorColor;
	std::shared_ptr<sre::Mesh> floor;
	std::shared_ptr<sre::Material> floorMat;

	// Array for all chunks
	const int chunkArraySize = 3;
	std::shared_ptr<Chunk>** chunkArray;

	//Different meshes for different blocks
	std::shared_ptr<sre::Mesh> stoneMesh;
	std::shared_ptr<sre::Mesh> brickMesh;
	std::shared_ptr<sre::Mesh> grassMesh;
	std::shared_ptr<sre::Mesh> woolBlueMesh;
	std::shared_ptr<sre::Mesh> sandMesh;
	std::shared_ptr<sre::Mesh> dirtMesh;
	std::shared_ptr<sre::Mesh> gravelMesh;
	std::shared_ptr<sre::Mesh> rockMesh;
	std::shared_ptr<sre::Mesh> woodMesh;
	std::shared_ptr<sre::Mesh> planksMesh;
};
