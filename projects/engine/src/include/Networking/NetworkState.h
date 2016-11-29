#pragma once

#include <cstdint>

#include <EASTL/vector.h>
#include <EASTL/algorithm.h>

#include "Networking/Network.h"

namespace MAD
{
	class UNetworkState
	{
	public:
		UNetworkState();
		UNetworkState(const UNetworkState&) = delete;
		UNetworkState& operator=(const UNetworkState&) = delete;

		void TargetObject(class UObject* inTargetObject, bool inCreateStateBuffer);

		void SerializeState(eastl::vector<uint8_t>& inOutByteBuffer, bool inIsReading);
	private:
		bool SerializeState_Internal(yojimbo::WriteStream& inOutWStream);
		bool DeserializeState_Internal(yojimbo::ReadStream& inOutRStream);
	private:
		class UObject* m_targetObject;
		eastl::vector<uint8_t> m_stateBuffer;
		eastl::vector<SObjectReplInfo> m_stateReplInfo;
	};
}
