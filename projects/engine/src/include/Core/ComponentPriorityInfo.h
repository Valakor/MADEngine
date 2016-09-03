#pragma once

#include <cstdint>

namespace MAD
{
	using PriorityLevel = uint32_t;

	class TComponentPriorityInfo
	{
	public:
		static const PriorityLevel s_defaultPriorityLevel;
	public:
		explicit TComponentPriorityInfo(PriorityLevel inInitialPriorityLevel = s_defaultPriorityLevel) : m_priorityLevel(inInitialPriorityLevel) {}

		inline void UpdatePriorityLevel(PriorityLevel inNewPriorityLevel) { m_priorityLevel = inNewPriorityLevel; }

		inline PriorityLevel GetPriorityLevel() const { return m_priorityLevel; }
	private:
		PriorityLevel m_priorityLevel;
	};
}