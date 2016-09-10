#pragma once

#include <cstdint>

namespace MAD
{
	struct SDrawItem
	{
		SDrawItem();

		void ProcessDrawItem(class UGraphicsDriver& inGraphicsDriver) const;

		uint32_t m_drawItemPriority; // TODO: Potentially use a more robust draw item priority system in the future (?)
		bool operator<(const SDrawItem& inOtherDrawItem) const;
	};
}
