#include "sliding.h"
#include "cube.h"

#include <algorithm>

#include "soloud_wav.h"

extern double player_speed_m;


namespace sliding
{
	std::array<int, 16> blocks =
	{
	1, 6, 3, 0,
	10,13,-1,14,
	2, 8, 7, 12,
	4, 9, 5, 11
	};

	bool motion_enabled = false;

	bool won = false;

	bool inited = false;

	std::array<glm::vec2, 15> blocks_2d_positions;
}

int sliding::get_index(int x, int y)
{
	return y * 4 + x;;
}

glm::i32vec2 sliding::get_cords(int index)
{
	int y = index / 4;
	int x = index % 4;
	return glm::i32vec2(x, y);
}

glm::i32vec2 sliding::get_block_cords(int block_index)
{
	for (int i = 0; i < blocks.size(); ++i)
	{
		if (blocks[i] == block_index)
		{
			return get_cords(i);
		}
	}
	abort();
}

int sliding::get_block_index(int index)
{
	return blocks[index];
}

void sliding::up()
{
	if (!motion_enabled) { return; }
	auto empty_cords = get_block_cords(-1);
	if (empty_cords.y == 3) { return; }
	std::swap(blocks[get_index(empty_cords.x, empty_cords.y + 1)], blocks[get_index(empty_cords.x, empty_cords.y)]);
	is_win();
}

void sliding::down()
{
	if (!motion_enabled) { return; }
	auto empty_cords = get_block_cords(-1);
	if (empty_cords.y == 0) { return; }
	std::swap(blocks[get_index(empty_cords.x, empty_cords.y - 1)], blocks[get_index(empty_cords.x, empty_cords.y)]);
	is_win();
}

void sliding::left()
{
	if (!motion_enabled) { return; }
	auto empty_cords = get_block_cords(-1);
	if (empty_cords.x == 3) { return; }
	std::swap(blocks[get_index(empty_cords.x + 1, empty_cords.y)], blocks[get_index(empty_cords.x, empty_cords.y)]);
	is_win();
}

void sliding::right()
{
	if (!motion_enabled) { return; }
	auto empty_cords = get_block_cords(-1);
	if (empty_cords.x == 0) { return; }
	std::swap(blocks[get_index(empty_cords.x - 1, empty_cords.y)], blocks[get_index(empty_cords.x, empty_cords.y)]);
	is_win();
}

bool sliding::is_win()
{
	if (won)
	{
		return true;
	}
	if (std::is_sorted(blocks.begin(), blocks.end()))
	{
		won = true;
		motion_enabled = false;
		on_win();
		return true;
	}
	return false;
}

extern SoLoud::Soloud soloud;
extern SoLoud::Wav voice3;

void sliding::on_win()
{
	//soloud.play(voice3);
	player_speed_m = 1.25;
}

void sliding::init()
{
	inited = true;

	std::sort(blocks.begin(), blocks.end());
	//set it to 10000
	for (int i = 0; i < 0U; ++i)
	{
		switch (rand() % 4)
		{
		case 0:
		{
			auto empty_cords = get_block_cords(-1);
			if (empty_cords.y == 3) { continue; }
			std::swap(blocks[get_index(empty_cords.x, empty_cords.y + 1)], blocks[get_index(empty_cords.x, empty_cords.y)]);
		}
		break;
		case 1:
		{
			auto empty_cords = get_block_cords(-1);
			if (empty_cords.y == 0) { continue; }
			std::swap(blocks[get_index(empty_cords.x, empty_cords.y - 1)], blocks[get_index(empty_cords.x, empty_cords.y)]);
		}
		break;
		case 2:
		{
			auto empty_cords = get_block_cords(-1);
			if (empty_cords.x == 3) { continue; }
			std::swap(blocks[get_index(empty_cords.x + 1, empty_cords.y)], blocks[get_index(empty_cords.x, empty_cords.y)]);
		}
		break;
		case 3:
		{
			auto empty_cords = get_block_cords(-1);
			if (empty_cords.x == 0) { continue; }
			std::swap(blocks[get_index(empty_cords.x - 1, empty_cords.y)], blocks[get_index(empty_cords.x, empty_cords.y)]);
		}
		break;
		}
	}

	for (int i = 0; i < blocks_2d_positions.size(); ++i)
	{
		blocks_2d_positions[i] = glm::vec2(get_block_cords(i));
	}

	srand(time(0)+ 374U + rand());
}

void sliding::enable()
{
	motion_enabled = true;
	player_speed_m = 0.0;
}

void sliding::update(double delta_time)
{
	for (int i = 0; i < blocks_2d_positions.size(); ++i)
	{
		auto desired_pos = glm::vec2(get_block_cords(i));
		auto diff = desired_pos - blocks_2d_positions[i];
		blocks_2d_positions[i] += diff * float(delta_time) * 16.0f;
	}
}

void sliding::render(Shader* shader)
{
	static std::array<Texture, 15> textures = {
	std::string("res/textures/sliding/0.png"),
	std::string("res/textures/sliding/1.png"),
	std::string("res/textures/sliding/2.png"),
	std::string("res/textures/sliding/3.png"),
	std::string("res/textures/sliding/4.png"),
	std::string("res/textures/sliding/5.png"),
	std::string("res/textures/sliding/6.png"),
	std::string("res/textures/sliding/7.png"),
	std::string("res/textures/sliding/8.png"),
	std::string("res/textures/sliding/9.png"),
	std::string("res/textures/sliding/10.png"),
	std::string("res/textures/sliding/11.png"),
	std::string("res/textures/sliding/12.png"),
	std::string("res/textures/sliding/13.png"),
	std::string("res/textures/sliding/14.png"),
	};

	for (int i = 0; i < blocks_2d_positions.size(); ++i)
	{
		auto pos = blocks_2d_positions[i];
		render_cube(glm::vec3{ pos.x - 963.0, -pos.y + 4, -1003.0 + 0.015625 }, &textures[i], shader, 0.0);
	}
}
