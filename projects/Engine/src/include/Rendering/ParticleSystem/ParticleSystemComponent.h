#pragma once

#include "Core/Component.h"

namespace MAD
{
	class CParticleSystemComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CParticleSystemComponent, UComponent)
	public:
		explicit CParticleSystemComponent(OGameWorld* inOwningWorld);

	private:
		class UParticleSystem* m_particleSystem;
	};
}