#pragma once

#include <EASTL/internal/config.h>

namespace MAD
{
	enum class MessageBoxType
	{
		Default, // [ OK ] [ Cancel ]
		Ok,      // [ OK ]
		Error,   // [ Abort ] [ Retry ] [ Ignore ]
		YesNo,   // [ Yes ] [ No ]
	};

	enum class MessageBoxResult
	{
		OK = 1,
		Cancel,
		Abort,
		Retry,
		Ignore,
		Yes,
		No,
	};

	extern MessageBoxResult MAD_MessageBox(const char* inTitle, const char* inText, MessageBoxType inType);

#ifdef _DEBUG
	extern bool AssertFunc(bool inExpr, const char* inExprStr, const char* inDesc, int inLine, const char* inFileName);

#define MAD_ASSERT_DESC(expr, desc)															\
	do																						\
	{																						\
		if (AssertFunc((expr), #expr, desc, __LINE__, __FILE__)) { EASTL_DEBUG_BREAK(); }	\
	} while (0)

#define MAD_CHECK_DESC(expr, desc) MAD_ASSERT_DESC(expr, desc)

#else
#define MAD_ASSERT_DESC(expr, desc) (void)0
#define MAD_CHECK_DESC(expr, desc) expr
#endif
}
