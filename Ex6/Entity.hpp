#pragma once

#include "sre/SDLRenderer.hpp"

/*
Created: 31-11-2017
Entity in the world that you can pick up
Has physics, colliders and will move around. All Entities are the same, except for their appearence
*/

class Entity {
public:
	Entity(sre::Sprite* sprite);
	~Entity();

	void update(float dt);
	void draw(sre::RenderPass& renderpass);
private:
	sre::Sprite* sprite;
	glm::vec3 position;
};

