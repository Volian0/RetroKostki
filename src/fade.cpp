#include "fade.h"

#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <learnopengl/filesystem.h>
#include <optional>
#include <algorithm>

float acc_time = 0;
float total_time = 1.0;
glm::vec3 _my_colorf;

void fade::fade(glm::vec3 color, float duration)
{
	acc_time = duration;
	total_time = duration;
	_my_colorf = color;
}

void fade::update(float delta, Shader* shaderz)
{
	if (acc_time > 0.0f)
	{
		acc_time -= delta;
		shaderz->setVec4("fade_color", glm::vec4(_my_colorf, std::max<float>(acc_time / total_time, 0.0f)));
	}
}
