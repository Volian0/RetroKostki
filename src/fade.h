#pragma once

#include <glm/glm.hpp>

#include <C:\LearnOpenGL-master\includes\learnopengl\shader_m.h>

namespace fade
{
	void fade(glm::vec3 color, float duration);
	void update(float delta, class Shader* shaderz);
};