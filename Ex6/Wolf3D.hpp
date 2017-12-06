#pragma once

#include "sre/SDLRenderer.hpp"
#include "sre/Material.hpp"
#include "FirstPersonController.hpp"
#include "Physics.hpp"
#include "sre/SpriteAtlas.hpp"
#include "Chunk.hpp"

class Wolf3D {
public:
    Wolf3D();
	static Wolf3D* getInstance();
	Block* locationToBlock(glm::vec3 location);

	Physics physics;

	std::shared_ptr<sre::Material> blockMaterial; // #WIP blockTextures
	glm::vec4 textureCoordinates(int blockID);

private:
    void init();
    void update(float deltaTime);
    void render();
	void renderFloor(sre::RenderPass & renderPass);
	void renderChunk(sre::RenderPass & renderPass);
	void drawGUI();
	void handleDebugKeys(SDL_Event& e);	

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

	glm::mat4 floorTransform;
	glm::vec4 floorColor;
	std::shared_ptr<sre::Mesh> floor;
	std::shared_ptr<sre::Material> floorMat;

	std::vector<std::shared_ptr<Chunk>> chunkList;
//	std::shared_ptr<Chunk> chunk;
};
