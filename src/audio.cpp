#include "audio.h"


Timepoint::Timepoint()
	:_timepoint{ std::chrono::steady_clock::now() }
{
}

Timepoint& Timepoint::operator+=(Number duration)
{
	_timepoint = std::chrono::time_point_cast<std::chrono::steady_clock::duration>(_timepoint + std::chrono::duration<Number>(duration));
	return *this;
}

Timepoint Timepoint::operator+(Number duration) const
{
	return Timepoint(*this) += duration;
}

Timepoint& Timepoint::operator-=(Number duration)
{
	return *this += -duration;
}

Timepoint Timepoint::operator-(Number duration) const
{
	return Timepoint(*this) -= duration;
}

Number Timepoint::operator-(const Timepoint& previous_timepoint) const
{
	return std::chrono::duration<Number>(_timepoint - previous_timepoint._timepoint).count();
}

bool Timepoint::operator<(const Timepoint& timepoint) const
{
	return _timepoint < timepoint._timepoint;
}

Number Timepoint::operator%(Number duration) const
{
	return std::chrono::duration<Number>(_timepoint.time_since_epoch() % std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<Number>(duration))).count();
}



NoteEvent::NoteEvent(Type type_, uint8_t key_, uint8_t velocity_)
	:type{ type_ },
	key{ key_ },
	velocity{ velocity_ }
{
}



#define TSF_IMPLEMENTATION
#include <C:/Users/Volian/Downloads/tsf.h>

#include <iostream>

Soundfont::Soundfont(const std::string& filename)
{
	//std::scoped_lock lock(_mutex);
	_ptr = tsf_load_filename(filename.c_str());
	tsf_set_output(reinterpret_cast<tsf*>(_ptr), TSF_STEREO_INTERLEAVED, 44100); //sample rate
	tsf_set_volume(reinterpret_cast<tsf*>(_ptr), 1.0L);
}

Soundfont::~Soundfont()
{
	std::scoped_lock lock(_mutex);
	tsf_close(reinterpret_cast<tsf*>(_ptr));
}

void Soundfont::cc(uint8_t control, uint8_t value)
{
	std::scoped_lock lock(_mutex);
	tsf_channel_midi_control(reinterpret_cast<tsf*>(_ptr), 0, control, value);
}

void Soundfont::note_on(uint8_t key, uint8_t velocity)
{
	std::scoped_lock lock(_mutex);
	tsf_note_on(reinterpret_cast<tsf*>(_ptr), 0, key, Number(velocity) / 127.0L);
	if (_notes_on.count(key))
	{
		std::cout << "Duplicated note on: " << unsigned(key) << std::endl;
	}
	_notes_on.emplace(key);
}

void Soundfont::note_off(uint8_t key)
{
	std::scoped_lock lock(_mutex);
	tsf_note_off(reinterpret_cast<tsf*>(_ptr), 0, key);
	_notes_on.erase(key);
}

void Soundfont::all_notes_off()
{
	std::scoped_lock lock(_mutex);
	tsf_note_off_all(reinterpret_cast<tsf*>(_ptr));
	_notes_on.clear();
}

void Soundfont::reset()
{
	std::scoped_lock lock(_mutex);
	tsf_reset(reinterpret_cast<tsf*>(_ptr));
	_notes_on.clear();
}

void Soundfont::render(uint32_t frames, void* buffer)
{
	play_events();
	std::scoped_lock lock(_mutex);
	tsf_render_short(reinterpret_cast<tsf*>(_ptr), reinterpret_cast<short*>(buffer), frames, 0);
}

void Soundfont::add_event(const Timepoint& timepoint, const NoteEvent& event)
{
	std::scoped_lock lock(_mutex_events);
	if (!_events.empty() && timepoint < _events.rbegin()->first)
	{
		std::cout << "WARNING: Fixing soundfont event position!" << std::endl;
		_events.emplace(_events.rbegin()->first, event);
	}
	else
	{
		_events.emplace(timepoint, event);
	}
}

void Soundfont::play_event(const NoteEvent& event)
{
	if (event.type == NoteEvent::Type::ALL_OFF)
	{
		all_notes_off();
	}
	else if (event.type == NoteEvent::Type::ON)
	{
		note_on(event.key, event.velocity);
	}
	else if (event.type == NoteEvent::Type::OFF)
	{
		note_off(event.key);
	}
}

void Soundfont::play_all_events()
{
	std::scoped_lock lock(_mutex_events);
	for (const auto& [timepoint, event] : _events)
	{
		play_event(event);
	}
	_events.clear();
}

void Soundfont::play_events(const Timepoint& bound)
{
	std::scoped_lock lock(_mutex_events);
	auto lower_bound = _events.lower_bound(bound);
	for (auto it = _events.begin(); it != lower_bound; ++it)
	{
		play_event(it->second);
	}
	_events.erase(_events.begin(), lower_bound);
}


#define MINIAUDIO_IMPLEMENTATION
#include <C:/Users/Volian/Downloads/miniaudio.h>

#include <atomic>

std::atomic_bool active_audio = true;

void AudioDevice::data_callback(ma_device* device, void* output, const void* input, uint32_t frames)
{
	if (!active_audio)
		return;

	AudioDevice& audio_device = *reinterpret_cast<AudioDevice*>(device->pUserData);
	std::scoped_lock lock(audio_device._soundfont_mutex);
	if (audio_device._soundfont)
	{
		audio_device._soundfont->render(frames, output);
	}
}

AudioDevice::AudioDevice()
{
	_ptr = new ma_device;

	ma_device_config device_cfg = ma_device_config_init(ma_device_type_playback);
	device_cfg.playback.format = ma_format_s16;
	device_cfg.playback.channels = 2;
	device_cfg.sampleRate = 44100;
	device_cfg.dataCallback = data_callback;
	device_cfg.pUserData = this;

	ma_device_init(NULL, &device_cfg, reinterpret_cast<ma_device*>(_ptr));
	ma_device_start(reinterpret_cast<ma_device*>(_ptr));
}

AudioDevice::~AudioDevice()
{
	ma_device_stop(reinterpret_cast<ma_device*>(_ptr));
	ma_device_uninit(reinterpret_cast<ma_device*>(_ptr));

	delete reinterpret_cast<ma_device*>(_ptr);
}

void AudioDevice::load_soundfont(const std::shared_ptr<Soundfont>& soundfont)
{
	std::scoped_lock lock(_soundfont_mutex);
	_soundfont = soundfont;
}

std::shared_ptr<Soundfont> AudioDevice::load_soundfont(const std::string& filename)
{
	std::scoped_lock lock(_soundfont_mutex);
	_soundfont.reset();
	_soundfont = std::make_shared<Soundfont>(filename);
	return _soundfont;
}

void AudioDevice::unload_soundfont()
{
	std::scoped_lock lock(_soundfont_mutex);
	_soundfont.reset();
}