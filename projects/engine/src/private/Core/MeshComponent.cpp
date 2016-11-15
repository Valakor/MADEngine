#include "Core/MeshComponent.h"
#include "Core/Entity.h"
#include "Core/GameEngine.h"
#include "Core/GameWorldLoader.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/DrawItem.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogTransformComponent);

	CMeshComponent::CMeshComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
	{
		m_meshInstance.m_bVisible = false;
		m_meshInstance.m_perDrawConstants.m_objectToWorldMatrix = Matrix::Identity;
	}

	void CMeshComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;

		//LOG(LogTransformComponent, Log, "Updating Mesh Component for %s #%d\n", GetOwner().GetTypeInfo()->GetTypeName(), GetOwner().GetObjectID());
		
		// Have the owning SMeshInstance create a draw item to submit to renderer
		if (m_meshInstance.m_bVisible && m_meshInstance.m_mesh)
		{
			// Only create the draw item if our mesh instance is initialized properly with a mesh and direct transform
			ConstructDrawItem();
		}
	}

	void CMeshComponent::ConstructDrawItem()
	{
		URenderer& targetRenderer = gEngine->GetRenderer();
		eastl::vector<SDrawItem> constructedDrawItems;

		memset(&constructedDrawItems, 0, sizeof(constructedDrawItems));

		m_meshInstance.m_perDrawConstants.m_objectToWorldMatrix = GetWorldTransform().GetMatrix();
		m_meshInstance.m_mesh->BuildDrawItems(constructedDrawItems, m_meshInstance.m_perDrawConstants);

		// Set the draw item properties
		for (const auto& currentDrawItem : constructedDrawItems)
		{
			targetRenderer.QueueDrawItem(currentDrawItem);
		}
	}

	void CMeshComponent::Load(const UGameWorldLoader& inLoader)
	{
		inLoader.GetBool("visible", m_meshInstance.m_bVisible);

		eastl::string meshName;
		if (inLoader.GetString("mesh", meshName))
		{
			m_meshInstance.m_mesh = UMesh::Load(meshName);
		}
	}
}
