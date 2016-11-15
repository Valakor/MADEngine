#pragma once

#include <yojimbo/yojimbo.h>

#include "Networking/NetworkTypes.h"

namespace MAD
{
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
		//virtual bool ProcessUserPacket(int clientIndex, yojimbo::Packet* packet) override;

	protected:
		YOJIMBO_SERVER_MESSAGE_FACTORY(UGameMessageFactory);
	};
}
