#include "Core/Component.h"

namespace MAD
{
	UComponent::UComponent(AEntity& inCompOwner, TickType inCompTickType /*= TickType::TT_PrePhysicsTick*/) : m_tickType(inCompTickType), m_owner(&inCompOwner) { }
}
