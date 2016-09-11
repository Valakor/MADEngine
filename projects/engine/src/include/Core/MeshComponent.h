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
		
		virtual void UpdateComponent(float inDeltaTime) override;
	
		// TODO: Setup transform hierarchy for entities and components
		void TEMPInitializeMeshInstance(const eastl::string& inMeshFileName, const DirectX::SimpleMath::Matrix& inDirectTransform, bool inIsVisibleInitial = true);
	private:
		void ConstructDrawItem();
	private:
		SMeshInstance m_meshInstance;
	};
}
