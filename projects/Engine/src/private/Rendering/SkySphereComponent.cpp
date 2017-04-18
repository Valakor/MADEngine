#include "Rendering/SkySphereComponent.h"
#include "Core/Pipeline/GameWorldLoader.h"
#include "Core/BaseEngine.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	CSkySphereComponent::CSkySphereComponent(OGameWorld* inOwningWorld) : Super_t(inOwningWorld) {}

	void CSkySphereComponent::PostInitializeComponents()
	{
		auto& renderer = gEngine->GetRenderer();

		eastl::vector<SDrawItem> currentSkySphereItems;

		m_skySphereMesh.m_mesh->BuildDrawItems(currentSkySphereItems, ULinearTransform::CreateScale(GetWorldScale()));

		MAD_ASSERT_DESC(currentSkySphereItems.size() == 1, "Limitation: The sky sphere mesh cannot have multiple submeshes");

		currentSkySphereItems.front().m_rasterizerState = renderer.GetRasterizerState(EFillMode::Solid, ECullMode::Front);

		renderer.SetSkySphereItem(currentSkySphereItems.front());
	}

	void CSkySphereComponent::Load(const UGameWorldLoader&, const UObjectValue& inPropertyObj)
	{
		eastl::string meshAssetPath;

		if (inPropertyObj.GetProperty("mesh", meshAssetPath))
		{
			m_skySphereMesh.m_mesh = UMesh::Load(meshAssetPath);
		}
	}

}
