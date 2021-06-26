#pragma once

#include <array>

#include <glm/glm.hpp>
#include <C:\LearnOpenGL-master\includes\learnopengl\shader_m.h>

namespace sliding
{
	extern std::array<int, 16> blocks;
	extern std::array<glm::vec2, 15> blocks_2d_positions;

	extern bool motion_enabled;

	extern bool inited;

	extern bool won;

	extern int get_index(int x, int y);
	extern glm::i32vec2 get_cords(int index);

	extern glm::i32vec2 get_block_cords(int block_index);
	extern int get_block_index(int index);

	extern void up();
	extern void down();
	extern void left();
	extern void right();

	extern bool is_win();
	extern void on_win();

	extern void init();

	extern void enable();

	extern void update(double delta_time);

	extern void render(Shader* shader);
};