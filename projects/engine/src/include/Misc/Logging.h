#pragma once

#include <fstream>
#include <string>

using std::string;

namespace MAD
{
	enum class ELogVerbosity
	{
		Log,
		Warning,
		Error
	};

	struct SLogCategory
	{
		SLogCategory(string inName);

	private:
		friend class ULog;

		string mCategoryName;
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

		void LogF(const SLogCategory& inCategory, ELogVerbosity inVerbosity, const char* inFilename, int inLine, const char* inFormat, ...);

	private:
		ULog();

		bool bWasInitialized;

		bool bHasConsole;
		void* mConsoleOut;
		void* mConsoleErr;

		bool bHasLogFile;
		std::ofstream mLogFile;
	};

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#ifdef _DEBUG
#define LOG(Category, Verbosity, ...) \
	ULog::Get().LogF(Category, ELogVerbosity::Verbosity, __FILENAME__, __LINE__, __VA_ARGS__); \
	__pragma(warning(push)) \
	__pragma(warning(disable:4127)) \
	if (ELogVerbosity::Verbosity == ELogVerbosity::Error) __debugbreak(); \
	__pragma(warning(pop))
#else
#define LOG(...)
#endif

#define DECLARE_LOG_CATEGORY(LogCategoryName) \
	static struct SLogCategory##LogCategoryName : public MAD::SLogCategory \
	{ \
		SLogCategory##LogCategoryName() : SLogCategory(#LogCategoryName) { } \
	} LogCategoryName;

	DECLARE_LOG_CATEGORY(LogDefault);
}
