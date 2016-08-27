#pragma once

#include <cstdint>

namespace MAD
{
	class UFrameTimer
	{
	public:
		UFrameTimer();

		void Start();
		float GetFrameTime(float inFrameStep = -1.0f);

	private:
		uint64_t mFreq;
		uint64_t mStart;

		float GetElapsed() const;
	};
}
