#pragma once

#include <windows.h>

class timer
{
public :
	timer()
	{
		LONGLONG counts_per_sec;
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&counts_per_sec));
		seconds_per_count = 1.0 / static_cast<double>(counts_per_sec);

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&curr_time));
		base_time = curr_time;
		prev_time = curr_time;
	}
	~timer() = default;
	timer(const timer&) = delete;
	timer& operator=(const timer&) = delete;	
	timer(timer&&) noexcept = delete;
	timer& operator=(timer&&) noexcept = delete;

	float time_stamp() const
	{
		if (stopped)
		{
			return static_cast<float>(((stop_time - paused_time) - base_time) * seconds_per_count);
		}

		else
		{
			return static_cast<float>(((curr_time - paused_time) - base_time) * seconds_per_count);
		}
	}

	float time_interval() const
	{
		return static_cast<float>(delta_time);
	}

	void reset()
	{
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&curr_time));
		base_time = curr_time;
		prev_time = curr_time;
		stop_time = 0;
		stopped = false;
	}

	void start()
	{
		LONGLONG start_time;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start_time));
		if (stopped)
		{
			paused_time += (start_time - stop_time);
			prev_time = start_time;
			stop_time = 0;
			stopped = false;
		}
	}

	void stop()
	{
		if (!stopped)
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stop_time));
			stopped = true;
		}
	}
	
	void tick()
	{
		if (stopped)
		{
			delta_time = 0.0f;
			return;
		}

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&curr_time));

		delta_time = (curr_time - prev_time) * seconds_per_count;

		prev_time = curr_time;

		if (delta_time < 0.0f)
		{
			delta_time = 0.0f;
		}
	}

private :
	double seconds_per_count = 0.0f;
	double delta_time = 0.0f;

	LONGLONG base_time = 0;
	LONGLONG paused_time = 0;
	LONGLONG stop_time = 0;
	LONGLONG prev_time = 0;
	LONGLONG curr_time = 0;

	bool stopped = false;
};