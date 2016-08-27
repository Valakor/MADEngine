#include "Core\Entity.h"

#include <iostream>

namespace MAD
{
	MAD_IMPLEMENT_ACTOR(AEntity, UObject)

	AEntity::AEntity(UGameWorld* owningWorld) : m_owningWorld(owningWorld) {}
}