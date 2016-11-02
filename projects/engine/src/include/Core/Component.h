#pragma once

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

		bool IsOwnerValid() const;
		void SetOwner(AEntity& inOwner) { m_ownerPtr = &inOwner; }
		void SetScale(float inScale);
		void SetRotation(const Quaternion& inRotation);
		void SetTranslation(const Vector3& inTranslation);

		AEntity& GetOwner() { return *m_ownerPtr; }
		const AEntity& GetOwner() const { return *m_ownerPtr; }
		ULinearTransform GetWorldTransform() const { return m_worldTransform; }
		
		virtual void Load(const class UGameWorldLoader& inLoader) { (void)inLoader; }
	private:
		void UpdateComponentTransform();
	private:
		AEntity* m_ownerPtr;

		ULinearTransform m_worldTransform;

		float m_componentScale;
		Quaternion m_componentRotation;
		Vector3 m_componentTranslation;
	};
}
