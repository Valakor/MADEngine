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

		bool IsOwnerValid() const;
		void SetOwningEntity(AEntity& inOwner) { m_parentEntity = &inOwner; }
		AEntity& GetOwningEntity() { return *m_parentEntity; }
		const AEntity& GetOwningEntity() const { return *m_parentEntity; }
		UComponent* GetParent() const { return m_parentComponent; }

		virtual void Load(const class UGameWorldLoader& inLoader) { (void)inLoader; }
	private:
		AEntity* m_parentEntity;
	
		UComponent* m_parentComponent;

		eastl::vector<eastl::shared_ptr<UComponent>> m_attachedChildren;
	};
}
