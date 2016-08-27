#include "Core/FrameTimer.h"

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace MAD
{
	UFrameTimer::UFrameTimer()
	{
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&mFreq));
	}

	void UFrameTimer::Start()
	{
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&mFreq));
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&mStart));
	}

	float UFrameTimer::GetFrameTime(float inFrameStep)
	{
		float frameTime = GetElapsed();
		
		if (inFrameStep > 0.0f)
		{
			while (frameTime < inFrameStep)
			{
				frameTime = GetElapsed();
			}
			frameTime = inFrameStep;
		}

		Start();

		return frameTime;
	}

	float UFrameTimer::GetElapsed() const
	{
		LARGE_INTEGER end;
		QueryPerformanceCounter(&end);
		return static_cast<float>(end.QuadPart - mStart) / mFreq;
	}
}
