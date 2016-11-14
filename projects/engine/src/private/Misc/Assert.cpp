#include "Misc/Assert.h"

#include <cstdlib>
#include <wchar.h>

#include "Misc/ErrorHandling.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"
#include <cassert>

namespace MAD
{
	extern bool AssertFunc(bool inExpr, const char* inExprStr, const char* inDesc, int inLine, const char* inFileName)
	{
		bool bShouldHalt = !inExpr;
		if (bShouldHalt)
		{
			static char szBuffer[1024];

			{
				auto short_file_name = strrchr(inFileName, '\\');
				if (short_file_name) short_file_name = short_file_name + 1;
				else short_file_name = inFileName;

				_snprintf_s(szBuffer, 1024, _TRUNCATE,
							 "Assertion Failed!\n\n\tDescription: %s\n\n\tExpression: %s\n\tFile: %s\n\tLine: %d\n",
							 inDesc, inExprStr, inFileName, inLine);
				ULog::Get().LogF(LogError, ELogVerbosity::Error, short_file_name, inLine, szBuffer);
			}

			assert(false);

			_snprintf_s(szBuffer, 1024, _TRUNCATE,
						 "Assertion Failed!\n\nDescription: %s\n\nExpression: %s\nFile: %s\nLine: %d\n\nPress Retry to debug.",
						 inDesc, inExprStr, inFileName, inLine);
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
