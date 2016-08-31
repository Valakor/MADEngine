#include "Misc/Assert.h"

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdlib>
#include <wchar.h>

namespace MAD
{
	extern bool AssertFunc(bool expr, const wchar_t* expr_str, const wchar_t* desc, int line, const wchar_t* file_name)
	{
		bool bShouldHalt = !expr;
		if (bShouldHalt)
		{
			static wchar_t szBuffer[1024];
			_snwprintf_s(szBuffer, 1024, _TRUNCATE,
						 L"Assertion Failed!\n\nDescription: %s\nExpression: %s\nFile: %s\nLine: %d\n\nPress Retry to debug.",
						 desc, expr_str, file_name, line);
			int msgBox = MessageBoxW(nullptr, szBuffer, L"Assert", MB_ICONERROR | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2);
			switch (msgBox)
			{
			case IDABORT:
				exit(1);
			case IDRETRY:
				break;
			case IDIGNORE:
				bShouldHalt = false;
				break;
			default:
				break;
			}
		}

		return bShouldHalt;
	}
}
