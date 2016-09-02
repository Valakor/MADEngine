#pragma once

#include "Object.h"

namespace MAD
{
	class AEntity;

	class UComponent : public UObject
	{
		MAD_DECLARE_CLASS(UComponent, UObject)

	public:
		UComponent() = delete;
		explicit UComponent(AEntity& inCompOwner);
		virtual ~UComponent() { }

		virtual void UpdateComponent(float inDeltaTime) = 0;
		
		inline AEntity& GetOwner() { return m_owner; }
		inline const AEntity& GetOwner() const { return m_owner; }

	private:
		AEntity& m_owner; // Raw pointer to owner is fine in this case because the component will only exist if it's owner exists and we don't have uncertainty as to if it's owner still exists or not or will the component use the owner ptr
	};
}
