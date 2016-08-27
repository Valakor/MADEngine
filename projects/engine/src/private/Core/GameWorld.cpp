#include "Core\GameEngine.h"
#include "Core\GameWorld.h"
#include "Core\Entity.h"

namespace MAD
{
	MAD_IMPLEMENT_CLASS(UGameWorld, UObject)

	void UGameWorld::Update(float inDeltaTime)
	{
		m_componentUpdater.UpdateTickGroup<TickType::TT_PrePhysicsTick>(inDeltaTime);

		// Update physics begin -------------------

		// ...Stuff

		// Update physics end ---------------------

		m_componentUpdater.UpdateTickGroup<TickType::TT_PostPhysicsTick>(inDeltaTime);
	}

}