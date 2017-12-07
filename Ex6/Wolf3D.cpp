//
// Created by Morten Nobel-Jørgensen on 22/09/2017.
//
#include <glm/gtx/rotate_vector.hpp>
#include "Wolf3D.hpp"
#include <sre/Profiler.hpp>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include <fstream>
#include <iostream>
#include <glm/gtc/matrix_access.inl>

using namespace sre;
using namespace glm;


Wolf3D::Wolf3D() {
	// Singleton-ish
	// TODO clean this to be proper singleton?
	Wolf3D::instance = this;
	Wolf3D::instanceFlag = true;

	// Init renderer and this game
	physics.init();
    renderer.init();
    init();

	// Run Update
    renderer.frameUpdate = [&](float deltaTime){
		physics.update();
        update(deltaTime);

		// Draw falling ball
		fallRigidBody->getMotionState()->getWorldTransform(sphereTrans);
		sphereTransform = glm::translate(vec3(sphereTrans.getOrigin().getX(), sphereTrans.getOrigin().getY(), sphereTrans.getOrigin().getZ()));
    };

	// Render Frame
    renderer.frameRender = [&](){
        render();
    };

	// Handle Keys
    renderer.keyEvent = [&](SDL_Event& e){
        fpsController->onKey(e);
		handleDebugKeys(e);
		
    };

	// Handle mouse events
    renderer.mouseEvent = [&](SDL_Event& e){
		fpsController->onMouse(e);
    };

	// Start the event loop
    renderer.startEventLoop();
}


// TODO clean this to be proper singleton ?
bool Wolf3D::instanceFlag = false;
Wolf3D* Wolf3D::instance = NULL;
Wolf3D* Wolf3D::getInstance(){
	if(!instanceFlag) {
		instance = new Wolf3D();
		instanceFlag = true;
		return instance;
	} else {
		return instance;
	}
}


void Wolf3D::update(float deltaTime) {
    fpsController->update(deltaTime);

	particleSystem->update(deltaTime);
}


void Wolf3D::render() {
	// Create a render pass
	auto renderPass = RenderPass::create()
		.withCamera(camera)
		.withWorldLights(&worldLights)
		.withClearColor(true, { 0.73f, 0.83f, 1, 1 })
		.build();

	// Draw objects 
	// TODO make more generic
	renderPass.draw(sphere, sphereTransform, sphereMaterial);
	renderFloor(renderPass);
	fpsController->draw(renderPass);

	particleSystem->draw(renderPass);

	// TODO TEMP remove
	std::vector<vec3> rays;
	rays.push_back(fpsController->fromRay);
	rays.push_back(fpsController->toRay);
	renderPass.drawLines(rays);

	std::vector<vec3> rays1;
	rays1.push_back(fpsController->fromRay1);
	rays1.push_back(fpsController->toRay2);
	renderPass.drawLines(rays1, vec4(1, 0, 0, 1));

	//We're only drawing one chunk.
	// TODO render a list of chunks.
	renderChunk(renderPass);

	// Allow physics debug drawer to draw
	physics.drawDebug(&renderPass);

	// Draw GUI
	drawGUI();

	// Draw profiler
	if (debugProfiler) {
		static Profiler profiler;
		profiler.update();
		profiler.gui(false);
	}

	// Pass for Crosshair
	sre::Camera simpleCamera;
	auto simplePass = RenderPass::create()
		.withCamera(simpleCamera)
		.withClearColor(false, { 0, 0, 0, 1 })
		.withGUI(false)
		.build();

	std::vector<glm::vec3> cross = {
		glm::vec3(.1,0,0),
		glm::vec3(-.1,0,0),
		glm::vec3(0,.1,0),
		glm::vec3(0,-.1,0)
	};
	simplePass.drawLines(cross);
}


void Wolf3D::renderChunk(sre::RenderPass & renderPass) {

	for (int i = 0; i < chunkArraySize; i++) {
		for (int j = 0; j < chunkArraySize; j++) {
			chunkArray[i][j]->draw(renderPass);
		}
	}
}	


void Wolf3D::renderFloor(RenderPass & renderpass) {
	renderpass.draw(floor, floorTransform, floorMat);
}


void Wolf3D::drawGUI() {
	ImGui::SetNextWindowPos(ImVec2(Renderer::instance->getWindowSize().x / 2 - 100, .0f), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Always);
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

	glm::vec3 pos = fpsController->getPosition();
	glm::vec3 lookPos = fpsController->toRay;
	ImGui::Text("pos: %.1f %.1f %.1f", pos.x, pos.y, pos.z);
	ImGui::Text("lookAt: %.1f %.1f %.1f", lookPos.x, lookPos.y, lookPos.z);

	ImGui::End();
}

