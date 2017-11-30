#pragma once

class Voxel {
public:
	Voxel();
	~Voxel();

	void draw();
	void setType(VoxelType);
private:
	VoxelType type;
};

enum VoxelType {
	Grass,
	Rock
};