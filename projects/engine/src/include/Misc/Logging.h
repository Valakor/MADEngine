#pragma once

#include "Misc/Assert.h"

#include <fstream>
#include <EASTL/internal/config.h>

namespace MAD
{
#ifndef MAD_ENABLE_LOGGING
#define MAD_ENABLE_LOGGING 0
#endif

#if _DEBUG | DEBUG | MAD_ENABLE_LOGGING
#define MAD_DO_LOGGING 1
#endif

	enum class ELogVerbosity
	{
		Log,
		Warning,
		Error
	};

	struct SLogCategory
	{
		SLogCategory(const char* inName);

	private:
		friend class ULog;

		const char* mCategoryName;
	};

	class ULog
	{
	public:
		static ULog& Get()
		{
			static ULog sInstance;
			return sInstance;
		}

		void Init();
		void Shutdown();

		// Logs formatted text to available consoles. Returns true if the LOG should cause a debug break.
		bool LogF(const SLogCategory& inCategory, ELogVerbosity inVerbosity, const char* inFilename, int inLine, const char* inFormat, ...);

	private:
		ULog();

		bool bWasInitialized;

		bool bHasConsole;
		void* mConsoleOut;
		void* mConsoleErr;

		bool bHasLogFile;
		std::wofstream mLogFile;
	};

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#if MAD_DO_LOGGING
#define LOG(Category, Verbosity, Format, ...)																	\
	do																											\
	{																											\
		if (ULog::Get().LogF(Category, ELogVerbosity::Verbosity, __FILENAME__, __LINE__, Format, __VA_ARGS__))	\
		{																										\
			EASTL_DEBUG_BREAK();																				\
		}																										\
	} while (0)
#else
#define LOG(...) (void)0
#endif

#define DECLARE_LOG_CATEGORY(LogCategoryName)									\
	static struct SLogCategory##LogCategoryName : public MAD::SLogCategory		\
	{																			\
		SLogCategory##LogCategoryName() : SLogCategory(#LogCategoryName) { }	\
	} LogCategoryName

	DECLARE_LOG_CATEGORY(LogDefault);
}
