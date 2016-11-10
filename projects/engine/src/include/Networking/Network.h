#pragma once

#include <cstdint>
#include <EASTL/numeric_limits.h>

namespace MAD
{
#define SERVER_DEFAULT_LISTEN_PORT 6000
#define NETWORK_PROTOCOL_ID 0xdeadbeef

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
		UNRELIABLE_CHANNEL,
		RELIABLE_CHANNEL,
		NUM_CHANNELS
	};

	struct SNetworkID
	{
		typedef int32_t HandleType;
		static const HandleType Invalid = -1;

		SNetworkID() :
			mHandle(Invalid)
		{ }

	private:

		explicit SNetworkID(HandleType inHandle) :
			mHandle(inHandle)
		{ }

		HandleType mHandle;

	public:
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
}