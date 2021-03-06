#include "Misc/Logging.h"

#include <EASTL/string.h>

#include "Core/GameEngine.h"
#include "Misc/Assert.h"
#include "Misc/Parse.h"
#include "Misc/utf8conv.h"

using eastl::string;

#define LOG_FILENAME "log.txt"

namespace MAD
{
	SLogCategory::SLogCategory(const char* inName) :
		mCategoryName(inName)
	{ }


	ULog::ULog() :
		bWasInitialized(false),
		bHasConsole(false),
		mConsoleOut(nullptr),
		mConsoleErr(nullptr),
		bHasLogFile(false)
	{ }

	void ULog::Init()
	{
#if MAD_DO_LOGGING
		if (SParse::Find(SCmdLine::Get(), "-Log"))
		{
			AllocConsole();
			mConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
			mConsoleErr = GetStdHandle(STD_ERROR_HANDLE);
			bHasConsole = true;
		}

		string logFilename = LOG_FILENAME;
		SParse::Get(SCmdLine::Get(), "-LogFile=", logFilename);

		mLogFile = std::wofstream(utf8util::UTF16FromUTF8(logFilename).c_str(), std::ios_base::trunc);
		bHasLogFile = mLogFile.is_open() && mLogFile.good();
#endif

		bWasInitialized = true;
	}

	void ULog::Shutdown()
	{
		MAD_ASSERT_DESC(bWasInitialized, "Should not call Shutdown() if Init() has not been called.");

		FreeConsole();
		bHasConsole = false;
		mConsoleOut = nullptr;
		mConsoleErr = nullptr;
		bHasLogFile = false;
		mLogFile.close();

		bWasInitialized = false;
	}

	bool ULog::LogF(const SLogCategory& inCategory, ELogVerbosity inVerbosity, const char* inFilename, int inLine, const char* inFormat, ...)
	{
#if !MAD_DO_LOGGING
		(void)inCategory; (void)inFilename; (void)inLine; (void)inFormat;

		if (inVerbosity != ELogVerbosity::Error)
		{
			return false;
		}

		auto result = MAD_MessageBox("Error", "Logging not enabled", MessageBoxType::Error);
		switch (result)
		{
		case MessageBoxResult::Abort:
			exit(1);
		case MessageBoxResult::Retry:
			break;
		case MessageBoxResult::Ignore:
			return false;
		default:
			break;
		}

		return true;
#else
		static char outBuf[2048];
		HANDLE handle = nullptr;

		int startPos = sprintf_s(outBuf, "[%s:%4i] ", inFilename, inLine);
		MAD_ASSERT_DESC(startPos > 0, "Failed to write filename + line number to buffer.");
		MAD_ASSERT_DESC(startPos < _countof(outBuf), "Filename + line number too long for buffer.");

		startPos += sprintf_s(outBuf + startPos, _countof(outBuf) - startPos, "[%s] ", inCategory.mCategoryName);
		MAD_ASSERT_DESC(startPos > 0, "Failed to write category name to buffer.");
		MAD_ASSERT_DESC(startPos < _countof(outBuf), "Filename + line number + category name too long for buffer.");

		switch (inVerbosity)
		{
		case ELogVerbosity::Log:
			startPos += sprintf_s(outBuf + startPos, _countof(outBuf) - startPos, "[Log] ");
			SetConsoleTextAttribute(mConsoleOut, 15); // Normal white text
			handle = mConsoleOut;
			break;
		case ELogVerbosity::Warning:
			startPos += sprintf_s(outBuf + startPos, _countof(outBuf) - startPos, "[Warning] ");
			SetConsoleTextAttribute(mConsoleErr, 14); // Normal yellow text
			handle = mConsoleErr;
			break;
		case ELogVerbosity::Error:
			startPos += sprintf_s(outBuf + startPos, _countof(outBuf) - startPos, "[Error] ");
			SetConsoleTextAttribute(mConsoleErr, FOREGROUND_RED | FOREGROUND_INTENSITY); // Intense red text
			handle = mConsoleErr;
			break;
		default:
			MAD_ASSERT_DESC(false, "Non-supported ELogVerbosity.");
			break;
		}
		MAD_ASSERT_DESC(startPos > 0, "Failed to write log verbosity to buffer.");
		MAD_ASSERT_DESC(startPos < 2048, "Filename + line number + category name + verbosity too long for buffer.");

		va_list args;
		va_start(args, inFormat);
		vsprintf_s(outBuf + startPos, _countof(outBuf) - startPos, inFormat, args);
		va_end(args);

		bool bDebuggerPresent = IsDebuggerPresent() != 0;

		if (bDebuggerPresent || bHasConsole || bHasLogFile)
		{
			auto outWide = utf8util::UTF16FromUTF8(outBuf);

			if (bDebuggerPresent)
			{
				OutputDebugStringW(outWide.c_str());
			}

			if (bHasConsole)
			{
				DWORD numWritten;
				WriteConsoleW(handle, outWide.c_str(), static_cast<DWORD>(outWide.size()), &numWritten, nullptr);
			}

			if (bHasLogFile)
			{
				mLogFile << outWide.c_str();
				mLogFile.flush();
			}
		}

		if (inVerbosity != ELogVerbosity::Error)
		{
			return false;
		}

		auto result = MAD_MessageBox("Error", outBuf, MessageBoxType::Error);
		switch (result)
		{
		case MessageBoxResult::Abort:
			exit(1);
		case MessageBoxResult::Retry:
			break;
		case MessageBoxResult::Ignore:
			return false;
		default:
			break;
		}

		return true;
#endif
	}
}
