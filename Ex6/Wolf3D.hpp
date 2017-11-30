#pragma once

#include "sre/SDLRenderer.hpp"
#include "sre/Material.hpp"
#include "WorldMap.hpp"
#include "FirstPersonController.hpp"
#include "Physics.hpp"
#include "sre/SpriteAtlas.hpp"

class Wolf3D {
public:
    Wolf3D();
	static Wolf3D* getInstance();
	Physics physics;

	std::shared_ptr<sre::Material> blockMaterial; // #WIP blockTextures
	glm::vec4 textureCoordinates(int blockID);

private:
    void init();
    void update(float deltaTime);
    void render();
	void renderCeiling(sre::RenderPass & renderPass);
	void renderFloor(sre::RenderPass & renderPass);
	void drawGUI();
	void addCube(std::vector<glm::vec3>& vertexPositions, std::vector<glm::vec4>& textureCoordinates, int x, int z, int type);

	static bool instanceFlag;
	static Wolf3D* instance;

    sre::SDLRenderer renderer;
    sre::Camera camera;
	FirstPersonController* fpsController;

    WorldMap map;
    std::shared_ptr<sre::Mesh> walls;
    std::shared_ptr<sre::Material> wallMaterial;

    bool lockRotation = false;
	bool physicsDebugDraw = false;
	bool mouseLock = false;
	bool debugProfiler = true;
	
	btRigidBody* fallRigidBody;
	glm::mat4 sphereTransform;
	std::shared_ptr<sre::Mesh> sphere;
	std::shared_ptr<sre::Material> sphereMaterial;
	btTransform sphereTrans;

	glm::mat4 floorTransform;
	glm::mat4 ceilTransorm;
	glm::vec4 floorColor;
	glm::vec4 ceilColor;
	std::shared_ptr<sre::Mesh> floor;
	std::shared_ptr<sre::Mesh> ceil;
	std::shared_ptr<sre::Material> floorMat;
	std::shared_ptr<sre::Material> ceilMat;
};

