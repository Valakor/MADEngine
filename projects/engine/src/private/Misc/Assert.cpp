#include "Misc/Assert.h"

#include <cstdlib>
#include <wchar.h>

#include "Misc/ErrorHandling.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"
#include <cassert>

namespace MAD
{
	extern MessageBoxResult MAD_MessageBox(const char* inTitle, const char* inText, MessageBoxType inType)
	{
		UINT flags = 0;

		switch (inType)
		{
		case MessageBoxType::Default: flags |= MB_OKCANCEL         | MB_ICONQUESTION    | MB_DEFBUTTON2; break;
		case MessageBoxType::Ok:      flags |= MB_OK               | MB_ICONINFORMATION | MB_DEFBUTTON1; break;
		case MessageBoxType::Error:   flags |= MB_ABORTRETRYIGNORE | MB_ICONERROR       | MB_DEFBUTTON2; break;
		case MessageBoxType::YesNo:   flags |= MB_YESNO            | MB_ICONQUESTION    | MB_DEFBUTTON2; break;
		default: assert(false);
		}

		auto result = MessageBoxW(nullptr, utf8util::UTF16FromUTF8(inText).c_str(), utf8util::UTF16FromUTF8(inTitle).c_str(), flags);
		return static_cast<MessageBoxResult>(result);
	}

	extern bool AssertFunc(bool inExpr, const char* inExprStr, const char* inDesc, int inLine, const char* inFileName)
	{
		bool bShouldHalt = !inExpr;
		if (bShouldHalt)
		{
			static char szBuffer[1024];

			auto short_file_name = strrchr(inFileName, '\\');
			if (short_file_name) short_file_name = short_file_name + 1;
			else short_file_name = inFileName;

			_snprintf_s(szBuffer, 1024, _TRUNCATE,
							"Assertion Failed!\n\nDescription: %s\n\nExpression: %s\nFile: %s\nLine: %d\n",
							inDesc, inExprStr, inFileName, inLine);

			return ULog::Get().LogF(LogError, ELogVerbosity::Error, short_file_name, inLine, szBuffer);
		}

		return bShouldHalt;
	}
}