void Wolf3D::handleDebugKeys(SDL_Event& e) {
	// Toggle debug drawing of physics with 1
	if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_1) {
		physicsDebugDraw = !physicsDebugDraw;
		if (physicsDebugDraw)
			physics.setDebugDrawMode(btIDebugDraw::DBG_DrawWireframe);
		else
			physics.setDebugDrawMode(btIDebugDraw::DBG_NoDebug);
	}

	// Toggle debug profiler
	if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_2) {
		debugProfiler = !debugProfiler;
	}

	// Toggle mouse capture
	if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE) {
		mouseLock = !mouseLock;
		renderer.setMouseCursorVisible(mouseLock);
		renderer.setMouseCursorLocked(!mouseLock);
		SDL_SetWindowGrab(renderer.getSDLWindow(), mouseLock ? SDL_TRUE : SDL_FALSE);
		SDL_SetRelativeMouseMode(mouseLock ? SDL_TRUE : SDL_FALSE);
		fpsController->lockRotation = !mouseLock;
	}
}

void Wolf3D::updateApperance()
{
	particleSystem->updateAppearance = [&](const Particle& p) {
		p.color = glm::mix(colorFrom, colorTo, p.normalizedAge);
		p.size = glm::mix(sizeFrom, sizeTo, p.normalizedAge);
	};
}

void Wolf3D::updateEmit()
{
	particleSystem->emitter = [&](Particle& p) {
		p.position = emitPosition;
		p.velocity = glm::sphericalRand(emitVelocity);
		p.rotation = 90;
		p.angularVelocity = 10;
		p.size = 50;
	};
}


void Wolf3D::init() {
	// TODO Clean up +  dealloc

	// Initialise all the block meshes
//	loadBlocks("blocks.json");

	// TODO make sure this is Dealloc this!
	// Create all the blocks
	blockMeshes = new std::shared_ptr<sre::Mesh>[BlockType::LENGTH];
	for (int i = 0; i < BlockType::LENGTH; i++) {
		blockMeshes[i] = initializeMesh((BlockType)i);
	}


	// Create a ground plane
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1.5f, 0))); // #HACK floor needs ot be at -1.5f ?? should be -.5f??
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);


	physics.addRigidBody(groundRigidBody);
	btTransform transform;
	groundRigidBody->getMotionState()->getWorldTransform(transform);

	// Particle System
	particleTexture = sre::Texture::getWhiteTexture();
	particleSystem = std::make_shared<ParticleSystem>(10, particleTexture);
	particleSystem->gravity = { 0,-.2,0 };
	particleMaterial = sre::Shader::getStandard()->createMaterial();
	updateApperance();
	updateEmit();

	// Create falling ball
	btCollisionShape* fallShape = new btSphereShape(1.0f);
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));

	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	fallShape->calculateLocalInertia(mass, fallInertia);

	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
	fallRigidBody = new btRigidBody(fallRigidBodyCI);
	
	physics.addRigidBody(fallRigidBody);

	// Create ball mesh
	sphere = Mesh::create().withSphere(16, 32, 1.0f).build();
	sphereMaterial = Shader::getStandard()->createMaterial();
	sphereMaterial->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	//Create the chunks	
	int chunkDimension = Chunk::getChunkDimensions();
	
	// TODO make sure this is Dealloc this!
	chunkArray = new std::shared_ptr<Chunk>*[chunkArraySize];
	for (int i = 0; i < chunkArraySize; i++) {
		chunkArray[i] = new std::shared_ptr<Chunk>[chunkArraySize];
	}

	for (int x = 0; x < chunkArraySize; x++) {
		for (int z = 0; z < chunkArraySize; z++) {
			chunkArray[x][z] = std::make_shared<Chunk>(glm::vec3(x * chunkDimension, 0, z * chunkDimension));
		}
	}

	// Directional Light
	worldLights.addLight(Light::create()
		.withDirectionalLight(glm::normalize(glm::vec3(0.7f, 0.7f, 0.7f)))
		.build());

	// Load and create walls
	blockMaterial = Shader::getStandard()->createMaterial();
	auto tiles = Texture::create().withFile("tileset.png")
		.withGenerateMipmaps(false)
		.withFilterSampling(false)
		.build();
	blockMaterial->setTexture(tiles);

	// Setup FPS Controller
	fpsController = new  FirstPersonController(&camera);
    fpsController->setPosition(vec3(-1.0f, 10.0f, -1.0f), 0);

	// Create floor
	floor = Mesh::create().withQuad(100).build();
	floorMat = Shader::getStandard()->createMaterial();
	floorMat->setColor(vec4(.44f, .44f, .44f, 1.0f));
	floorTransform = glm::translate(vec3(0, -1.0f, 0));
	floorTransform = glm::rotate(floorTransform, glm::radians(-90.0f), vec3(1, 0, 0));

	// setMouseCursorLocked state correctly
	mouseLock = !mouseLock;
	renderer.setMouseCursorVisible(mouseLock);
	renderer.setMouseCursorLocked(!mouseLock);
	SDL_SetWindowGrab(renderer.getSDLWindow(), mouseLock ? SDL_TRUE : SDL_FALSE);
	SDL_SetRelativeMouseMode(mouseLock ? SDL_TRUE : SDL_FALSE);
	fpsController->lockRotation = !mouseLock;
}


