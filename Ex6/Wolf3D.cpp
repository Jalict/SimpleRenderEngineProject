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
            .build();

	// Draw objects 
	// TODO make more generic
	renderPass.draw(sphere, sphereTransform, sphereMaterial);
	renderFloor(renderPass);

	//We're only drawing one chunk.
	// TODO render a list of chunks.
	renderChunk(renderPass);
	renderPass.draw(walls, glm::mat4(1), wallMaterial);

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
	sphereMaterial = Shader::getUnlit()->createMaterial();
	sphereMaterial->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	//Create an example chunk
	glm::mat4 chunkTransform = glm::translate(vec3(10.0f, 0.0f, -10.0f));
	chunk = new Chunk(chunkTransform);

	// Load and create walls
    wallMaterial = Shader::getUnlit()->createMaterial();
    auto texture = Texture::create().withFile("level0.png")
            .withGenerateMipmaps(false)
            .withFilterSampling(false)
            .build();
    wallMaterial->setTexture(texture);

	// Load and create walls
	blockMaterial = Shader::getUnlit()->createMaterial();
	auto tiles = Texture::create().withFile("tileset.png")
		.withGenerateMipmaps(false)
		.withFilterSampling(false)
		.build();
	blockMaterial->setTexture(tiles);

	// Load serialized level
	map.loadMap("level0.json");

    std::vector<glm::vec3> vertexPositions;
    std::vector<glm::vec4> textureCoordinates;

    for (int x=0; x < map.getWidth(); x++){
        for (int y=0; y < map.getHeight(); y++){
            int field = map.getTile(x,y);

            if (field != -1){
//			addCube(vertexPositions,textureCoordinates,x,y,field);
            }
        }
    }

	walls = Mesh::create()
		.withPositions(vertexPositions)
		.withUVs(textureCoordinates)
		.build();


	// Setup FPS Controller
	fpsController = new  FirstPersonController(&camera);
    fpsController->setInitialPosition(map.getStartingPosition(), map.getStartingRotation());


	// Create floor
	floor = Mesh::create().withQuad(100).build();

	floorMat = Shader::getUnlit()->createMaterial();
	floorMat->setColor(map.getFloorColor());

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


vec4 Wolf3D::textureCoordinates(int blockID){
	glm::vec2 textureSize(1024, 2048);
	glm::vec2 tileSize(128, 128);

	float tileWidth = tileSize.x / textureSize.x;
	float tileHeight = tileSize.y / textureSize.y;

	glm::vec2 min = vec2(0, 16 * tileSize.y) / textureSize;
	glm::vec2 max = min + tileSize / textureSize;

	min.x += (blockID % 8) * tileWidth * 2;
	max.x += (blockID % 8) * tileWidth * 2;

	min.y -= ((blockID - (blockID % 8)) / 8) * tileHeight;
	max.y -= ((blockID - (blockID % 8)) / 8) * tileHeight;

	return vec4(min.x, min.y, max.x, max.y);
}


int main(){
    new Wolf3D();
    return 0;
}