/*
* Initially Created by Morten Nobel-Jørgensen on 29/09/2017.
*
* FirstPersonController - included in project from: 30-11-2017
* Wrapper for all the bullet physics.
*/
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
	// TODO clean this to be proper singleton
	Wolf3D::instance = this;
	Wolf3D::instanceFlag = true;

	// First initialize physics, then the renderer and lastly the game itself.
	physics.init();
    renderer.init();
    init();

	// Run Update, first update physics, then the game.
    renderer.frameUpdate = [&](float deltaTime){
		physics.update();
        update(deltaTime);
    };

	// Render Frame
    renderer.frameRender = [&](){
        render();
    };

	// Handle Keys
    renderer.keyEvent = [&](SDL_Event& e){
        fpsController->onKey(e);
		onKey(e);
		
    };

	// Handle mouse events
    renderer.mouseEvent = [&](SDL_Event& e){
		fpsController->onMouse(e);
    };

	// Start the event loop
    renderer.startEventLoop();
}


Wolf3D::~Wolf3D() {
	delete blockMeshes;

	for (int x = 0; x < chunkArrayX; x++) {
		for (int y = 0; y < chunkArrayY; y++) {
			delete chunkArray[x][y];
		}
		delete chunkArray[x];
	}
	delete chunkArray;
}


// TODO clean this to be proper singleton 
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
	// Update the FPS controller
    fpsController->update(deltaTime);

	// Only load colliders near the player
	vec3 playerPosition = fpsController->getPosition();
	loadColliders((int)(playerPosition.x / Chunk::chunkSize), (int)(playerPosition.y / Chunk::chunkSize), (int)(playerPosition.z / Chunk::chunkSize));

	// Update all chunks
	for (int i = 0; i < chunkArrayX; i++) {
		for (int j = 0; j < chunkArrayY; j++) {
			for (int k = 0; k < chunkArrayZ; k++) {
				chunkArray[i][j][k]->update(deltaTime);
			}
		}
	}

	// Update particle systems
	particleSystem->update(deltaTime);

	// # TODO move into particle system?
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
	drawChunks(renderPass);
	fpsController->draw(renderPass);

	// Draw Particles
	particleSystem->draw(renderPass);

	// Allow physics debug drawer to draw if enabled
	if(physicsDebugDraw)
		physics.drawDebug(&renderPass);

	// Draw GUI
	drawGUI();

	// Draw profiler if enabled.
	if (debugProfiler) {
		static Profiler profiler;
		profiler.update();
		profiler.gui(false);
	}

	// Create a second renderpass for the crosshair.
	sre::Camera simpleCamera;
	auto simplePass = RenderPass::create()
		.withCamera(simpleCamera)
		.withClearColor(false, { 0, 0, 0, 1 })
		.withGUI(false)
		.build();

	// Draw the crosshair.
	std::vector<glm::vec3> cross = {
		glm::vec3(.1,0,0),
		glm::vec3(-.1,0,0),
		glm::vec3(0,.1,0),
		glm::vec3(0,-.1,0)
	};
	simplePass.drawLines(cross);
}


void Wolf3D::drawChunks(sre::RenderPass & renderPass) {
	for (int x = 0; x < chunkArrayX; x++) {
		for (int y = 0; y < chunkArrayY; y++) {
			for (int z = 0; z < chunkArrayZ; z++) {
				chunkArray[x][y][z]->draw(renderPass);
			}
		}
	}
}	


void Wolf3D::drawGUI() {
	ImGui::SetNextWindowPos(ImVec2(Renderer::instance->getWindowSize().x / 2 - 100, .0f), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Always);
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

	// Show mining progress
	ImGui::ProgressBar(fpsController->getMinedAmount());

	// Show the position of the FPS controller
	glm::vec3 pos = fpsController->getPosition();
	ImGui::Text("pos: %.1f %.1f %.1f", pos.x, pos.y, pos.z);

	// Show the block we last interacted with
	glm::vec3 lookPos = fpsController->getLookAt();
	ImGui::Text("lookAt: %.1f %.1f %.1f", lookPos.x, lookPos.y, lookPos.z);

	ImGui::End();
}


