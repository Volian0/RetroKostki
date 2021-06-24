#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <map>

class Texture
{
public:
	Texture(const std::string& filename);

	void use();

private:
	uint32_t _id;
};