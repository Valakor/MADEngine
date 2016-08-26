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

		float GetDeltaTime() const { return mDeltaTime; }

		// Set the FPS cap. Positive values limit max FPS, negative values remove any cap.
		void SetFPSCap(float FPS = -1.0f) { mFrameStep = 1.0f / FPS; }
		float GetFPSCap() const { return 1.0f / mFrameStep; }
		float GetFrameStep() const { return mFrameStep; }

		shared_ptr<class URenderer> GetRenderer() const { return mRenderer; }
		shared_ptr<class UGameWindow> GetWindow() const { return mGameWindow; }

	private:
		void Tick();

		bool bContinue;

		float mDeltaTime;
		float mFrameStep;

		shared_ptr<class UFrameTimer> mFrameTimer;
		shared_ptr<class UGameInstance> mGameInstance;
		shared_ptr<class UGameWindow> mGameWindow;
		shared_ptr<class URenderer> mRenderer;
	};
}
