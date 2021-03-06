#pragma once

#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogError);

	// Used to generate compiler warnings
	// See #pragma message in GameEngine.cpp
	// Example:
	//     #pragma message(__LOC__"This version of SParse::Get<T> is not yet implemented")
	#define __STR2__(x) #x
	#define __STR1__(x) __STR2__(x)
	#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Warning MADEngine: "

#ifdef _DEBUG
	// Use the UNREACHABLE macro
	__declspec(noreturn)
	inline void UnreachableInternal(const char* inMsg, const char* inFile, unsigned inLine)
	{
		ULog::Get().LogF(LogError, ELogVerbosity::Error, inFile, inLine, inMsg);
		__debugbreak();
		abort();
	}

	#define UNREACHABLE(msg) UnreachableInternal(msg, __FILENAME__, __LINE__)
#else
	#define UNREACHABLE(msg) __assume(false)
#endif
}
