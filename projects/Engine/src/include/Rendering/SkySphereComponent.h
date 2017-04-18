#pragma once

#include "Core/Component.h"
#include "Rendering/Mesh.h"

namespace MAD
{
	class CSkySphereComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CSkySphereComponent, UComponent)
	public:
		explicit CSkySphereComponent(OGameWorld* inOwningWorld);

		virtual void PostInitializeComponents() override;
		virtual void Load(const class UGameWorldLoader& inLoader, const class UObjectValue& inPropertyObj) override;
	private:
		SMeshInstance m_skySphereMesh;
	};
}
