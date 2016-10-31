#include "Core/Entity.h"
#include "Core/GameWorld.h"
#include "Core/GameWorldLayer.h"

namespace MAD
{
	AEntity::AEntity(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
		, m_isPendingForKill(false)
		, m_owningWorldLayer(nullptr)
		, m_isTransformDirty(true)
		, m_entityParent(nullptr)
	{}

	void AEntity::Destroy()
	{
		// Destroying an Entity means to basically destroy all of it's attached components
		// HOWEVER, an Entity shouldn't be destroyed right away or else we will run into issues if
		// we try to destroy an Entity within a Component's update function. Destroying an Entity would mean that we need to destroy all
		// of it's components, but obviously we can't modify the component lists while iterating over them'
		// THEREFORE, we must mark the entity as pending for kill so that we can safely destroy the entity
		m_isPendingForKill = true;
	}

	OGameWorld& AEntity::GetWorld()
	{
		return *m_owningWorldLayer->GetOwningWorld();
	}

	const OGameWorld& AEntity::GetWorld() const
	{
		return *m_owningWorldLayer->GetOwningWorld();
	}

	void AEntity::GetEntityComponents(ConstComponentContainer& inOutConstEntityComponents) const
	{
		for (auto& currentComponent : m_actorComponents)
		{
			inOutConstEntityComponents.emplace_back(currentComponent);
		}
	}

	void AEntity::GetEntityComponents(ComponentContainer& inOutEntityComponents)
	{
		for (auto& currentComponent : m_actorComponents)
		{
			inOutEntityComponents.emplace_back(currentComponent);
		}
	}

	void AEntity::UpdateEntityWorldTransform()
	{
		// Walk the parent hierarchy
		ULinearTransform finalWorldTransform = m_localTransform;
		
		if (m_entityParent)
		{
			finalWorldTransform *= m_entityParent->GetWorldTransform();
		}

		m_worldTransform = finalWorldTransform;
	}

	void AEntity::SetScale(float inScale)
	{
		m_entityScale = inScale;
		
	}

	void AEntity::SetRotation(const Quaternion& inRotation)
	{
		
	}

	void AEntity::SetPosition(const Vector3& inPosition)
	{
		
	}
}
