#include "Core/FrameTimer.h"

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace MAD
{
	double Now()
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);

		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);

		return static_cast<double>(now.QuadPart) / freq.QuadPart;
	}

	UFrameTimer::UFrameTimer(): mTimerStart(0.0)
	                          , mLastCheckpoint(0.0)
	{ }

	void UFrameTimer::Start()
	{
		mTimerStart = mLastCheckpoint = Now();
	}

	void UFrameTimer::Checkpoint()
	{
		mLastCheckpoint = Now();
	}

	double UFrameTimer::TimeSinceCheckpoint() const
	{
		return Now() - mLastCheckpoint;
	}

	double UFrameTimer::TimeSinceStart() const
	{
		return Now() - mTimerStart;
	}
}
