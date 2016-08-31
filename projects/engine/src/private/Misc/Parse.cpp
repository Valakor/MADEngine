#include "Misc/Parse.h"

using eastl::string;

namespace MAD
{
	bool SParse::FindMatch(const string& inStr, const string& inMatch, size_t& outPos)
	{
		auto pos = inStr.find(inMatch);
		if (inMatch.size() == 0 || pos == string::npos)
		{
			return false;
		}

		outPos = pos + inMatch.size();
		return true;
	}

	bool SParse::Find(const string& inStr, const string& inMatch)
	{
		size_t pos;
		return SParse::FindMatch(inStr, inMatch, pos);
	}

	bool SParse::Get(const string& inStr, const string& inMatch, int& outVal)
	{
		size_t pos;
		if (!SParse::FindMatch(inStr, inMatch, pos))
		{
			return false;
		}

		outVal = atoi(&inStr[pos]);
		return true;
	}

	bool SParse::Get(const string& inStr, const string& inMatch, float& outVal)
	{
		size_t pos;
		if (!SParse::FindMatch(inStr, inMatch, pos))
		{
			return false;
		}

		outVal = static_cast<float>(atof(&inStr[pos]));
		return true;
	}

	bool SParse::Get(const string& inStr, const string& inMatch, string& outVal)
	{
		size_t pos;
		if (!SParse::FindMatch(inStr, inMatch, pos))
		{
			return false;
		}

		auto endPos = inStr.find_first_of(" \t", pos);
		outVal = inStr.substr(pos, endPos - pos);
		return true;
	}

}
