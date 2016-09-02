#include "Core/ComponentUpdater.h"

namespace MAD
{
	void ComponentUpdater::UpdatePrePhysicsComponents(float inDeltaTime)
	{
		(void)inDeltaTime;
		// Iterate over the entries of the component containers while the priority level is lower than the physics priority level
	}

	void ComponentUpdater::UpdatePostPhysicsComponents(float inDeltaTime)
	{
		(void)inDeltaTime;
		// Iterate over the entries of the component containers from the physics priority level end of the container
	}

}