// TODO set sites correctly
std::shared_ptr<sre::Mesh> Wolf3D::initializeMesh(BlockType type) {
	// Store the texture coordinates in a vector
	std::vector<glm::vec4> texCoords;			
	texCoords.clear();

	// Collect texture coordinates for each side
	glm::vec4 coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Front));
	texCoords.insert(texCoords.end(), {
		// +z
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0)
	});
	
	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Left));
	texCoords.insert(texCoords.end(), {
		// ?
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Back));
	texCoords.insert(texCoords.end(),{
		// ?
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Right));
	texCoords.insert(texCoords.end(),{
		// ?
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Top));
	texCoords.insert(texCoords.end(),{
		// top
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Bottom));
	texCoords.insert(texCoords.end(),{
		// bottom
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	return sre::Mesh::create().withCube(0.5f).withUVs(texCoords).build();
}


glm::vec4 Wolf3D::textureCoordinates(int blockID) {
	glm::vec2 textureSize(1024, 2048);
	glm::vec2 tileSize(128, 128);

	float tileWidth = tileSize.x / textureSize.x;
	float tileHeight = tileSize.y / textureSize.y;

	glm::vec2 min = glm::vec2(0, 1.0f);							// Start at top left
	glm::vec2 max = min + glm::vec2(tileWidth, -tileHeight);	// Move max to bottom right corner of this block

	int tilesX = textureSize.x / tileSize.x;

	float xOffset = (blockID % tilesX) * tileWidth;
	float yOffset = ((blockID - (blockID % tilesX)) / tilesX) * tileHeight;

	min.x += xOffset;
	max.x += xOffset;

	min.y -= yOffset;
	max.y -= yOffset;

	return glm::vec4(min.x, min.y, max.x, max.y);
}


std::shared_ptr<sre::Mesh> Wolf3D::getMesh(BlockType type) {
	return blockMeshes[(int)type];
}

/*
void Wolf3D::loadBlocks(std::string fromFile) {
	using namespace rapidjson;
	std::ifstream fis(fromFile);
	IStreamWrapper isw(fis);
	Document doc;
	doc.ParseStream(isw);
	

	auto blocks = doc["blocks"].GetArray();
	std::cout << blocks[0]["top"].GetInt() << std::endl;

	// Loop over all blocks and load them
	for (int i = 0; i < BlockType::LENGTH; i++) {

	}
	
}*/


Block* Wolf3D::locationToBlock(int x,  int y,  int z) {
	int chunkSize = Chunk::getChunkDimensions();

	// Determine the chunk we need 
	vec3 blockPos = glm::vec3(x % chunkSize, y % chunkSize, z % chunkSize);
	vec2 chunkPos = glm::vec2((x - blockPos.x)/Chunk::getChunkDimensions(), (z - blockPos.z)/ Chunk::getChunkDimensions());

	// If the chunk does not exist, return null pointerf
	if(chunkPos.x < 0 || chunkPos.y < 0 || chunkPos.x >= chunkArraySize || chunkPos.y >= chunkArraySize){
		std::cout << "chunk doesnt exist" << std::endl;
		return nullptr;
	}
		
	// Else get the right block
	auto chunk = chunkArray[(int)chunkPos.x][(int)chunkPos.y];

	// Get the block on the chunk
	return chunk->getBlock((int)blockPos.x, (int)blockPos.y, (int)blockPos.z);
}	


int main(){
    new Wolf3D();
    return 0;
}