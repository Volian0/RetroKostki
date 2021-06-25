#include "player.h"

#include <algorithm>

#include "minesweeper.h"



//#include "audio.h"

extern bool motion;

extern float yaw;

double player_speed_m = 1.0;

void Player::move_forward(double delta_time, double angle, const std::vector<Cube>& cubes)
{
	if (!motion)
		return;

	glm::vec3 old_pos = pos;
	glm::vec3 new_pos = pos;

	double speed_m = (2.0 - std::min(std::abs(y_velocity / 4.0), 2.0)) * player_speed_m;

	new_pos.x = pos.x + cos(glm::radians(yaw + angle)) * delta_time * speed_m;
	new_pos.z = pos.z + sin(glm::radians(yaw + angle)) * delta_time * speed_m;

	pos.x = new_pos.x;
	if (check_collision(cubes))
		pos.x = std::round(pos.x);

	pos.z = new_pos.z;
	if (check_collision(cubes))
		pos.z = std::round(pos.z);
}



bool Player::check_collision(const std::vector<Cube>& cubes)
{
	for (const auto& cube : cubes)
	{
		if (std::abs(pos.x - cube.pos.x) < 1.0 && (pos.y - cube.pos.y < 2.0f && cube.pos.y - pos.y < 1.0f) && std::abs(pos.z - cube.pos.z) < 1.0)
			return true;
	}
	return false;
}

#include <soloud.h>
#include <soloud_wav.h>
#include <iostream>

extern SoLoud::Soloud soloud;

extern SoLoud::Wav sfx_jump;
extern SoLoud::Wav sfx_land;

void Player::gravity(double delta_time, const std::vector<Cube>& cubes)
{
	if (!motion)
		return;
	glm::vec3 old_pos = pos;
	y_velocity = std::max<>(y_velocity - delta_time * 8.0, -32.0);
	pos.y += y_velocity * delta_time;
	bool was_on_ground = on_ground;
	on_ground = false;
	if (check_collision(cubes))
	{
		on_ground = y_velocity <= 0.0;
		if (!was_on_ground && on_ground)
		{
			soloud.play(sfx_land);
			if (minesweeper::allow_modify)
			{
				//x = od 1 do 8
				//y = -1
				//z = od -999 do -992
				intmax_t px = std::round(pos.x) - 1.0;
				intmax_t pz = std::round(pos.z) + 999.0;
				if (px < 8 && pz < 8 && px >= 0 && pz >= 0)
				{
					minesweeper::cur_x = pz;
					minesweeper::cur_y = px;
					minesweeper::uncover();
				}
			}
		}
		y_velocity = 0.0;
		pos.y = std::round(pos.y);
	}
}

//AudioDevice device;
//auto soundfont = device.load_soundfont("res/soundfonts/piano.sf2");

extern uint8_t yacieowo;

void Player::jump()
{
	if (!motion)
		return;

	if (on_ground)
	{
		y_velocity = 5.0f;
		//soundfont->play_all_events();
		//soundfont->all_notes_off();
		//uint8_t offset = rand() % 3;
		//Timepoint now;
		//soundfont->add_event(now, NoteEvent(NoteEvent::Type::ALL_OFF));
		//soundfont->add_event(now, NoteEvent(NoteEvent::Type::ON, 30 + rand()%48, 40+rand()%64));
		soloud.play(sfx_jump);
		yacieowo = (yacieowo + 1) % 4;
	}
}