void Wolf3D::onKey(SDL_Event& e) {
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
		fpsController->setLockRotation(!mouseLock);
	}
}


void Wolf3D::init() {
//	loadBlocks("blocks.json");

	// Setup the material used by all blocks
	blockMaterial = Shader::getStandard()->createMaterial();
	auto tiles = Texture::create().withFile("tileset.png")
		.withGenerateMipmaps(false)
		.withFilterSampling(false)
		.build();
	blockMaterial->setTexture(tiles);


	// Setup a block mesh for all blocktypes we have. 
	// These are used to display a block in the hand of the controller.
	blockMeshes = new std::shared_ptr<sre::Mesh>[BlockType::LENGTH];
	for (int i = 0; i < BlockType::LENGTH; i++) {
		blockMeshes[i] = createBlockMesh((BlockType)i);
	}


	// Setup the Particle System
	particleTexture = sre::Texture::getWhiteTexture();
	particleSystem = std::make_shared<ParticleSystem>(10, particleTexture);
	particleSystem->gravity = { 0, -9.82, 0 };

	updateApperance();
	updateEmit();


	// Locally store the size of the chunks, since we will be using it a lot
	int chunkSize = Chunk::chunkSize;
	
	// Setup the multidimensional chunk array which holds all our chunks
	chunkArray = new std::shared_ptr<Chunk>**[chunkArrayX];
	for (int x = 0; x < chunkArrayX; x++) {
		chunkArray[x] = new std::shared_ptr<Chunk>*[chunkArrayY];
		for (int y = 0; y < chunkArrayY; y++) {
			chunkArray[x][y] = new std::shared_ptr<Chunk>[chunkArrayZ];
		}
	}

	// Fill the chunk arrays
	for (int x = 0; x < chunkArrayX; x++) {
		for (int y = 0; y < chunkArrayY; y++) {
			for (int z = 0; z < chunkArrayZ; z++) {
				chunkArray[x][y][z] = std::make_shared<Chunk>(glm::vec3(x * chunkSize, y * chunkSize, z * chunkSize));
			}
		}
	}


	// Directional Light
	worldLights.addLight(Light::create()
		.withDirectionalLight(glm::normalize(glm::vec3(0.7f, 0.7f, 0.7f)))
		.build());


	// Setup FPS Controller
	fpsController = new  FirstPersonController(&camera);

	// Spawn the player in the middle of the world.
    fpsController->translateController(vec3(chunkArrayX * Chunk::chunkSize *  0.5f, Chunk::chunkSize + 3.0f, chunkArrayZ * Chunk::chunkSize *  0.5f), 0);


	// Setup the mouse lock to our default state
	renderer.setMouseCursorVisible(mouseLock);
	renderer.setMouseCursorLocked(!mouseLock);
	SDL_SetWindowGrab(renderer.getSDLWindow(), mouseLock ? SDL_TRUE : SDL_FALSE);
	SDL_SetRelativeMouseMode(mouseLock ? SDL_TRUE : SDL_FALSE);
	fpsController->setLockRotation(!mouseLock);

	// Setup the colliders to be loaded near the player
	// Only load colliders near the player
	vec3 playerPosition = fpsController->getPosition();
	loadColliders((int)(playerPosition.x / Chunk::chunkSize), (int)(playerPosition.y / Chunk::chunkSize), (int)(playerPosition.z / Chunk::chunkSize));
}


