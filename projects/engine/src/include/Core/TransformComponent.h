#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"

namespace MAD
{
	class CTransformComponent : public UComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CTransformComponent, UComponent, 0)
	public:
		explicit CTransformComponent(OGameWorld* inOwningWorld);

		virtual void UpdateComponent(float inDeltaTime) override;
	private:
		uint32_t m_updateCount = 0;
	};
}
