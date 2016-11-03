#pragma once

#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>

#include "Core/Object.h"
#include "Core/SimpleMath.h"

namespace MAD
{
	class AEntity;

	class UComponent : public UObject
	{
		MAD_DECLARE_BASE_COMPONENT(UComponent, UObject)
	public:
		explicit UComponent(OGameWorld* inOwningWorld);

		virtual ~UComponent() { }

		virtual void UpdateComponent(float inDeltaTime) { (void)inDeltaTime; };

		void AttachComponent(eastl::shared_ptr<UComponent> inChildComponent);
		
		void SetWorldScale(float inScale);
		void SetWorldRotation(const Quaternion& inRotation);
		void SetWorldTranslation(const Vector3& inTranslation);
		
		const ULinearTransform& GetWorldTransform() const { return m_componentWorldTransform; }
		float GetWorldScale() const { return m_componentWorldTransform.GetScale(); }
		const Quaternion& GetWorldRotation() const { return m_componentWorldTransform.GetRotation(); }
		const Vector3& GetWorldTranslation() const { return m_componentWorldTransform.GetTranslation(); }

		bool IsOwnerValid() const;
		void SetOwningEntity(AEntity& inOwner) { m_owningEntity = &inOwner; }
		AEntity& GetOwningEntity() { return *m_owningEntity; }
		const AEntity& GetOwningEntity() const { return *m_owningEntity; }
		UComponent* GetParent() const { return m_parentComponent; }

		virtual void Load(const class UGameWorldLoader& inLoader) { (void)inLoader; }
	private:
		friend class AEntity;

		void AttachToParent(AEntity* inParent);
		void UpdateWorldTransform();
		void UpdateChildWorldTransforms();
	private:
		AEntity* m_owningEntity;
	
		UComponent* m_parentComponent;

		ULinearTransform m_componentWorldTransform;

		eastl::vector<eastl::shared_ptr<UComponent>> m_childComponents;
	};
}
