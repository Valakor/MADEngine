#pragma once

#include <EASTL/string.h>

#include "Core/Component.h"
#include "Rendering/Mesh.h"

namespace MAD
{
	class CMeshComponent : public UComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CMeshComponent, UComponent, EPriorityLevelReference::EPriorityLevel_Physics + 1)
	public:
		explicit CMeshComponent(OGameWorld* inOwningWorld);
		
		virtual void Load(const UGameWorldLoader& inLoader) override;
		virtual void UpdateComponent(float inDeltaTime) override;
	
	private:
		void ConstructDrawItem();
	private:
		SMeshInstance m_meshInstance;
	};
}
