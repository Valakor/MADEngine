#include "Core/Component.h"

namespace MAD
{
	UComponent::UComponent(AEntity& inCompOwner) : m_owner(inCompOwner) { }
}
