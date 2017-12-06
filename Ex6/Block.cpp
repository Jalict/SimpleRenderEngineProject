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

	// # TODO dealloc
	// Add physics collider
	collider = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
	
	btTransform startTransform;
	startTransform.setIdentity();
	btScalar mass(0);
	startTransform.setOrigin(btVector3(btScalar(position.x), btScalar(position.y), btScalar(position.z)));
	rigidbody = createRigidBody(mass, startTransform, collider);
}

Block::~Block() {
	// TODO destroy physics
}

void Block::setMesh(BlockType type) {
	this->mesh = Wolf3D::getInstance()->getMesh(type);
}

void Block::setType(BlockType type) {
	this->type = type;
	setMesh(type);
}

std::shared_ptr<sre::Mesh> Block::getMesh(){
	return mesh;
}

glm::vec3 Block::getPosition(){
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

bool Block::getActive() {
	return this->active;
}

// # TODO test between the different modes
void Block::setActive(bool active) {
	// If the block is already in the correct state, we do not have to do anything.
	if(this->active == active)
		return;

	this->active = active;
	if(active)
		Wolf3D::getInstance()->physics.addRigidBody(rigidbody);
//		rigidbody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_STATIC_OBJECT);
	else
		Wolf3D::getInstance()->physics.removeRigidBody(rigidbody);
//		rigidbody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE);

}