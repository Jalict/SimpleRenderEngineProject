#include "Block.hpp"
#include "Physics.hpp"
#include "Wolf3D.hpp"



Block::Block() { 
}


Block::Block(BlockType type, glm::vec3 position) {
	// Set the type of the block
	setType(type);
	this->position = position;
}


Block::~Block() {
	removeColliderFromWorld();

	if(rigidbody != nullptr){
		delete rigidbody->getMotionState();
		delete rigidbody;
	}
	if(collider != nullptr){
		delete collider;
	}
}


void Block::initCollider() {
	// Setup the physics, but do not add it to the world yet. 
	// Just have it ready for when we need it.
	collider = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));

	btScalar mass(0);
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(btScalar(position.x), btScalar(position.y), btScalar(position.z)));
	btVector3 localInertia(0, 0, 0);
	btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, collider, localInertia);
	rigidbody = new btRigidBody(cInfo);
}


void Block::addColliderToWorld() {
	// Only add rigidbodies for blocks that are active and not yet in the physics world.
	if (active && !inPhysicsWorld){
		Wolf3D::getInstance()->physics.addRigidBody(rigidbody);
		inPhysicsWorld = true;
	}
}


void Block::removeColliderFromWorld() {
	// Only remove rigidbodies for blocks that are in the physics world.
	if(inPhysicsWorld){
		Wolf3D::getInstance()->physics.removeRigidBody(rigidbody);
		inPhysicsWorld = false;
	}
}


void Block::setType(BlockType type) {
	this->type = type;
}


void Block::setActive(bool active) {
	// If the block is already in the correct state, we do not have to do anything.
	if(this->active == active)
		return;

	// Bedrock cannot be deactivated, therefore stop if this is  bedrock.
	if (type == BlockType::Bedrock)
		return;

	// If this is grass see if there is something above, if so turn into dirt since grass needs air
	if(type == BlockType::Grass){
		Block* b = Wolf3D::getInstance()->locationToBlock(position.x, position.y + 1, position.z, true);

		// If there is a block above us and it is active, this grass must turn into dirt.
		if (b != nullptr && b->isActive()) {
				type = BlockType::Dirt;
		}
	}
	
	// If block below is grass turn it into dirt, since now there is something on top.
	auto b = Wolf3D::getInstance()->locationToBlock(position.x, position.y - 1, position.z, false);
	if (b != nullptr && active && b->isActive() && b->getType() == BlockType::Grass){
		b->setType(BlockType::Dirt);
	}

	// If the block is activated, add its collider back to the world.
	if(active){
		addColliderToWorld();
	}
	// Else if the block is deactivated, remove the collider form the world.
	else {
		removeColliderFromWorld();
		Wolf3D::getInstance()->placeParticleSystem(position);
	}

	// Set the activation state
	this->active = active;
}


int Block::getTextureIndex(BlockType type, BlockSides side){
	switch (type) {
		case BlockType::Stone:
			return 0;
		case BlockType::Brick:
			return 1;
		case BlockType::Dirt:
			return 9;
		case BlockType::Gravel:
			return 25;
		case BlockType::Rock:
			return 50;
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
		case BlockType::Grass:
			switch (side) {
			case BlockSides::Top:
				return 23;
			case BlockSides::Bottom:
				return 9;
			default:
				return 10;
			}
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
		default:
			return 0;
	}
}