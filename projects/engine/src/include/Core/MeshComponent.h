#pragma once

#include "Core/Component.h"

namespace MAD
{
	class CMeshComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CMeshComponent, UComponent)
	public:
		explicit CMeshComponent(OGameWorld* inOwningWorld);
		
		virtual void UpdateComponent(float inDeltaTime) override;
	};
}