// # TODO add a cooldown to a change, so not everything is constantly switching when the player is on a chunk edge
void  Wolf3D::loadColliders(int xPos, int yPos, int zPos) {
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


// # TODO rename function
std::shared_ptr<sre::Mesh> Wolf3D::createBlockMesh(BlockType type) {
	// Store the uv coordinates in a vector
	std::vector<glm::vec4> uvs;			

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


// # TODO rename function
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


// # TODO rename function
Block* Wolf3D::locationToBlock(int x, int y, int z, bool ghostInspect) {
	// Determine the chunk coordinates, and local block coordinates.
	vec3 blockPos = glm::vec3(x % Chunk::chunkSize, y % Chunk::chunkSize, z % Chunk::chunkSize);
	vec3 chunkPos = glm::vec3((x - blockPos.x) / Chunk::chunkSize, (y - blockPos.y) / Chunk::chunkSize, (z - blockPos.z) / Chunk::chunkSize);

	// Get a pointer to the chunk we want to access.
	auto chunk = getChunk((int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z);

	// If we tried to get a chunk which does not exist, we can already return and don't need to do anything else.
	if (chunk == nullptr)
		return nullptr;

	// If ghost mode is not activated, changes can occur to this block.
	// Thus we should notify surrounding chunks to recalculate if necessary.
	// And ourself
	if (!ghostInspect) {
		flagNeighboursForRecalculateIfNecessary(x, y, z);
	}

	// Return the actual block that was requested;
	return chunk->getBlock((int)blockPos.x, (int)blockPos.y, (int)blockPos.z);
}


// # TODO rename function
void Wolf3D::flagNeighboursForRecalculateIfNecessary(int x,  int y, int z) {
	vec3 blockPos = glm::vec3(x % Chunk::chunkSize, y % Chunk::chunkSize, z % Chunk::chunkSize);
	vec3 chunkPos = glm::vec3((x - blockPos.x) / Chunk::chunkSize, (y - blockPos.y) / Chunk::chunkSize, (z - blockPos.z) / Chunk::chunkSize);

	// Check if we need to update chunk left.
	if (blockPos.x == 0) {
		auto neighbour = getChunk(chunkPos.x - 1, chunkPos.y, chunkPos.z);

		if (neighbour != nullptr) {
			neighbour->flagRecalculateMesh();
		}
	}
	// Check if we need to update chunk right.
	else if (blockPos.x >= Chunk::chunkSize - 1) {
		auto neighbour = getChunk(chunkPos.x + 1, chunkPos.y, chunkPos.z);

		if (neighbour != nullptr) {
			neighbour->flagRecalculateMesh();
		}
	}

	// Check if we need to update chunk below.
	if (blockPos.y == 0) {
		auto neighbour = getChunk(chunkPos.x, chunkPos.y - 1, chunkPos.z);

		if (neighbour != nullptr) {
			neighbour->flagRecalculateMesh();
		}
	}
	// Check if we need to update chunk above.
	else if (blockPos.y >= Chunk::chunkSize - 1) {
		auto neighbour = getChunk(chunkPos.x, chunkPos.y + 1, chunkPos.z);

		if (neighbour != nullptr) {
			neighbour->flagRecalculateMesh();
		}
	}


	// Check if we need to update chunk in front.
	if (blockPos.z == 0) {
		auto neighbour = getChunk(chunkPos.x, chunkPos.y, chunkPos.z - 1);

		if (neighbour != nullptr) {
			neighbour->flagRecalculateMesh();
		}
	}
	// Check if we need to update chunk in behind.
	else if (blockPos.z >= Chunk::chunkSize - 1) {
		auto neighbour = getChunk(chunkPos.x, chunkPos.y, chunkPos.z + 1);

		if (neighbour != nullptr) {
			neighbour->flagRecalculateMesh();
		}
	}

	// We always need to update this chunk
	auto chunk = getChunk(chunkPos.x, chunkPos.y, chunkPos.z);

	if(chunk != nullptr)
		chunk->flagRecalculateMesh();
}


std::shared_ptr<Chunk> Wolf3D::getChunk(int x, int y, int z) {
	// If the chunk is not within bounds, return null pointer
	if (x < 0 || y < 0 || z < 0 || x >= chunkArrayX || y >= chunkArrayY || z >= chunkArrayZ) {
		return nullptr;
	}

	// Otherwise, we can just return the chunk requested
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


int main(){
    new Wolf3D();
    return 0;
}