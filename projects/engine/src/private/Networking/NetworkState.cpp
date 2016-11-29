#include "Networking/NetworkState.h"

#include "yojimbo/yojimbo_stream.h"
#include <yojimbo/yojimbo_serialize.h>

#include "Core/Object.h"
#include "Core/Entity.h"
#include "Core/Component.h"
#include "Networking/NetworkTypes.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogUNetworkState);

	UNetworkState::UNetworkState()
		: m_targetObject(nullptr)
	{}

	void UNetworkState::TargetObject(UObject* inTargetObject, bool inCreateStateBuffer)
	{
		if (!inTargetObject)
		{
			return;
		}

		m_targetObject = inTargetObject;

		m_stateReplInfo.clear();
		
		// Retrieve the replication info of the target object
		m_targetObject->GetReplicatedProperties(m_stateReplInfo);

		if (!inCreateStateBuffer)
		{
			// Don't bother creating and populating a local state buffer (usually reserved for servers only)
			return;
		}

		size_t totalReplSize = 0;

		for (const auto& currentReplInfo : m_stateReplInfo)
		{
			totalReplSize += currentReplInfo.m_replAttrSize;
		}

		m_stateBuffer.resize(totalReplSize);

		size_t currentBufferOffset = 0;

		for (const auto& currentReplInfo : m_stateReplInfo)
		{
			uint8_t* stateBufferData = reinterpret_cast<uint8_t*>(m_stateBuffer.data()) + currentBufferOffset;
			uint8_t* targetReplData = nullptr;

			if (currentReplInfo.m_replAttrOwnerIndex != SObjectReplInfo::InvalidIndex)
			{
				if (AEntity* entityOwner = Cast<AEntity>(m_targetObject))
				{
					// If the owner of the attribute is a component, the target object must be an entity and we need to retrieve the index of
					// component within the entity's component array
					auto targetCompWeakPtr = entityOwner->GetEntityComponentByIndex(currentReplInfo.m_replAttrOwnerIndex);

					if (!targetCompWeakPtr.expired())
					{
						targetReplData = reinterpret_cast<uint8_t*>(targetCompWeakPtr.lock().get()) + currentReplInfo.m_replAttrOffset;
					}
				}
			}
			else
			{
				// Else, we're trying to replicate data within a UObject itself and can perform regular serialization
				targetReplData = reinterpret_cast<uint8_t*>(m_targetObject) + currentReplInfo.m_replAttrOffset;
			}

				// Copy dirty data back to local buffer to detect subsequent data changes
			memcpy(stateBufferData, targetReplData, currentReplInfo.m_replAttrSize);

			currentBufferOffset += currentReplInfo.m_replAttrSize;
		}
	}

	void UNetworkState::SerializeState(eastl::vector<uint8_t>& inOutByteBuffer, bool inIsReading)
	{
		if (!m_targetObject)
		{
			LOG(LogUNetworkState, Warning, "Warning: Trying to serialize state without a target object\n");
			return;
		}

		if (inIsReading)
		{
			yojimbo::ReadStream readStream(inOutByteBuffer.data(), static_cast<int>(inOutByteBuffer.size()));

			DeserializeState_Internal(readStream);
		}
		else
		{
			inOutByteBuffer.resize(MaxStateUpdateSize); // Reserve enough memory for the maximum size of message

			yojimbo::WriteStream writeStream(inOutByteBuffer.data(), MaxStateUpdateSize);
		
			SerializeState_Internal(writeStream);
		}
	}

	bool UNetworkState::SerializeState_Internal(yojimbo::WriteStream& inOutWStream)
	{
		size_t currentBufferOffset = 0;

		using Stream = yojimbo::WriteStream;

		for (const auto& currentReplInfo : m_stateReplInfo)
		{
			uint8_t* stateBufferData = reinterpret_cast<uint8_t*>(m_stateBuffer.data()) + currentBufferOffset;
			uint8_t* targetReplData = nullptr;

			if (currentReplInfo.m_replAttrOwnerIndex != SObjectReplInfo::InvalidIndex)
			{
				if (AEntity* entityOwner = Cast<AEntity>(m_targetObject))
				{
					// If the owner of the attribute is a component, the target object must be an entity and we need to retrieve the index of
					// component within the entity's component array
					auto targetCompWeakPtr = entityOwner->GetEntityComponentByIndex(currentReplInfo.m_replAttrOwnerIndex);

					if (!targetCompWeakPtr.expired())
					{
						targetReplData = reinterpret_cast<uint8_t*>(targetCompWeakPtr.lock().get()) + currentReplInfo.m_replAttrOffset;
					}
				}
			}
			else
			{
				// Else, we're trying to replicate data within a UObject itself and can perform regular serialization
				targetReplData = reinterpret_cast<uint8_t*>(m_targetObject) + currentReplInfo.m_replAttrOffset;
			}

			bool isAttrDirty = !currentReplInfo.m_replComparisonFunc(targetReplData, stateBufferData);

			serialize_bool(inOutWStream, isAttrDirty);
				
			if (isAttrDirty)
			{
				serialize_bytes(inOutWStream, targetReplData, static_cast<int>(currentReplInfo.m_replAttrSize));

				// Copy dirty data back to local buffer to detect subsequent data changes
				memcpy(stateBufferData, targetReplData, currentReplInfo.m_replAttrSize);
			}

			currentBufferOffset += currentReplInfo.m_replAttrSize;
		}

		inOutWStream.Flush();

		return true;
	}
		
	bool UNetworkState::DeserializeState_Internal(yojimbo::ReadStream& inOutRStream)
	{
		using Stream = yojimbo::ReadStream;

		for (const auto& currentReplInfo : m_stateReplInfo)
		{
			bool currentAttrDirty = false;

			serialize_bool(inOutRStream, currentAttrDirty);

			if (currentAttrDirty)
			{
				if (currentReplInfo.m_replAttrOwnerIndex != SObjectReplInfo::InvalidIndex)
				{
					if (AEntity* entityOwner = Cast<AEntity>(m_targetObject))
					{
						// If the owner of the attribute is a component, the target object must be an entity and we need to retrieve the index of
						// component within the entity's component array
						auto targetCompWeakPtr = entityOwner->GetEntityComponentByIndex(currentReplInfo.m_replAttrOwnerIndex);

						if (!targetCompWeakPtr.expired())
						{
							uint8_t* compAttrRawAddress = reinterpret_cast<uint8_t*>(targetCompWeakPtr.lock().get()) + currentReplInfo.m_replAttrOffset;
							
							serialize_bytes(inOutRStream, compAttrRawAddress, static_cast<int>(currentReplInfo.m_replAttrSize));
						}
					}
				}
				else
				{
					// Else, we're trying to replicate data within a UObject itself and can perform regular serialization
					uint8_t* targetReplData = reinterpret_cast<uint8_t*>(m_targetObject) + currentReplInfo.m_replAttrOffset;

					serialize_bytes(inOutRStream, targetReplData, static_cast<int>(currentReplInfo.m_replAttrSize));
				}

				currentReplInfo.m_replCallback.ExecuteIfBound();
			}
		}

		return true;
	}
}
