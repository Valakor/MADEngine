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
		: Super_t(inOwningWorld)
	{
		m_meshInstance.m_bVisible = false;
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

	bool CMeshComponent::LoadFrom(const eastl::string& inAssetName)
	{
		m_meshInstance.m_mesh = UMesh::Load(inAssetName);
		return m_meshInstance.m_mesh != nullptr;
	}

	void CMeshComponent::ConstructDrawItem() const
	{
		URenderer& targetRenderer = gEngine->GetRenderer();
		eastl::vector<SDrawItem> constructedDrawItems;

		m_meshInstance.m_mesh->BuildDrawItems(constructedDrawItems, GetWorldTransform());

		// Set the draw item properties
		for (size_t i = 0; i < constructedDrawItems.size(); ++i)
		{
			auto& currentDrawItem = constructedDrawItems[i];

			currentDrawItem.m_uniqueID = MakeDrawItemID(GetObjectID(), i);
			targetRenderer.QueueDrawItem(currentDrawItem);
		}
	}

	size_t CMeshComponent::MakeDrawItemID(size_t inMeshCompID, size_t inDrawItemIdx)
	{
		size_t a = eastl::hash<size_t>{}(inMeshCompID);
		size_t b = eastl::hash<size_t>{}(inDrawItemIdx);

		// Boost reference implementation of hash_combine
		a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);

		return a;
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
