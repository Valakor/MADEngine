#pragma once

#include "Core/Component.h"
#include "Rendering/Mesh.h"

namespace MAD
{
	class CReflectionProbeComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CReflectionProbeComponent, UComponent)
	public:
		explicit CReflectionProbeComponent(OGameWorld* inOwningWorld);

		virtual void PostInitializeComponents() override;
		virtual void Load(const UGameWorldLoader& inLoader, const class UObjectValue& inPropertyObj) override;
	private:
		SMeshInstance m_probeMesh;
	};
}
