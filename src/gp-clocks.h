/*
GamePulse for OBS — stream / recording session clocks.
Fed by obs_frontend events on the OBS main thread; readable from any thread.
Recording clock subtracts paused time. Millisecond precision is sufficient
for chapter markers, journals and YouTube chapters.
*/

#pragma once

#include <atomic>
#include <cstdint>

namespace gamepulse {

class SessionClock {
public:
	void start(int64_t now_ms)
	{
		start_ms_ = now_ms;
		paused_accum_ms_ = 0;
		pause_began_ms_ = -1;
		active_ = true;
	}

	void stop()
	{
		active_ = false;
		pause_began_ms_ = -1;
	}

	void pause(int64_t now_ms)
	{
		if (active_ && pause_began_ms_ < 0)
			pause_began_ms_ = now_ms;
	}

	void unpause(int64_t now_ms)
	{
		if (active_ && pause_began_ms_ >= 0) {
			paused_accum_ms_ += now_ms - pause_began_ms_;
			pause_began_ms_ = -1;
		}
	}

	bool active() const { return active_; }

	/* elapsed session ms at wall time now_ms; -1 when inactive */
	int64_t elapsed(int64_t now_ms) const
	{
		if (!active_)
			return -1;
		int64_t paused = paused_accum_ms_;
		int64_t pb = pause_began_ms_;
		if (pb >= 0)
			paused += now_ms - pb;
		int64_t e = now_ms - start_ms_ - paused;
		return e < 0 ? 0 : e;
	}

	int64_t started_at() const { return start_ms_; }

private:
	std::atomic<bool> active_{false};
	std::atomic<int64_t> start_ms_{0};
	std::atomic<int64_t> paused_accum_ms_{0};
	std::atomic<int64_t> pause_began_ms_{-1};
};

} // namespace gamepulse
