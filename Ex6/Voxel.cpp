#include "Voxel.hpp"

Voxel::Voxel() {
	type = VoxelType::Rock;
}

Voxel::~Voxel() {

}

void Voxel::draw(sre::RenderPass& pass) {

}

void Voxel::setType(VoxelType type) {
	this->type = type;
}