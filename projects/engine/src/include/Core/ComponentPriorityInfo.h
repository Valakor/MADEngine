#pragma once

#include <cstdint>

namespace MAD
{
	using PriorityLevel_t = uint32_t;

	enum EPriorityLevelReference
	{
		EPriorityLevel_Default = 500,
		EPriorityLevel_Physics = 1000
	};

	class UComponentPriorityInfo
	{
	public:
		explicit UComponentPriorityInfo(PriorityLevel_t inInitialPriorityLevel = EPriorityLevelReference::EPriorityLevel_Default) : m_priorityLevel(inInitialPriorityLevel) {}

		inline void UpdatePriorityLevel(PriorityLevel_t inNewPriorityLevel) { m_priorityLevel = inNewPriorityLevel; }

		inline PriorityLevel_t GetPriorityLevel() const { return m_priorityLevel; }
	private:
		PriorityLevel_t m_priorityLevel;
	};
}
