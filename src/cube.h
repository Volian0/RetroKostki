#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <map>
#include <optional>

#include "texture.h"

extern void render_cube(glm::vec3 pos, Texture* texture, class Shader* shader, double delta);

struct Cube
{
	glm::vec3 pos;
	Texture* texture;

	inline void render(class Shader* shader)
	{
		render_cube(pos, texture, shader, anim_tp);
	}

	//bool discovered = true;
	bool is_static = false;
	double anim_tp = 0.0;
	//uint8_t offset = 0;
};