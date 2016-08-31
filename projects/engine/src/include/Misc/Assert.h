#pragma once

namespace MAD
{
#ifdef _DEBUG
	extern bool AssertFunc(bool expr, const wchar_t* expr_str, const wchar_t* desc, int line, const wchar_t* file_name);

	// These macros convert __FILE__ from char* to wchar_t*
#define MAD_WIDEN2(x) L##x
#define MAD_WIDEN(x) MAD_WIDEN2(x)
#define __WFILE__ MAD_WIDEN(__FILE__)

#define MAD_ASSERT_DESC(expr, desc) { if (AssertFunc((expr), L#expr, L##desc, __LINE__, __WFILE__)) { __debugbreak(); } }
#else
	#define MAD_ASSERT_DESC(expr, desc)
#endif
}
