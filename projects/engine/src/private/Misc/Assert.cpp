#include "Misc/Assert.h"

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdlib>
#include <wchar.h>

#include "Misc/ErrorHandling.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"

namespace MAD
{
	extern bool AssertFunc(bool expr, const char* expr_str, const char* desc, int line, const char* file_name)
	{
		bool bShouldHalt = !expr;
		if (bShouldHalt)
		{
			static char szBuffer[1024];

			{
				auto short_file_name = strrchr(file_name, '\\');
				if (short_file_name) short_file_name = short_file_name + 1;
				else short_file_name = file_name;

				_snprintf_s(szBuffer, 1024, _TRUNCATE,
							 "Assertion Failed!\n\n\tDescription: %s\n\n\tExpression: %s\n\tFile: %s\n\tLine: %d\n",
							 desc, expr_str, file_name, line);
				ULog::Get().LogF(LogError, ELogVerbosity::Error, short_file_name, line, szBuffer);
			}

			_snprintf_s(szBuffer, 1024, _TRUNCATE,
						 "Assertion Failed!\n\nDescription: %s\n\nExpression: %s\nFile: %s\nLine: %d\n\nPress Retry to debug.",
						 desc, expr_str, file_name, line);
			int msgBox = MessageBoxW(nullptr, utf8util::UTF16FromUTF8(szBuffer).c_str(), L"Assert", MB_ICONERROR | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2);
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
