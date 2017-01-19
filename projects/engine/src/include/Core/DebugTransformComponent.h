#pragma once

#include "Core/Component.h"

namespace MAD
{
	class CDebugTransformComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CDebugTransformComponent, UComponent)
	public:
		explicit CDebugTransformComponent(OGameWorld* inOwningWorld);

		virtual void Load(const UGameWorldLoader& inLoader) override;
		virtual void UpdateComponent(float inDeltaTime) override;
	private:
		float m_debugScale;
	};
}