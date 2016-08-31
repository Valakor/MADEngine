#pragma once

#include "Object.h"

namespace MAD
{
	class AEntity;

	enum class TickType : uint8_t
	{
		TT_PrePhysicsTick = 0,
		TT_PostPhysicsTick = 1,
		TT_TickTypeCount = 2,
	};

	class UComponent : public UObject
	{
		MAD_DECLARE_CLASS(UComponent)

	public:
		UComponent() = delete;
		explicit UComponent(AEntity& inCompOwner, TickType inCompTickType = TickType::TT_PrePhysicsTick);
		virtual ~UComponent() { }

		virtual void UpdateComponent(float inDeltaTime) = 0;

		inline TickType GetTickType() const { return m_tickType; }
		inline AEntity* GetOwner() { return m_owner; }
		inline const AEntity* GetOwner() const { return m_owner; }

	private:
		TickType m_tickType;
		AEntity* m_owner; // Raw pointer to owner is fine in this case because the component will only exist if it's owner exists and we don't have uncertainty as to if it's owner still exists or not or will the component use the owner ptr
	};
}
