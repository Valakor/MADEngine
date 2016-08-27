#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace MAD
{
	class UFrameTimer
	{
	public:
		UFrameTimer();

		void Start();
		float GetFrameTime(float inFrameStep = -1.0f);

	private:
		LARGE_INTEGER mFreq;
		LARGE_INTEGER mStart;

		float GetElapsed() const;
	};
}
