#pragma once

#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/queue.h>

#include "Core/Object.h"
#include "Core/SimpleMath.h"

namespace MAD
{
	class AEntity;

	using ComponentContainer_t = eastl::vector<eastl::weak_ptr<class UComponent>>;
	using ConstComponentContainer_t = eastl::vector<eastl::weak_ptr<const class UComponent>>;

	class UComponent : public UObject
	{
		MAD_DECLARE_BASE_COMPONENT(UComponent, UObject)
	public:
		explicit UComponent(OGameWorld* inOwningWorld);

		virtual ~UComponent() { }

		virtual void PostInitializeComponents() {}
		virtual void OnBeginPlay() {}

		virtual void UpdateComponent(float inDeltaTime) { (void)inDeltaTime; }

		bool IsActive() const { return m_isActive; }
		void SetActive(bool inIsActive) { m_isActive = inIsActive; }

		virtual void Destroy() override;

		void AttachComponent(eastl::shared_ptr<UComponent> inChildComponent);
		
		void UpdateWorldTransform();
		void SetWorldScale(float inScale);
		void SetWorldRotation(const Quaternion& inRotation);
		void SetWorldTranslation(const Vector3& inTranslation);

		void SetRelativeScale(float inScale);
		void SetRelativeRotation(const Quaternion& inRotation);
		void SetRelativeTranslation(const Vector3& inTranslation);

		Vector3 GetComponentForward() const;
		Vector3 GetComponentRight() const;
		Vector3 GetComponentUp() const;

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

		void PrintTranslationHierarchy(uint8_t inDepth) const;
		void PopulateTransformQueue(eastl::queue<ULinearTransform>& inOutTransformQueue) const;
	private:
		friend class AEntity;

		void UpdateChildWorldTransforms();
	private:
		AEntity* m_owningEntity;
		bool m_isActive;
	
		UComponent* m_parentComponent;

		ULinearTransform m_componentLocalTransform;
		ULinearTransform m_componentWorldTransform;

		eastl::vector<eastl::shared_ptr<UComponent>> m_childComponents;
	};
}
