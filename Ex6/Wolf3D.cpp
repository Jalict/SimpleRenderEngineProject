//
// Created by Morten Nobel-Jørgensen on 22/09/2017.
//
#include <glm/gtx/rotate_vector.hpp>
#include "Wolf3D.hpp"
#include <sre/Profiler.hpp>

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

	// TODO TEMP remove
	std::vector<vec3> rays;
	rays.push_back(fpsController->fromRay);
	rays.push_back(fpsController->toRay);
	renderPass.drawLines(rays);

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
}


void Wolf3D::renderChunk(sre::RenderPass & renderPass) {
	for (std::vector<std::shared_ptr<Chunk>>::iterator it = chunkList.begin(); it != chunkList.end(); ++it) {
		std::shared_ptr<Chunk> itChunk = (*it);
		itChunk->draw(renderPass);
	}

//	chunk->draw(renderPass);
}	


void Wolf3D::renderFloor(RenderPass & renderpass) {
	renderpass.draw(floor, floorTransform, floorMat);
}


void Wolf3D::drawGUI() {
	ImGui::SetNextWindowPos(ImVec2(Renderer::instance->getWindowSize().x / 2 - 100, .0f), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_Always);
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
//	ImGui::DragFloat("Rot", &fpsController->rotation);
//	ImGui::DragFloat3("Pos", &(fpsController->position.x), 0.1f);
//	ImGui::Checkbox("LockRotation", &lockRotation);
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
	stoneMesh = initializeMesh(BlockType::Stone);
	brickMesh = initializeMesh(BlockType::Brick);
	grassMesh = initializeMesh(BlockType::Grass);
	woolBlueMesh = initializeMesh(BlockType::WoolBlue);
	sandMesh = initializeMesh(BlockType::Sand);
	dirtMesh = initializeMesh(BlockType::Dirt);
	gravelMesh = initializeMesh(BlockType::Gravel);
	rockMesh = initializeMesh(BlockType::Rock);
	woodMesh = initializeMesh(BlockType::Wood);
	planksMesh = initializeMesh(BlockType::Planks);

	// Create a ground plane
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1.0f, 0)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);

	physics.addRigidBody(groundRigidBody);

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

	//Create a bunch of chunks	
	int chunkDimension = Chunk::getChunkDimensions();
	for (float x = 0; x < 1.0f; x++) {
		for (float y = 0; y < 1.0f; y++) {
			chunkList.push_back(std::make_shared<Chunk>(glm::vec3(x * chunkDimension, 0, y * chunkDimension)));
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
    fpsController->setInitialPosition(vec2(5.5f, 5.5f), 0);


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


std::shared_ptr<sre::Mesh> Wolf3D::initializeMesh(BlockType type) {
	const glm::vec4 coords = textureCoordinates((int)type);
	std::vector<glm::vec4> texCoords;			// texCoords for block
	texCoords.clear();
	texCoords.insert(texCoords.end(), {
		// +z
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

		// ?
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

		// ?
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

		// ?
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

		// top
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0),
		glm::vec4(coords.x,coords.y,0,0), glm::vec4(coords.z,coords.w,0,0), glm::vec4(coords.x,coords.w,0,0),

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
	switch (type) {
	case BlockType::Stone:
		return stoneMesh;
	case BlockType::Brick:
		return brickMesh;
	case BlockType::Grass:
		return grassMesh;
	case BlockType::WoolBlue:
		return woolBlueMesh;
	case BlockType::Sand:
		return sandMesh;
	case BlockType::Dirt:
		return dirtMesh;
	case BlockType::Gravel:
		return gravelMesh;
	case BlockType::Rock:
		return rockMesh;
	case BlockType::Wood:
		return woodMesh;
	case BlockType::Planks:
		return planksMesh;
	}
}


//I didn't know if we still used this, so I just commented it out
//vec4 Wolf3D::textureCoordinates(int blockID){
//	glm::vec2 textureSize(1024, 2048);
//	glm::vec2 tileSize(128, 128);
//
//	float tileWidth = tileSize.x / textureSize.x;
//	float tileHeight = tileSize.y / textureSize.y;
//
//	glm::vec2 min = vec2(0, 16 * tileSize.y) / textureSize;
//	glm::vec2 max = min + tileSize / textureSize;
//
//	min.x += (blockID % 8) * tileWidth * 2;
//	max.x += (blockID % 8) * tileWidth * 2;
//
//	min.y -= ((blockID - (blockID % 8)) / 8) * tileHeight;
//	max.y -= ((blockID - (blockID % 8)) / 8) * tileHeight;
//
//	return vec4(min.x, min.y, max.x, max.y);
//}


Block* Wolf3D::locationToBlock(glm::vec3 location) {
	// Floor the locations since blocks are always at whole intergers
	int x = (int)location.x;
	int y = (int)location.y;
	int z = (int)location.z;

	int chunkSize = Chunk::getChunkDimensions();

	// Determine the chunk we need 
	
	vec3 blockPos = glm::vec3(fmod(x, chunkSize), y, fmod(z, chunkSize));
	vec2 chunkPos = glm::vec2(x - blockPos.x, z - blockPos.z);

	// Get it 
	auto chunk = chunkList.at(0);

	// 
	return chunk->getBlock(blockPos.x, blockPos.y, blockPos.z);
}	


int main(){
    new Wolf3D();
    return 0;
}