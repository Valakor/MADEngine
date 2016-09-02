#pragma once

namespace MAD
{
#ifdef _DEBUG
	extern bool AssertFunc(bool expr, const char* expr_str, const char* desc, int line, const char* file_name);

#define MAD_ASSERT_DESC(expr, desc) { if (AssertFunc((expr), #expr, desc, __LINE__, __FILE__)) { __debugbreak(); } }
#else
#define MAD_ASSERT_DESC(expr, desc)
#endif
}
