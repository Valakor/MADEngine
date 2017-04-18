#include "Rendering/ReflectionProbeComponent.h"
#include "Core/BaseEngine.h"
#include "Core/Pipeline/GameWorldLoader.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	CReflectionProbeComponent::CReflectionProbeComponent(OGameWorld* inOwningWorld) : Super_t(inOwningWorld) {}

	void CReflectionProbeComponent::PostInitializeComponents()
	{
		// TODO For now, our reflection probes won't be able to move through the editor
		auto& renderer = gEngine->GetRenderer();

		eastl::vector<SDrawItem> reflectionProbeDrawItems;

		memset(&reflectionProbeDrawItems, 0, sizeof(reflectionProbeDrawItems));

		m_probeMesh.m_mesh->BuildDrawItems(reflectionProbeDrawItems, GetWorldTransform());

		for (size_t i = 0; i < reflectionProbeDrawItems.size(); ++i)
		{
			// Queue up a reflection probe draw item
			renderer.QueueReflectionProbeItem(reflectionProbeDrawItems[i]);
		}
	}

	void CReflectionProbeComponent::Load(const UGameWorldLoader&, const UObjectValue& inPropertyObj)
	{
		eastl::string meshAssetPath;
		if (inPropertyObj.GetProperty("mesh", meshAssetPath))
		{
			m_probeMesh.m_mesh = UMesh::Load(meshAssetPath);
		}
	}

}
