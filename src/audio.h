#pragma once

#include <string>
#include <mutex>
#include <map>
#include <unordered_set>

using Number = long double;

#include <chrono>

class Timepoint
{
public:
	//create a timepoint that represents the current point in time
	Timepoint();

	//add duration to the timepoint
	Timepoint& operator+=(Number duration);
	Timepoint operator+(Number duration) const;

	//subtract duration from the timepoint
	Timepoint& operator-=(Number duration);
	Timepoint operator-(Number duration) const;

	//create a duration by subtracting the previous timepoint
	Number operator-(const Timepoint& previous_timepoint) const;

	//compare with the other timepoint
	bool operator<(const Timepoint& timepoint) const;

	Number operator%(Number duration) const;

private:
	std::chrono::time_point<std::chrono::steady_clock> _timepoint;
};

struct NoteEvent
{
	enum class Type : uint8_t {
		ON, OFF, ALL_OFF
	} type;
	uint8_t key, velocity;

	NoteEvent() = default;
	NoteEvent(Type type_, uint8_t key_ = 0, uint8_t velocity_ = 0);
};

class Soundfont
{
public:
	Soundfont(const std::string& filename);
	~Soundfont();

	void cc(uint8_t control, uint8_t value);
	void note_on(uint8_t key, uint8_t velocity);
	void note_off(uint8_t key);
	void all_notes_off();
	void reset();

	void render(uint32_t frames, void* buffer);

	void add_event(const Timepoint& timepoint, const NoteEvent& event);
	void play_event(const NoteEvent& event);
	void play_all_events();
	void play_events(const Timepoint& bound = {});

private:
	mutable std::mutex _mutex;
	mutable std::mutex _mutex_events;
	std::multimap<Timepoint, NoteEvent> _events;
	void* _ptr;
	std::unordered_set<uint8_t> _notes_on;
};


#include <memory>
#include <mutex>

class AudioDevice
{
public:
	AudioDevice();
	~AudioDevice();

	void load_soundfont(const std::shared_ptr<Soundfont>& soundfont);
	std::shared_ptr<Soundfont> load_soundfont(const std::string& filename);
	void unload_soundfont();

	static void data_callback(struct ma_device* device, void* output, const void* input, uint32_t frames);

private:
	void* _ptr;
	std::shared_ptr<Soundfont> _soundfont;
	std::mutex _soundfont_mutex;
};
