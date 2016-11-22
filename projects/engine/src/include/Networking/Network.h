#pragma once

#include <cstdint>
#include <EASTL/numeric_limits.h>

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

		inline bool IsValid() const { return mHandle != Invalid; }

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

	struct SObjectReplInfo
	{
		static const int32_t InvalidIndex = -1;

		EReplicationType m_replType;

		int32_t						 m_replAttrOwnerIndex = InvalidIndex; // Owner is component is not -1 and either UObject or AEntity type if equal to -1
		ReplAttrOffset_t			 m_replAttrOffset;
		ReplAttrSize_t				 m_replAttrSize;

		ReplComparisonFunc_t m_replComparisonFunc; // Returns true if equal
	};

	int32_t DetermineComponentIndex(class UComponent* inTargetComponent);
	
	template <typename T>
	bool IsSame(const void* inLeftData, const void* inRightData)
	{
		const T* inLeftTypeData = reinterpret_cast<const T*>(inLeftData);
		const T* inRightTypeData = reinterpret_cast<const T*>(inRightData);

		return *inLeftTypeData == *inRightTypeData;
	}

#define MAD_ADD_REPLICATION_PROPERTY(OutPropContainer, ReplType, OwnerType, ReplVarName)						\
		do																										\
		{																										\
			SObjectReplInfo outReplInfo;																		\
																												\
			outReplInfo.m_replType = ReplType;																	\
																												\
			if (UComponent* componentOwner = Cast<UComponent>(this))											\
			{																									\
				/* Find the index that this component is within it's owner */									\
				outReplInfo.m_replAttrOwnerIndex = DetermineComponentIndex(componentOwner);						\
			}																									\
																												\
			outReplInfo.m_replAttrOffset = offsetof(OwnerType, ReplVarName);									\
			outReplInfo.m_replAttrSize = sizeof(ReplVarName);													\
			outReplInfo.m_replComparisonFunc = &IsSame<decltype(ReplVarName), sizeof(ReplVarName)>;				\
																												\
			OutPropContainer.push_back(outReplInfo);															\
																												\
		} while (0)

}
