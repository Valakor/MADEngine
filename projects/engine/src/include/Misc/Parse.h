#pragma once

#include <EASTL/string.h>

namespace MAD
{
	struct SParse
	{
		static bool Get(const eastl::string& inStr, const eastl::string& inMatch, int& outVal);
		static bool Get(const eastl::string& inStr, const eastl::string& inMatch, float& outVal);
		static bool Get(const eastl::string& inStr, const eastl::string& inMatch, eastl::string& outVal);

		static bool Find(const eastl::string& inStr, const eastl::string& inMatch);

	private:
		static bool FindMatch(const eastl::string& inStr, const eastl::string& inMatch, size_t& outPos);
	};
}
