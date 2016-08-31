#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

namespace MAD
{
	struct SCmdLine
	{
		static const eastl::string& Get() { return mCmdLine; }

		static void SetCmdLine(const eastl::string& inCmdLine) { mCmdLine = eastl::move(inCmdLine); }

	private:
		static eastl::string mCmdLine;
	};

	struct SParse
	{
		static bool Get(const eastl::string& inStr, const eastl::string& inMatch, int& outVal);
		static bool Get(const eastl::string& inStr, const eastl::string& inMatch, float& outVal);
		static bool Get(const eastl::string& inStr, const eastl::string& inMatch, eastl::string& outVal);

		static bool Find(const eastl::string& inStr, const eastl::string& inMatch);

	private:
		static bool FindMatch(const eastl::string& inStr, const eastl::string& inMatch, size_t& outPos);
	};

	extern class UGameEngine* gEngine;

	class UGameEngine
	{
	public:
		UGameEngine();
		~UGameEngine();

		bool Init(const eastl::string& inGameName, int inWindowWidth, int inWindowHeight);
		void Run();
		void Stop();

		float GetDeltaTime() const { return static_cast<float>(TARGET_DELTA_TIME); }
		float GetFrameTime() const { return static_cast<float>(mFrameTime); }
		float GetGameTime() const { return static_cast<float>(mGameTime); }

		eastl::shared_ptr<class URenderer> GetRenderer() const { return mRenderer; }
		eastl::shared_ptr<class UGameWindow> GetWindow() const { return mGameWindow; }

	private:
		const int MAX_SIMULATION_STEPS = 10;
		const double TARGET_DELTA_TIME = 0.016666666666666666; // 60 FPS

		void Tick();

		bool bContinue;

		double mGameTime;
		double mFrameTime;
		double mFrameAccumulator;

		eastl::vector<eastl::shared_ptr<class UGameWorld>> m_worlds;
		eastl::shared_ptr<class UFrameTimer> mFrameTimer;
		eastl::shared_ptr<class UGameInstance> mGameInstance;
		eastl::shared_ptr<class UGameWindow> mGameWindow;
		eastl::shared_ptr<class URenderer> mRenderer;
	};
}
