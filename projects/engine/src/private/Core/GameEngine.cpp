#include "Core/GameEngine.h"

#include "Core/FrameTimer.h"
#include "Core/GameInstance.h"
#include "Misc/Assert.h"
#include "Misc/ErrorHandling.h"
#include "Rendering/Renderer.h"
#include "Misc/utf8conv.h"

namespace MAD
{
	string SCmdLine::mCmdLine;

	bool SParse::FindMatch(const string& inStr, const string& inMatch, size_t& outPos)
	{
		auto pos = inStr.find(inMatch);
		if (inMatch.size() == 0 || pos == string::npos)
		{
			return false;
		}

		outPos = pos + inMatch.size();
		return true;
	}

	bool SParse::Find(const string& inStr, const string& inMatch)
	{
		size_t pos;
		return SParse::FindMatch(inStr, inMatch, pos);
	}

	bool SParse::Get(const string& inStr, const string& inMatch, int& outVal)
	{
		size_t pos;
		if (!SParse::FindMatch(inStr, inMatch, pos))
		{
			return false;
		}

		outVal = atoi(&inStr[pos]);
		return true;
	}

	bool SParse::Get(const string& inStr, const string& inMatch, float& outVal)
	{
		size_t pos;
		if (!SParse::FindMatch(inStr, inMatch, pos))
		{
			return false;
		}

		outVal = static_cast<float>(atof(&inStr[pos]));
		return true;
	}
	
	bool SParse::Get(const string& inStr, const string& inMatch, string& outVal)
	{
		size_t pos;
		if (!SParse::FindMatch(inStr, inMatch, pos))
		{
			return false;
		}

		auto endPos = inStr.find_first_of(" \t", pos);
		outVal = inStr.substr(pos, endPos - pos);
		return true;
	}

	DECLARE_LOG_CATEGORY(LogGameEngine);

	UGameEngine* gEngine = nullptr;

	UGameEngine::UGameEngine(): bContinue(true)
	                          , mDeltaTime(0)
	                          , mFrameStep(0) { }

	bool UGameEngine::Init(const string& inGameName, int inWindowWidth, int inWindowHeight)
	{
		UGameWindow::SetWorkingDirectory();

		SCmdLine::SetCmdLine(utf8util::UTF8FromUTF16(GetCommandLineW()));
		ULog::Get().Init();

		LOG(LogGameEngine, Log, "Engine initialization begin...\n");
		LOG(LogGameEngine, Log, "Commandline: %s\n", SCmdLine::Get().c_str());

		// Set global engine ptr
		gEngine = this;

		// Create a window
		mGameWindow = std::make_shared<UGameWindow>();
		if (!UGameWindow::CreateGameWindow(inGameName, inWindowWidth, inWindowHeight, *mGameWindow))
		{
			return false;
		}

		// Init renderer
		mRenderer = std::make_shared<URenderer>();
		if (!mRenderer->Init())
		{
			return false;
		}

		// Start the FrameTimer
		mFrameTimer = std::make_shared<UFrameTimer>();
		mFrameTimer->Start();

		// Set FPS cap to 120 for now. Will make this configurable later
		// Servers will probably be set to 60 FPS cap
		SetFPSCap(120.f);

		// Create the GameInstance
		mGameInstance = std::make_shared<UGameInstance>();
		mGameInstance->OnStartup();

		LOG(LogGameEngine, Log, "Engine initialization successful\n");
		return true;
	}

	void UGameEngine::Run()
	{
		while (bContinue)
		{
			Tick();
		}

		mGameWindow->CaptureCursor(false);
	}

	void UGameEngine::Stop()
	{
		LOG(LogGameEngine, Log, "Engine stopping...\n");
		bContinue = false;
	}

	UGameEngine::~UGameEngine()
	{
		mGameInstance->OnShutdown();
		mGameInstance = nullptr;

		mRenderer->Shutdown();
		mRenderer = nullptr;

		mGameWindow = nullptr;

		LOG(LogGameEngine, Log, "Engine shutdown complete\n");
		ULog::Get().Shutdown();
	}

	void UGameEngine::Tick()
	{
		mDeltaTime = mFrameTimer->GetFrameTime(mFrameStep);
		mDeltaTime = min(mDeltaTime, 0.25f); // Min FPS is 4

		// Tick native message queue
		UGameWindow::PumpMessageQueue();

		// Tick input
		UGameInput::Get().Tick();

		// Tick each world
		//for (auto& world : m_worlds)
		//{
		//	world->Update(mDeltaTime);
		//}

		// Tick renderer
		mRenderer->Frame();
	}
}
