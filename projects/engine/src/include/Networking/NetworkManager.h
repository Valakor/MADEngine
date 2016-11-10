#pragma once

#include <EASTL/shared_ptr.h>
#include <yojimbo/yojimbo.h>

#include "Networking/Networking.h"

namespace MAD
{
#define SERVER_DEFAULT_LISTEN_PORT 6000
#define NETWORK_PROTOCOL_ID 0xdeadbeef

	enum ENetworkChannels
	{
		UNRELIABLE_CHANNEL,
		RELIABLE_CHANNEL,
		NUM_CHANNELS
	};

	class UNetworkTransport : public yojimbo::SocketTransport
	{
	public:

		UNetworkTransport(const yojimbo::Address& address = yojimbo::Address("0.0.0.0"))
			: SocketTransport(yojimbo::GetDefaultAllocator(), address, NETWORK_PROTOCOL_ID)
		{ }

		~UNetworkTransport();
	};

	struct UPacketA : public yojimbo::Packet
	{
		int a;

		UPacketA()
		{
			a = 1;
		}

		template <typename Stream> bool Serialize(Stream & stream)
		{
			serialize_int(stream, a, -10, 10);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	enum EPacketTypes
	{
		PACKET_A = yojimbo::CLIENT_SERVER_NUM_PACKETS,
		NUM_PACKET_TYPES
	};

	YOJIMBO_PACKET_FACTORY_START(UGamePacketFactory, yojimbo::ClientServerPacketFactory, NUM_PACKET_TYPES);
		YOJIMBO_DECLARE_PACKET_TYPE(PACKET_A, UPacketA);
	YOJIMBO_PACKET_FACTORY_FINISH();

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

	enum EMessageTypes
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

	protected:
		virtual void OnStart(int maxClients) override;
		virtual void OnStop() override;
		virtual void OnConnectionRequest(yojimbo::ServerConnectionRequestAction action, const yojimbo::ConnectionRequestPacket& packet, const yojimbo::Address& address, const yojimbo::ConnectToken& connectToken) override;
		virtual void OnChallengeResponse(yojimbo::ServerChallengeResponseAction action, const yojimbo::ChallengeResponsePacket& packet, const yojimbo::Address& address, const yojimbo::ChallengeToken& challengeToken) override;
		virtual void OnClientConnect(int clientIndex) override;
		virtual void OnClientDisconnect(int clientIndex) override;
		virtual void OnClientError(int clientIndex, yojimbo::ServerClientError error) override;
		virtual void OnPacketSent(int packetType, const yojimbo::Address& to, bool immediate) override;
		virtual void OnPacketReceived(int packetType, const yojimbo::Address& from) override;
		//virtual void OnConnectionPacketSent(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionPacketAcked(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionPacketReceived(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionFragmentReceived(yojimbo::Connection* connection, uint16_t messageId, uint16_t fragmentId, int fragmentBytes, int channelId) override;
		virtual bool ProcessUserPacket(int clientIndex, yojimbo::Packet* packet) override;

	protected:
		YOJIMBO_SERVER_PACKET_FACTORY(UGamePacketFactory);
		YOJIMBO_SERVER_MESSAGE_FACTORY(UGameMessageFactory);
	};

	class UNetworkClient : public yojimbo::Client
	{
		using yojimbo::Client::Client;

	protected:
		virtual void OnConnect(const yojimbo::Address& address) override;
		virtual void OnClientStateChange(int previousState, int currentState) override;
		virtual void OnDisconnect() override;
		virtual void OnPacketSent(int packetType, const yojimbo::Address& to, bool immediate) override;
		virtual void OnPacketReceived(int packetType, const yojimbo::Address& from) override;
		//virtual void OnConnectionPacketSent(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionPacketAcked(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionPacketReceived(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionFragmentReceived(yojimbo::Connection* connection, uint16_t messageId, uint16_t fragmentId, int fragmentBytes, int channelId) override;
		virtual bool ProcessUserPacket(yojimbo::Packet* packet) override;

	protected:
		YOJIMBO_CLIENT_PACKET_FACTORY(UGamePacketFactory);
		YOJIMBO_CLIENT_MESSAGE_FACTORY(UGameMessageFactory);
	};

	class UNetworkManager
	{
	public:
		UNetworkManager();

		bool Init();
		void Tick(float inDeltaTime);
		void Shutdown();

	private:
		ENetMode m_netMode;
		yojimbo::ClientServerConfig m_config;

		eastl::unique_ptr<UNetworkTransport> m_serverTransport;
		eastl::unique_ptr<UNetworkTransport> m_clientTransport;

		eastl::unique_ptr<UNetworkServer> m_server;
		eastl::unique_ptr<UNetworkClient> m_client;

		bool StartServer(int port);
		bool ConnectToServer(const yojimbo::Address& inServerAddress);
	};
}
