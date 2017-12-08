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

	// Update physics
	stepChunkPhysicsInit();

	for (int i = 0; i < chunkArrayX; i++) {
		for (int j = 0; j < chunkArrayY; j++) {
			for (int k = 0; k < chunkArrayZ; k++) {
				chunkArray[i][j][k]->update(deltaTime);
			}
		}
	}

	// Update particle systems
	particleSystem->update(deltaTime);

	if(particleSystem->emitting) {
		elapsedParticleTime += deltaTime;
		if (elapsedParticleTime > 0.05f) {
			elapsedParticleTime = 0;
			particleSystem->emitting = false;
		}
	}
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

	// TODO TEMP remove
	std::vector<vec3> rays;
	rays.push_back(fpsController->fromRay);
	rays.push_back(fpsController->toRay);
	renderPass.drawLines(rays);

	std::vector<vec3> rays1;
	rays1.push_back(fpsController->fromRay1);
	rays1.push_back(fpsController->toRay2);
	renderPass.drawLines(rays1, vec4(1, 0, 0, 1));

	renderChunk(renderPass);

	// Draw Particles
	particleSystem->draw(renderPass);

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
	for (int x = 0; x < chunkArrayX; x++) {
		for (int y = 0; y < chunkArrayY; y++) {
			for (int z = 0; z < chunkArrayZ; z++) {
				chunkArray[x][y][z]->draw(renderPass);
			}
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
	ImGui::ProgressBar(fpsController->getMinedAmount());

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


void Wolf3D::init() {
	// TODO Clean up +  dealloc

	// Initialise all the block meshes
//	loadBlocks("blocks.json");

	// TODO make sure this is Dealloc this!
	// Create all the blocks
	blockMeshes = new std::shared_ptr<sre::Mesh>[BlockType::LENGTH];
	for (int i = 0; i < BlockType::LENGTH; i++) {
		blockMeshes[i] = createBlockMesh((BlockType)i);
	}


	// Create a ground plane
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1.5f, 0))); // #HACK floor needs ot be at -1.5f ?? should be -.5f??
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);


	physics.addRigidBody(groundRigidBody);
	btTransform transform;
	groundRigidBody->getMotionState()->getWorldTransform(transform);

	// Create falling ball
	btCollisionShape* fallShape = new btSphereShape(1.0f);
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));

	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	fallShape->calculateLocalInertia(mass, fallInertia);

	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
	fallRigidBody = new btRigidBody(fallRigidBodyCI);
	
	physics.addRigidBody(fallRigidBody);

	// Particle System
	particleTexture = sre::Texture::getWhiteTexture();
	particleSystem = std::make_shared<ParticleSystem>(10, particleTexture);
	particleSystem->gravity = { 0, -9.82, 0 };

	updateApperance();
	updateEmit();

	// Create ball mesh
	sphere = Mesh::create().withSphere(16, 32, 1.0f).build();
	sphereMaterial = Shader::getStandard()->createMaterial();
	sphereMaterial->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	//Create the chunks	
	int chunkDimension = Chunk::getChunkDimensions();
	
	// TODO make sure this is Dealloc this!
	chunkArray = new std::shared_ptr<Chunk>**[chunkArrayX];
	for (int x = 0; x < chunkArrayX; x++) {
		chunkArray[x] = new std::shared_ptr<Chunk>*[chunkArrayY];
		for (int y = 0; y < chunkArrayY; y++) {
			chunkArray[x][y] = new std::shared_ptr<Chunk>[chunkArrayZ];
		}
	}

	for (int x = 0; x < chunkArrayX; x++) {
		for (int y = 0; y < chunkArrayY; y++) {
			for (int z = 0; z < chunkArrayZ; z++) {
				chunkArray[x][y][z] = std::make_shared<Chunk>(glm::vec3(x * chunkDimension, y * chunkDimension, z * chunkDimension));
			}
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
	// Spawn the player in the middle of the world.
    fpsController->setPosition(vec3(chunkArrayX * Chunk::getChunkDimensions() *  0.5f, Chunk::getChunkDimensions() + 3.0f, chunkArrayZ * Chunk::getChunkDimensions() *  0.5f), 0);

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

	// Setup physics for the first chunk
	stepChunkPhysicsInit();
}


// # TODO add a cooldown to a change, so not everything is constantly switching when the player is on a chunk edge
// # TOOD rename function
void  Wolf3D::stepChunkPhysicsInit() {
	vec3 playerPosition = fpsController->getPosition();

	int xPos = (int)(playerPosition.x / Chunk::getChunkDimensions());
	int yPos = (int)(playerPosition.y / Chunk::getChunkDimensions());
	int zPos = (int)(playerPosition.z / Chunk::getChunkDimensions());

	// Loop over all chunks
	for (int x = 0; x < chunkArrayX; x++){
		for (int y = 0; y < chunkArrayY; y++){
			for (int z = 0; z < chunkArrayZ; z++){
				auto chunk = getChunk(x, y, z);

				if(chunk == nullptr)
					continue;

				// Enable the 9 surrounding chunks
				if (x >= xPos - 1 && x <= xPos + 1 && y >= yPos - 1 && y <= yPos + 1 && z >= zPos - 1 && z <= zPos + 1) {
					chunk->addCollidersToWorld();
				}
				// Disable the others
				else {
					chunk->removeCollidersFromWorld();
				}
			}
		}
	}
}


// This function creates a complete block mesh for a specific type with the correct texture coordinates.
std::shared_ptr<sre::Mesh> Wolf3D::createBlockMesh(BlockType type) {
	// Store the uv coordinates in a vector
	std::vector<glm::vec4> uvs;			
//	uvs.clear();

	// Collect texture coordinates for each side
	glm::vec4 coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Front));
	uvs.insert(uvs.end(), { // z+
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0)
	});
	
	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Left));
	uvs.insert(uvs.end(), {
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Back));
	uvs.insert(uvs.end(),{
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Right));
	uvs.insert(uvs.end(),{
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Top));
	uvs.insert(uvs.end(),{ // top
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	coords = textureCoordinates(Block::getTextureIndex(type, BlockSides::Bottom));
	uvs.insert(uvs.end(),{ // bottom
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),
	});

	return sre::Mesh::create().withCube(0.5f).withUVs(uvs).withName("BlockInHandMesh").build();
}


