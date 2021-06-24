#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <array>

namespace minesweeper
{
	extern bool winner;
	extern int cur_x;
	extern int cur_y;

	extern bool allow_modify;

	extern std::array<bool, 64> tiles1;
	extern std::array<uint8_t, 64> tiles2;

	extern int mines;
	extern int uncovered;

	extern bool valid_position(int x, int y);
	extern int get_index(int x, int y);
	extern int count_near(int x, int y);
	extern void start_game();
	extern void game_win();
	extern void toggle_texture(int tile, int frame);
	extern void game_over();
	extern void hard_reset();
	extern void game_over_reset();
	extern void pop(int x, int y);
	extern void uncover();

	extern void init();
};