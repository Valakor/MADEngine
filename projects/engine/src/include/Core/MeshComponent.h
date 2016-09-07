#pragma once

#include "Core/Component.h"

namespace MAD
{
	class UMeshComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(UMeshComponent, UComponent)
	public:
		virtual ~UMeshComponent() {}

		virtual void UpdateComponent(float inDeltaTime) override;
	};
}
