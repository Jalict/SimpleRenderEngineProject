#include <iostream>
#include "Block.hpp"
#include "Physics.hpp"
#include "Wolf3D.hpp"

Block::Block() : Block(BlockType::Dirt, glm::vec3(0,0,0)) { // HACK if type is being passed on type = -8100000000000 or something (not initialized? mem loc? out of bounds?). Therefore we set default hardcoded here. 
}

Block::Block(BlockType type, glm::vec3 position) {
	// Set the type of the block
	setType(type);

	// # TODO dealloc
	// Add physics collider
	collider = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));

	// # TODO are we using position??
	this->position = position;
	
	// Add a physics collider to it
	btTransform startTransform;
	startTransform.setIdentity();
	btScalar mass(0);
	startTransform.setOrigin(btVector3(btScalar(position.x), btScalar(position.y), btScalar(position.z)));
	rigidbody = createRigidBody(mass, startTransform, collider);
}


Block::~Block() {
	// TODO destroy physics
}


void Block::setType(BlockType type) {
	this->type = type;
}


bool Block::isActive() {
	return this->active;
}

// # TODO test between the different modes
void Block::setActive(bool active) {
	// If the block is already in the correct state, we do not have to do anything.
	if(this->active == active)
		return;

	// Bedrock cannot be deactivated
	if (type == BlockType::Bedrock)
		return;

	// If this is grass see if there is something above, if so turn into dirt since grass needs air
	Block* b = Wolf3D::getInstance()->locationToBlock(position.x, position.y + 1, position.z, true);
	if (type == BlockType::Grass && b != nullptr && b->isActive()) 
			type = BlockType::Dirt;
	
	// If block below is grass turn it into dirt, since now there is something on top
	b = Wolf3D::getInstance()->locationToBlock(position.x, position.y - 1, position.z, false);
	if (b != nullptr && b->isActive() && b->getType() == BlockType::Grass)
		b->setType(BlockType::Dirt);

	this->active = active;

	if(active){
//		Wolf3D::getInstance()->physics.addRigidBody(rigidbody);
		addColliderToWorld();
//		rigidbody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_STATIC_OBJECT);
	}
	else {

		removeColliderFromWorld();
		//Wolf3D::getInstance()->physics.removeRigidBody(rigidbody);
		Wolf3D::getInstance()->placeParticleSystem(position);
		//rigidbody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE);
	}
}


void Block::addColliderToWorld() {
	if(active)
		Wolf3D::getInstance()->physics.addRigidBody(rigidbody);
}

void Block::removeColliderFromWorld(){
//	if(!active)
	Wolf3D::getInstance()->physics.removeRigidBody(rigidbody);
}


// Function returns the correct texture index for a block type on the correct side
int Block::getTextureIndex(BlockType type, BlockSides side){
	switch (type) {
		case BlockType::Stone:
			return 0;
		case BlockType::Brick:
			return 1;
		case BlockType::Grass:
			switch (side){
				case BlockSides::Top:
					return 23;
				case BlockSides::Bottom:
					return 9;
				default:
					return 10;
			}
		case BlockType::Dirt:
			return 9;
		case BlockType::Gravel:
			return 25;
		case BlockType::Rock:
			return 50;
		case BlockType::Wood:
			switch (side) {
			case BlockSides::Top:
				return 75;
			case BlockSides::Bottom:
				return 75;
			default:
				return 74;
			}
		case BlockType::WorkBench:
			switch (side) {
			case BlockSides::Top:
				return 67;
			case BlockSides::Bottom:
				return 83;
			default:
				return 83;
			}
		case BlockType::IronOre:
			return 51;
		case BlockType::CoalOre:
			return 53;
		case BlockType::DiamondOre:
			return 55;
		case BlockType::Planks:
			return 83;
		case BlockType::Bedrock:
			return 27;
		case BlockType::Glass:
			return 16;
		default:
			return 0;
	}
}


// #TODO are we using this naywere?
glm::vec3 Block::getPosition() {
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
	// Wolf3D::getInstance()->physics.addRigidBody(body);
	return body;
}