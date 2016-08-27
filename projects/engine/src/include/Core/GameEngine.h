#pragma once

#include "Engine.h"

namespace MAD
{
	struct SCmdLine
	{
		static const string& Get() { return mCmdLine; }

		static void SetCmdLine(const string& inCmdLine) { mCmdLine = std::move(inCmdLine); }

	private:
		static string mCmdLine;
	};

	struct SParse
	{
		static bool Get(const string& inStr, const string& inMatch, int& outVal);
		static bool Get(const string& inStr, const string& inMatch, float& outVal);
		static bool Get(const string& inStr, const string& inMatch, string& outVal);

		static bool Find(const string& inStr, const string& inMatch);

	private:
		static bool FindMatch(const string& inStr, const string& inMatch, size_t& outPos);
	};

	extern class UGameEngine* gEngine;

	class UGameEngine
	{
	public:
		UGameEngine();
		~UGameEngine();

		bool Init(const string& inGameName, int inWindowWidth, int inWindowHeight);
		void Run();
		void Stop();

		float GetDeltaTime() const { return static_cast<float>(TARGET_DELTA_TIME); }
		float GetFrameTime() const { return static_cast<float>(mFrameTime); }
		float GetGameTime() const { return static_cast<float>(mGameTime); }

		shared_ptr<class URenderer> GetRenderer() const { return mRenderer; }
		shared_ptr<class UGameWindow> GetWindow() const { return mGameWindow; }

	private:
		const int MAX_SIMULATION_STEPS = 10;
		const double TARGET_DELTA_TIME = 0.016666666666666666; // 60 FPS

		void Tick();

		bool bContinue;

		double mGameTime;
		double mFrameTime;
		double mFrameAccumulator;

		vector<shared_ptr<class UGameWorld>> m_worlds;
		shared_ptr<class UFrameTimer> mFrameTimer;
		shared_ptr<class UGameInstance> mGameInstance;
		shared_ptr<class UGameWindow> mGameWindow;
		shared_ptr<class URenderer> mRenderer;
	};
}
