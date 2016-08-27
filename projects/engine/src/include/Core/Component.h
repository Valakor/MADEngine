#pragma once

#include "Object.h"

#include <memory>

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
		UComponent(AEntity* inCompOwner, TickType inCompTickType = TickType::TT_PrePhysicsTick);

		virtual void UpdateComponent(float inDeltaTime) = 0;

		inline TickType GetTickType() const { return m_tickType; }
		inline AEntity* GetOwner() { return m_owner; }
		inline const AEntity* GetOwner() const { return m_owner; }
	private:
		TickType m_tickType;
		AEntity* m_owner; // Raw pointer to owner is fine in this case because the component will only exist if it's owner exists and we don't have uncertainty as to if it's owner still exists or not or will the component use the owner ptr
	};
}

namespace std
{
	template <>
	struct std::hash<std::weak_ptr<MAD::UComponent>>
	{
	public:
		size_t operator()(const std::weak_ptr<MAD::UComponent>& inObject) const
		{
			if (!inObject.expired())
			{
				return inObject.lock()->GetObjectID(); // ObjectID hash values for now (maybe something more complex in the future?)
			}
			else
			{
				return 0;
			}
		}
	};

	template <>
	struct std::equal_to<std::weak_ptr<MAD::UComponent>>
	{
	public:
		bool operator()(const std::weak_ptr<MAD::UComponent>& inLeft, const std::weak_ptr<MAD::UComponent>& inRight) const
		{
			if (inLeft.expired() && inRight.expired())
			{
				return true;
			}
			else if (!inLeft.expired() && !inRight.expired())
			{
				return inLeft.lock()->GetObjectID() == inRight.lock()->GetObjectID();
			}
			else
			{
				return false;
			}
		}
	};
}