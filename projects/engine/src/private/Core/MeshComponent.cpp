#include "Core/MeshComponent.h"
#include "Core/Entity.h"
#include "Core/GameEngine.h"
#include "Core/Pipeline/GameWorldLoader.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/DrawItem.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogTransformComponent);

	CMeshComponent::CMeshComponent(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
		, m_bIsDynamic(false)
	{
		m_meshInstance.m_bVisible = false;
	}

	void CMeshComponent::PostInitializeComponents()
	{
		if (!m_meshInstance.m_mesh || !m_meshInstance.m_bVisible)
		{
			return;
		}

		if (m_bIsDynamic)
		{
			return;
		}

		// If static object, queue up static draw item right now
		eastl::vector<SDrawItem> constructedDrawItems;

		m_meshInstance.m_mesh->BuildDrawItems(constructedDrawItems, GetWorldTransform());

		// Set the draw item properties
		for (size_t i = 0; i < constructedDrawItems.size(); ++i)
		{
			auto& currentDrawItem = constructedDrawItems[i];

			currentDrawItem.m_uniqueID = MakeDrawItemID(GetObjectID(), i);

			gEngine->GetRenderer().QueueStaticItem(currentDrawItem);
		}
	}

	void CMeshComponent::UpdateComponent(float)
	{
		// Only create the draw item if our mesh instance is initialized properly with a mesh and direct transform and the mesh is dynamic (moving around)
		if (!m_meshInstance.m_mesh || !m_meshInstance.m_bVisible)
		{
			return;
		}

		ConstructDrawItem();
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

		// Have the owning SMeshInstance create a draw item to submit to renderer
		m_meshInstance.m_mesh->BuildDrawItems(constructedDrawItems, GetWorldTransform());

		// Set the draw item properties
		for (size_t i = 0; i < constructedDrawItems.size(); ++i)
		{
			auto& currentDrawItem = constructedDrawItems[i];

			currentDrawItem.m_uniqueID = MakeDrawItemID(GetObjectID(), i);

			if (m_bIsDynamic)
			{
				targetRenderer.QueueDynamicItem(currentDrawItem);
			}
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

	void CMeshComponent::Load(const UGameWorldLoader&, const UObjectValue& inPropertyObj)
	{
		inPropertyObj.GetProperty("visible", m_meshInstance.m_bVisible);

		eastl::string meshName;
		if (inPropertyObj.GetProperty("mesh", meshName))
		{
			m_meshInstance.m_mesh = UMesh::Load(meshName);
		}

#ifdef MAD_EDITOR
		m_bIsDynamic = true;
#else
		bool isDynamic = false;
		if (inPropertyObj.GetProperty("dynamic", isDynamic))
		{
			m_bIsDynamic = isDynamic;
		}
#endif
	}

}
