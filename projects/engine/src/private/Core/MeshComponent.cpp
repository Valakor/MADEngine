#include "Core/MeshComponent.h"
#include "Core/Entity.h"
#include "Core/GameEngine.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/DrawItem.h"
#include "Misc/AssetCache.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogTransformComponent);

	CMeshComponent::CMeshComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld) {}

	void CMeshComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;

		LOG(LogTransformComponent, Log, "Updating Mesh Component for %s #%d\n", GetOwner().GetTypeInfo()->GetTypeName(), GetOwner().GetObjectID());
		
		// Have the owning SMeshInstance create a draw item to submit to renderer
		if (m_meshInstance.m_bVisible && m_meshInstance.m_mesh)
		{
			ConstructDrawItem();
		}
	}

	void CMeshComponent::TEMPInitializeMeshInstance(const eastl::string& inMeshFileName, const DirectX::SimpleMath::Matrix& inDirectTransform, bool inIsVisibleInitial)
	{
		m_meshInstance.m_bVisible = inIsVisibleInitial;
		m_meshInstance.m_perDrawConstants.m_objectToWorldMatrix = inDirectTransform;
		m_meshInstance.m_mesh = UAssetCache::Load<UMesh>(inMeshFileName);
	}

	void CMeshComponent::ConstructDrawItem()
	{
		URenderer& targetRenderer = gEngine->GetRenderer();
		eastl::vector<SDrawItem> constructedDrawItems;

		memset(&constructedDrawItems, 0, sizeof(constructedDrawItems));

		m_meshInstance.m_mesh->BuildDrawItems(constructedDrawItems, m_meshInstance.m_perDrawConstants);

		// Set the draw item properties
		for (const auto& currentDrawItem : constructedDrawItems)
		{
			targetRenderer.QueueDrawItem(currentDrawItem);
		}
	}

}
