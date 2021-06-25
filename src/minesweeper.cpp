#include "minesweeper.h"

#include "cube.h"
#include "texture.h"
#include "soloud_wav.h"
#include "thread"
#include "chrono"

extern std::vector<Cube> cubes;

extern SoLoud::Soloud soloud;
extern SoLoud::Wav sfx_pop;
extern SoLoud::Wav sfx_boom;

extern SoLoud::Wav bg_music2;

extern SoLoud::handle bg_music2i;

namespace minesweeper
{
    int cur_x = 3;
    int cur_y = 4;

    bool allow_modify = false;
	bool winner = false;

    std::array<bool, 64> tiles1;
    std::array<uint8_t, 64> tiles2;

    int mines = 1; //set it to 9!
    int uncovered = 0;
}

bool minesweeper::valid_position(int x, int y)
{
    return (x >= 0 && x <= 7 && y >= 0 && y <= 7);
}

int minesweeper::get_index(int x, int y)
{
    return y * 8 + x;
}

int minesweeper::count_near(int x, int y)
{
	int near_mines = 0;
	for (int ix = x - 1; ix <= x + 1; ++ix) {
		for (int iy = y - 1; iy <= y + 1; ++iy) {
			if (valid_position(ix, iy)) {
				if (tiles1[get_index(ix, iy)]) {
					++near_mines;
				}
			}
		}
	}
	return near_mines;
}

void minesweeper::start_game()
{
	for (int i = 0; i < mines;) {
		int index = rand() % 64;
		if (tiles1[index] == false) {
			tiles1[index] = true;
			++i;
		}
	}
	allow_modify = true;
}

void minesweeper::game_win()
{
	allow_modify = false;
	bg_music2.stop();
	//EntFire("script", "RunScriptCode", "vote()", 2);
	for (int i = 0; i < 64; ++i) {
		if (tiles1[i]) {
			toggle_texture(i, 10);
		}
	}
	winner = true;
}

int ms_offset_t = 0;

void minesweeper::toggle_texture(int tile, int frame)
{
	static std::array<Texture, 12> textures = {
		std::string("res/textures/mine/covered.png"),
		std::string("res/textures/mine/uncovered_1.png"),
		std::string("res/textures/mine/uncovered_2.png"),
		std::string("res/textures/mine/uncovered_3.png"),
		std::string("res/textures/mine/uncovered_4.png"),
		std::string("res/textures/mine/uncovered_5.png"),
		std::string("res/textures/mine/uncovered_6.png"),
		std::string("res/textures/mine/uncovered_7.png"),
		std::string("res/textures/mine/uncovered_8.png"),
		std::string("res/textures/mine/uncovered_0.png"),
		std::string("res/textures/mine/uncovered_bomb_safe.png"),
		std::string("res/textures/mine/uncovered_bomb.png")
	};
	if (cubes[ms_offset_t + tile].texture != &textures[frame])
	{
		cubes[ms_offset_t + tile].anim_tp = 1.0f;
	}
	cubes[ms_offset_t + tile].texture = &textures[frame];
}


void minesweeper::game_over()
{
	allow_modify = false;
	soloud.setVolume(bg_music2i, 0.125);
	soloud.play(sfx_boom);
	for (int i = 0; i < 64; ++i) {
		if (tiles1[i]) {
			toggle_texture(i, 11);
		}
	}
	std::thread t(game_over_reset);
	t.detach();
}

void minesweeper::hard_reset()
{
	static uint64_t yacioooooro = 0;
	srand(time(0) + yacioooooro);
	yacioooooro++;
	for (int i = 0; i < 64; ++i) {
		toggle_texture(i, 0);
		tiles1[i] = false;
		tiles2[i] = 0;
	}
	uncovered = 0;
	start_game();
}

void minesweeper::game_over_reset()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	soloud.setVolume(bg_music2i, 1.25);
	hard_reset();
}

void minesweeper::pop(int x, int y)
{
	++uncovered;
	int index = get_index(x, y);
	tiles2[index] = 1;
	int near_mines = count_near(x, y);
	if (near_mines == 0)
	{
		toggle_texture(index, 9);
		for (int ix = x - 1; ix <= x + 1; ++ix) {
			for (int iy = y - 1; iy <= y + 1; ++iy) {
				if (valid_position(ix, iy)) {
					if (tiles2[get_index(ix, iy)] != 1) {
						pop(ix, iy);
					}
				}
			}
		}
	}
	else
	{
		toggle_texture(index, near_mines);
	}
	if (uncovered + mines == 64)
	{
		game_win();
	}
}

void minesweeper::uncover()
{
	int index = get_index(cur_x, cur_y);
	if (!allow_modify || tiles2[index] != 0) { return; }
	if (tiles1[index]) {
		if (uncovered == 0) {
			while (tiles1[index]) {
				hard_reset();
			}
			uncover();
		}
		else
		{
			game_over();
		}
	}
	else {
		pop(cur_x, cur_y);
		soloud.play(sfx_pop);
	}
}

void minesweeper::init()
{
}
