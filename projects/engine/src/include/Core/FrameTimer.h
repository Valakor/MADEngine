#pragma once

namespace MAD
{
	class UFrameTimer
	{
	public:
		UFrameTimer();

		void Start();
		void Checkpoint();

		double TimeSinceCheckpoint() const;
		double TimeSinceStart() const;

	private:
		double mTimerStart;
		double mLastCheckpoint;
	};
}
