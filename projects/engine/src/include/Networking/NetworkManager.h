#pragma once

#include <EASTL/shared_ptr.h>
#include <yojimbo/yojimbo.h>

#include "Networking/Networking.h"

namespace MAD
{
#define SERVER_DEFAULT_LISTEN_PORT 6000
#define NETWORK_PROTOCOL_ID 0xdeadbeef

	class UNetworkTransport : public yojimbo::SocketTransport
	{
	public:

		UNetworkTransport(const yojimbo::Address& address = yojimbo::Address("0.0.0.0"))
			: SocketTransport(yojimbo::GetDefaultAllocator(), address, NETWORK_PROTOCOL_ID)
		{ }

		~UNetworkTransport();
	};

	/*enum TestPacketTypes
	{
		TEST_PACKET_A = yojimbo::CLIENT_SERVER_NUM_PACKETS,
		NUM_TEST_PACKETS
	};*/

	//YOJIMBO_PACKET_FACTORY_START(TestPacketFactory, yojimbo::ClientServerPacketFactory, NUM_TEST_PACKETS);
	//	//YOJIMBO_DECLARE_PACKET_TYPE(TEST_PACKET_A, TestPacketA);
	//YOJIMBO_PACKET_FACTORY_FINISH();

	struct UEventMessage : public yojimbo::Message
	{
		uint16_t sequence;

		UEventMessage()
		{
			sequence = 0;
		}

		template <typename Stream> bool Serialize(Stream & stream)
		{
			serialize_bits(stream, sequence, 16);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	enum MessageTypes
	{
		EVENT_MESSAGE,
		NUM_MESSAGE_TYPES
	};

	YOJIMBO_MESSAGE_FACTORY_START(UGameMessageFactory, yojimbo::MessageFactory, NUM_MESSAGE_TYPES);
		YOJIMBO_DECLARE_MESSAGE_TYPE(EVENT_MESSAGE, UEventMessage);
	YOJIMBO_MESSAGE_FACTORY_FINISH();

	class UNetworkServer : public yojimbo::Server
	{
		using yojimbo::Server::Server;

		//YOJIMBO_SERVER_PACKET_FACTORY(TestPacketFactory);
		YOJIMBO_SERVER_MESSAGE_FACTORY(UGameMessageFactory);
	};

	class UNetworkClient : public yojimbo::Client
	{
		using yojimbo::Client::Client;
	};

	class UNetworkManager
	{
	public:
		UNetworkManager();

		bool Init();
		void Tick(float inDeltaTime);
		void Shutdown();

	private:
		bool m_isNetworkInitialized;
		ENetMode m_netMode;

		eastl::unique_ptr<UNetworkTransport> m_transport;
		eastl::unique_ptr<UNetworkServer> m_server;
		eastl::unique_ptr<UNetworkClient> m_client;

		bool StartServer(int port);
	};
}
