#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <map>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "cube.h"

class Player
{
public:
	glm::vec3 pos = {-1, 1.0, 10};
	void move_forward(double delta_time, double angle, const std::vector<Cube>& cubes);
	bool check_collision(const std::vector<Cube>& cubes);
	void gravity(double delta_time, const std::vector<Cube>& cubes);
	void jump();

	double y_velocity = 0;
	bool on_ground = false;

private:

	
};

