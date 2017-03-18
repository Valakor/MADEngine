#pragma once

#include "Core/Component.h"
#include "Rendering/ParticleSystem/ParticleSystemManager.h"

namespace MAD
{
	class CParticleSystemComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CParticleSystemComponent, UComponent)
	public:
		explicit CParticleSystemComponent(OGameWorld* inOwningWorld);

		virtual void PostInitializeComponents() override;
		virtual void Load(const UGameWorldLoader& inLoader) override;
		virtual void UpdateComponent(float inDeltaTime) override;
	private:
		Vector3 GetViewSpacePosition() const;
	private:
		class UParticleSystem* m_particleSystem;
	};
}