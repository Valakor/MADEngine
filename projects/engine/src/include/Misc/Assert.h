#pragma once

#include <EASTL/internal/config.h>

namespace MAD
{
#ifdef _DEBUG
	extern bool AssertFunc(bool inExpr, const char* inExprStr, const char* inDesc, int inLine, const char* inFileName);

#define MAD_ASSERT_DESC(expr, desc)															\
	do																						\
	{																						\
		if (AssertFunc((expr), #expr, desc, __LINE__, __FILE__)) { EASTL_DEBUG_BREAK(); }	\
	} while (0)
#else
#define MAD_ASSERT_DESC(expr, desc) (void)0
#endif
}
