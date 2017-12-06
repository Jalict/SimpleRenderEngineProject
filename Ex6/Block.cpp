#include <iostream>
#include "Block.hpp"
#include "Physics.hpp"
#include "Wolf3D.hpp"

Block::Block() : Block(BlockType::Dirt, glm::vec3(0,0,0)) { // HACK if type is being passed on type = -8100000000000 or something (not initialized? mem loc? out of bounds?). Therefore we set default hardcoded here. 
}

Block::Block(BlockType type, glm::vec3 position) {
	// Set the type of the block
	setType(type);

	//this->position = position;

	// Create Mesh
	mesh = sre::Mesh::create()
		.withCube(0.5f)
		.withUVs(texCoords)
		.build();

	// # TODO dealloc
	// Add physics collider
	btBoxShape* blockShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
	
	btTransform startTransform;
	startTransform.setIdentity();
	btScalar mass(0);
	startTransform.setOrigin(btVector3(btScalar(position.x), btScalar(position.y), btScalar(position.z)));
	createRigidBody(mass, startTransform, blockShape);
}

Block::~Block() {

}

void Block::setType(BlockType type) {
	this->type = type;

	const glm::vec4 coords = textureCoordinates((int)this->type);

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

	std::cout << "type set" << std::endl;
}

std::shared_ptr<sre::Mesh> Block::getMesh()
{
	return mesh;
}

//(TODO) Move to somewhere more generic
// FPL, 1 December 2017
glm::vec4 Block::textureCoordinates(int blockID) {
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

glm::vec3 Block::getPosition()
{
	return position;
}


// TODO move to proper location / clean up
// http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Simple_Box
btRigidBody* Block::createRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape) {
//	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(cInfo);

//  Dont know what this does. Documentation does not have this listed.
//	body->setUserIndex(-1); 
	Wolf3D::getInstance()->physics.addRigidBody(body);
	return body;
}