// Function translates a texture index to UV coordinates
glm::vec4 Wolf3D::textureCoordinates(int textureId) {
	glm::vec2 textureSize(1024, 2048);
	glm::vec2 tileSize(128, 128);

	float tileWidth = tileSize.x / textureSize.x;
	float tileHeight = tileSize.y / textureSize.y;

	glm::vec2 min = glm::vec2(0, 1.0f);							// Start at top left
	glm::vec2 max = min + glm::vec2(tileWidth, -tileHeight);	// Move max to bottom right corner of this block

	int tilesX = textureSize.x / tileSize.x;

	float xOffset = (textureId % tilesX) * tileWidth;
	float yOffset = ((textureId - (textureId % tilesX)) / tilesX) * tileHeight;

	min.x += xOffset;
	max.x += xOffset;

	min.y -= yOffset;
	max.y -= yOffset;

	return glm::vec4(min.x, min.y, max.x, max.y);
}


// This function translate any position to a block pointer (inside the chunk) if any exists.
Block* Wolf3D::locationToBlock(int x, int y, int z, bool ghostInspect) {
	// Determine the chunk coordinates, and local block coordinates.
	vec3 blockPos = glm::vec3(x % Chunk::getChunkDimensions(), y % Chunk::getChunkDimensions(), z % Chunk::getChunkDimensions());
	vec3 chunkPos = glm::vec3((x - blockPos.x) / Chunk::getChunkDimensions(), (y - blockPos.y) / Chunk::getChunkDimensions(), (z - blockPos.z) / Chunk::getChunkDimensions());

	// Get a pointer to the chunk we want to access.
	auto chunk = getChunk((int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z);

	// If we tried to get a chunk which does not exist, we can already return and don't need to do anything else.
	if (chunk == nullptr)
		return nullptr;

	// If ghost mode is not activated, changes can occur to this block.
	// Thus we should notify surrounding chunks to recalculate if necessary.
	if (!ghostInspect) {
		// Check if we need to update chunk left.
		if (blockPos.x == 0) {
			auto neighbour = getChunk(chunkPos.x - 1, chunkPos.y, chunkPos.z);

			if (neighbour != nullptr){
				neighbour->flagRecalculateMesh();
			}
		}
		// Check if we need to update chunk right.
		else if (blockPos.x >= Chunk::getChunkDimensions() - 1) {
			auto neighbour = getChunk(chunkPos.x + 1, chunkPos.y, chunkPos.z);

			if (neighbour != nullptr){
				neighbour->flagRecalculateMesh();
			}
		}


		// Check if we need to update chunk below.
		if (blockPos.y == 0) {
			auto neighbour = getChunk(chunkPos.x, chunkPos.y - 1, chunkPos.z);

			if (neighbour != nullptr){
				neighbour->flagRecalculateMesh();
			}
		}
		// Check if we need to update chunk above.
		else if (blockPos.y >= Chunk::getChunkDimensions() - 1) {
			auto neighbour = getChunk(chunkPos.x, chunkPos.y + 1, chunkPos.z);

			if (neighbour != nullptr){
				neighbour->flagRecalculateMesh();
			}
		}


		// Check if we need to update chunk in front.
		if (blockPos.z == 0) {
			auto neighbour = getChunk(chunkPos.x, chunkPos.y, chunkPos.z - 1);

			if (neighbour != nullptr){
				neighbour->flagRecalculateMesh();
			}
		}
		// Check if we need to update chunk in behind.
		else if (blockPos.z >= Chunk::getChunkDimensions() - 1) {
			auto neighbour = getChunk(chunkPos.x, chunkPos.y, chunkPos.z + 1);

			if (neighbour != nullptr){
				neighbour->flagRecalculateMesh();
			}
		}
	}

	// Flag this chunk for recalculation if needed
	if(!ghostInspect)
		chunk->flagRecalculateMesh();

	// Return the actual block that was requested;
	return chunk->getBlock((int)blockPos.x, (int)blockPos.y, (int)blockPos.z);
}


std::shared_ptr<Chunk> Wolf3D::getChunk(int x, int y, int z) {
	// If the chunk does not exist, return null pointer
	if (x < 0 || y < 0 || z < 0 || x >= chunkArrayX || y >= chunkArrayY || z >= chunkArrayZ) {
		return nullptr;
	}

	// Else get the right block
	return chunkArray[x][y][z];
}

void Wolf3D::placeParticleSystem(glm::vec3 pos) {
	particleSystem->emitting = true;

	emitPosition = pos;

	particleSystem->emit();
}


void Wolf3D::updateApperance() {
	particleSystem->updateAppearance = [&](const Particle& p) {
		p.size = glm::mix(sizeFrom, sizeTo, p.normalizedAge);
		p.color = { 0.17f,0.08f,0.02f,1 };
	};
}

void Wolf3D::updateEmit() {
	particleSystem->emitter = [&](Particle& p) {
		p.position = emitPosition;
		p.velocity = glm::sphericalRand(emitVelocity);
		p.rotation = emitRotation;
		p.angularVelocity = emitAngularVelocity;
		p.size = sizeFrom;
	};
}


// Returns a pointer to a Mesh for a complete cube
std::shared_ptr<sre::Mesh> Wolf3D::getBlockMesh(BlockType type) {
	return blockMeshes[(int)type];
}


int main(){
    new Wolf3D();
    return 0;
}