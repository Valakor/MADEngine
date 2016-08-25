#include "Core/FrameTimer.h"

namespace MAD
{
	UFrameTimer::UFrameTimer()
	{
		QueryPerformanceFrequency(&mFreq);
	}

	void UFrameTimer::Start()
	{
		QueryPerformanceFrequency(&mFreq);
		QueryPerformanceCounter(&mStart);
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
		return static_cast<float>(end.QuadPart - mStart.QuadPart) / mFreq.QuadPart;
	}
}
