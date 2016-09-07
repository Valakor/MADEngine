#pragma once

#include <cstdint>

namespace MAD
{
	using PriorityLevel = uint32_t;

	enum EPriorityLevelReference
	{
		EPriorityLevel_Default = 500,
		EPriorityLevel_Physics = 1000
	};

	class TComponentPriorityInfo
	{
	public:
		explicit TComponentPriorityInfo(PriorityLevel inInitialPriorityLevel = EPriorityLevelReference::EPriorityLevel_Default) : m_priorityLevel(inInitialPriorityLevel) {}

		inline void UpdatePriorityLevel(PriorityLevel inNewPriorityLevel) { m_priorityLevel = inNewPriorityLevel; }

		inline PriorityLevel GetPriorityLevel() const { return m_priorityLevel; }
	private:
		PriorityLevel m_priorityLevel;
	};
}
