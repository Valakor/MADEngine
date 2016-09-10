#include "Rendering/DrawItem.h"
#include "Rendering/GraphicsDriver.h"

namespace MAD
{

	SDrawItem::SDrawItem() : m_drawItemPriority(0) {}

	void SDrawItem::ProcessDrawItem(UGraphicsDriver& inGraphicsDriver) const
	{
		(void)inGraphicsDriver;
	}

	bool SDrawItem::operator<(const SDrawItem& inOtherDrawItem) const
	{
		return m_drawItemPriority < inOtherDrawItem.m_drawItemPriority;
	}

}