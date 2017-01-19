#include "Core/DebugTransformComponent.h"
#include "Core/GameWorldLoader.h"
#include "Core/GameEngine.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	CDebugTransformComponent::CDebugTransformComponent(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
		, m_debugScale(1.0f)
	{
	}

	void CDebugTransformComponent::Load(const UGameWorldLoader& inLoader)
	{
		inLoader.GetFloat("debug_scale", m_debugScale);
	}

	void CDebugTransformComponent::UpdateComponent(float)
	{
		// Draw a transform equal to the owning entity's world transform
		gEngine->GetRenderer().DrawDebugTransform(GetOwningEntity().GetWorldTransform().GetMatrix(), 1.0, m_debugScale);
	}
}