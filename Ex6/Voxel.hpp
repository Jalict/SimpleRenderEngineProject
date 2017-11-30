#pragma once

#include "sre/SDLRenderer.hpp"

class Voxel {
public:
	Voxel();
	~Voxel();

	void draw(sre::RenderPass& renderpass);
	void setType(VoxelType type);
private:
	VoxelType type;
};

enum VoxelType {
	Grass,
	Rock
};