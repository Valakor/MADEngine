#pragma once

#include <yojimbo/yojimbo.h>

#include "Networking/NetworkTypes.h"

namespace MAD
{
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
		//virtual bool ProcessUserPacket(yojimbo::Packet* packet) override;

	protected:
		YOJIMBO_CLIENT_MESSAGE_FACTORY(UGameMessageFactory);
	};
}
