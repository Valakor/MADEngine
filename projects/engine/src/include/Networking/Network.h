#pragma once

#include <cstdint>
#include <EASTL/numeric_limits.h>

#include "Core/SimpleMath.h"
#include "Misc/Delegate.h"

namespace MAD
{
#define SERVER_DEFAULT_LISTEN_PORT 6000
#define NETWORK_PROTOCOL_ID 0xdeadbeef

	using NetworkPlayerID = int;
	static const NetworkPlayerID InvalidPlayerID = -1;

	enum class ENetRole : uint8_t
	{
		None = 0,
		Simulated_Proxy,
		Autonomous_Proxy,
		Authority
	};

	enum class ENetMode : uint8_t
	{
		Client = 0,
		ListenServer,
		DedicatedServer
	};

	enum ENetworkChannels
	{
		RELIABLE_CHANNEL,
		UNRELIABLE_CHANNEL,
		NUM_CHANNELS
	};

	struct SNetworkID
	{
		typedef uint16_t HandleType;
		static const HandleType Invalid = eastl::numeric_limits<HandleType>::max();

		SNetworkID() :
			mHandle(Invalid)
		{ }

	private:

		explicit SNetworkID(HandleType inHandle) :
			mHandle(inHandle)
		{ }

		HandleType mHandle;

	public:
		inline HandleType& GetUnderlyingHandleRef() { return mHandle; }
		inline HandleType GetUnderlyingHandle() const { return mHandle; }

		inline bool IsValid() const { return mHandle != Invalid; }

		inline void Invalidate() { mHandle = Invalid; }

		inline bool operator==(const SNetworkID& rhs) const noexcept
		{
			return mHandle == rhs.mHandle;
		}

		inline bool operator!=(const SNetworkID& rhs) const noexcept
		{
			return !operator==(rhs);
		}

		inline bool operator==(HandleType h) const noexcept
		{
			return mHandle == h;
		}

		inline bool operator!=(HandleType h) const noexcept
		{
			return !operator==(h);
		}
	};

	using ReplComparisonFunc_t = bool(*)(const void*, const void*);
	using ReplAttrOffset_t = size_t;
	using ReplAttrSize_t = size_t;

	enum class EReplicationType : uint8_t
	{
		Always,
		InitialOnly
	};

	DECLARE_DELEGATE(OnRepDelegate, void);

	struct SObjectReplInfo
	{
		static const int32_t InvalidIndex = -1;

		EReplicationType m_replType;

		int32_t						 m_replAttrOwnerIndex = InvalidIndex; // Owner is component is not -1 and either UObject or AEntity type if equal to -1
		ReplAttrOffset_t			 m_replAttrOffset;
		ReplAttrSize_t				 m_replAttrSize;

		ReplComparisonFunc_t m_replComparisonFunc; // Returns true if equal
		OnRepDelegate m_replCallback;
	};

	int32_t DetermineComponentIndex(const class UComponent* inTargetComponent);
	
	template <typename T>
	inline bool IsSame(const void* inLHS, const void* inRHS)
	{
		const T* inLeftTypeData = reinterpret_cast<const T*>(inLHS);  // Actual object
		const T* inRightTypeData = reinterpret_cast<const T*>(inRHS); // Local state buffer

		return *inLeftTypeData == *inRightTypeData;
	}

	template <>
	inline bool IsSame<float>(const void* inLHS, const void* inRHS)
	{
		const float inLeftTypeData = *reinterpret_cast<const float*>(inLHS);  // Actual object
		const float inRightTypeData = *reinterpret_cast<const float*>(inRHS); // Local state buffer

		return FloatEqual(inLeftTypeData, inRightTypeData);
	}

#define MAD_ADD_REPLICATION_PROPERTY(OutPropContainer, ReplType, OwnerType, ReplVarName)						\
		do																										\
		{																										\
			SObjectReplInfo outReplInfo;																		\
																												\
			outReplInfo.m_replType = ReplType;																	\
																												\
			if (const UComponent* componentOwner = Cast<const UComponent>(this))								\
			{																									\
				/* Find the index that this component is within it's owner */									\
				outReplInfo.m_replAttrOwnerIndex = DetermineComponentIndex(componentOwner);						\
			}																									\
																												\
			outReplInfo.m_replAttrOffset = offsetof(OwnerType, ReplVarName);									\
			outReplInfo.m_replAttrSize = sizeof(ReplVarName);													\
			outReplInfo.m_replComparisonFunc = &IsSame<decltype(ReplVarName)>;									\
																												\
			OutPropContainer.push_back(outReplInfo);															\
																												\
		} while (0)

#define MAD_ADD_REPLICATION_PROPERTY_CALLBACK(OutPropContainer, ReplType, OwnerType, ReplVarName, OnRepFunc)	\
		do																										\
		{																										\
			SObjectReplInfo outReplInfo;																		\
																												\
			outReplInfo.m_replType = ReplType;																	\
																												\
			if (const UComponent* componentOwner = Cast<const UComponent>(this))								\
			{																									\
				/* Find the index that this component is within it's owner */									\
				outReplInfo.m_replAttrOwnerIndex = DetermineComponentIndex(componentOwner);						\
			}																									\
																												\
			outReplInfo.m_replAttrOffset = offsetof(OwnerType, ReplVarName);									\
			outReplInfo.m_replAttrSize = sizeof(ReplVarName);													\
			outReplInfo.m_replComparisonFunc = &IsSame<decltype(ReplVarName)>;									\
			outReplInfo.m_replCallback.BindMember<OwnerType, &OwnerType::OnRepFunc>(this);						\
																												\
			OutPropContainer.push_back(outReplInfo);															\
																												\
		} while (0)

}

namespace eastl
{
	using namespace MAD;
	template <> struct hash<SNetworkID>
	{
		size_t operator()(SNetworkID val) const
		{
			return hash<SNetworkID::HandleType>{}(val.GetUnderlyingHandleRef());
		}
	};
}
