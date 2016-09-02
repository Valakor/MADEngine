#include "Core/GameWorld.h"

#include "Core/GameEngine.h"
#include "Core/Entity.h"

namespace MAD
{
	void UGameWorld::UpdatePrePhysics(float inDeltaTime)
	{
		m_componentUpdater.UpdatePrePhysicsComponents(inDeltaTime);
	}

	void UGameWorld::UpdatePostPhysics(float inDeltaTime)
	{
		m_componentUpdater.UpdatePostPhysicsComponents(inDeltaTime);
	}

}