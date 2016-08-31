#include "Core/Component.h"

namespace MAD
{
	MAD_IMPLEMENT_CLASS(UComponent, UObject)

	UComponent::UComponent(AEntity& inCompOwner, TickType inCompTickType /*= TickType::TT_PrePhysicsTick*/) : m_tickType(inCompTickType), m_owner(&inCompOwner) { }
}
