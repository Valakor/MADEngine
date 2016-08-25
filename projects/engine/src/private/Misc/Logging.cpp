#include "Misc/Logging.h"

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdarg>

#include "Engine.h"
#include "Misc/utf8conv.h"

#define LOG_FILENAME "log.txt"

namespace MAD
{
	SLogCategory::SLogCategory(string inName) :
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
#ifdef _DEBUG
		if (SParse::Find(SCmdLine::Get(), "-Log"))
		{
			AllocConsole();
			mConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
			mConsoleErr = GetStdHandle(STD_ERROR_HANDLE);
			bHasConsole = true;
		}

		string logFilename = LOG_FILENAME;
		SParse::Get(SCmdLine::Get(), "-LogFile=", logFilename);

		mLogFile = std::ofstream(utf8util::UTF16FromUTF8(logFilename), std::ios_base::trunc);
		bHasLogFile = mLogFile.is_open() && mLogFile.good();
#endif

		bWasInitialized = true;
	}

	void ULog::Shutdown()
	{
		assert(bWasInitialized && "Should not call Shutdown() if Init() has not been called.");

		FreeConsole();
		bHasConsole = false;
		mConsoleOut = nullptr;
		mConsoleErr = nullptr;
		bHasLogFile = false;
		mLogFile.close();

		bWasInitialized = false;
	}

	void ULog::LogF(const SLogCategory& inCategory, ELogVerbosity inVerbosity, const char* inFilename, int inLine, const char* inFormat, ...)
	{
		static char outBuf[2048];
		HANDLE handle = nullptr;

		bool bDebuggerPresent = IsDebuggerPresent() != 0;
		if (!bDebuggerPresent && !bHasConsole && !bHasLogFile)
		{
			return;
		}

		int startPos = sprintf_s(outBuf, "[%s:%4i] ", inFilename, inLine);
		assert(startPos > 0 && "Failed to write filename + line number to buffer.");
		assert(startPos < _countof(outBuf) && "Filename + line number too long for buffer.");

		startPos += sprintf_s(outBuf + startPos, _countof(outBuf) - startPos, "[%s] ", inCategory.mCategoryName.c_str());
		assert(startPos > 0 && "Failed to write category name to buffer.");
		assert(startPos < _countof(outBuf) && "Filename + line number + category name too long for buffer.");

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
			assert(false && "Non-supported ELogVerbosity.");
			break;
		}
		assert(startPos > 0 && "Failed to write log verbosity to buffer.");
		assert(startPos < 2048 && "Filename + line number + category name + verbosity too long for buffer.");

		va_list args;
		va_start(args, inFormat);
		vsprintf_s(outBuf + startPos, _countof(outBuf) - startPos, inFormat, args);
		va_end(args);

		if (bDebuggerPresent || bHasConsole)
		{
			std::wstring outWide = utf8util::UTF16FromUTF8(outBuf);

			if (bDebuggerPresent)
			{
				OutputDebugStringW(outWide.c_str());
			}

			if (bHasConsole)
			{
				DWORD numWritten;
				WriteConsoleW(handle, outWide.c_str(), static_cast<DWORD>(outWide.size()), &numWritten, nullptr);
			}
		}

		if (bHasLogFile)
		{
			mLogFile << outBuf;
			mLogFile.flush();
		}
	}
}
