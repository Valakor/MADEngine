/*
    Yojimbo Client/Server Network Protocol Library.
    
    Copyright © 2016, The Network Protocol Company, Inc.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
           in the documentation and/or other materials provided with the distribution.

        3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived 
           from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
    USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "yojimbo_config.h"
#include "yojimbo_server.h"
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

namespace yojimbo
{
    void Server::Defaults()
    {
        m_allocator = NULL;
        m_globalMemory = NULL;
        m_globalAllocator = NULL;
        m_transport = NULL;
        m_allocateConnections = false;
        m_time = 0.0;
        m_flags = 0;
        m_maxClients = -1;
        m_numConnectedClients = 0;
        m_challengeTokenNonce = 0;
        m_globalSequence = 1ULL<<63;
        m_globalPacketFactory = NULL;
        memset( m_privateKey, 0, KeyBytes );
        memset( m_clientMemory, 0, sizeof( m_clientMemory ) );
        memset( m_clientAllocator, 0, sizeof( m_clientAllocator ) );
        memset( m_clientMessageFactory, 0, sizeof( m_clientMessageFactory ) );
        memset( m_clientPacketFactory, 0, sizeof( m_clientPacketFactory ) );
        memset( m_clientReplayProtection, 0, sizeof( m_clientReplayProtection ) );
        memset( m_clientConnection, 0, sizeof( m_clientConnection ) );
        memset( m_clientSequence, 0, sizeof( m_clientSequence ) );
        memset( m_counters, 0, sizeof( m_counters ) );
        for ( int i = 0; i < MaxClients; ++i )
            ResetClientState( i );
    }

    Server::Server( Allocator & allocator, Transport & transport, const ClientServerConfig & config, double time )
    {
        Defaults();
        m_allocator = &allocator;
        m_transport = &transport;
        m_config = config;
        m_config.connectionConfig.connectionPacketType = CLIENT_SERVER_PACKET_CONNECTION;
        m_allocateConnections = m_config.enableConnection;
        m_time = time;
    }

    Server::~Server()
    {
		// IMPORTANT: You must stop the server before you destroy it
		assert( !IsRunning() );

        assert( m_transport );

        m_transport = NULL;
    }

    void Server::SetPrivateKey( const uint8_t * privateKey )
    {
        memcpy( m_privateKey, privateKey, KeyBytes );
    }

    void Server::SetServerAddress( const Address & address )
    {
        m_serverAddress = address;
    }

    void Server::Start( int maxClients )
    {
        assert( maxClients > 0 );
        assert( maxClients <= MaxClients );

        Stop();

        m_maxClients = maxClients;

        CreateAllocators();

        // global resources

        Allocator & globalAllocator = GetAllocator( SERVER_RESOURCE_GLOBAL );

        if ( !m_globalPacketFactory )
        {
            m_globalPacketFactory = CreatePacketFactory( globalAllocator, SERVER_RESOURCE_GLOBAL );

            assert( m_globalPacketFactory );
        }

        m_globalTransportContext = TransportContext( *m_globalAllocator, *m_globalPacketFactory );;

        m_transport->SetContext( m_globalTransportContext );

        // per-client resources

        for ( int clientIndex = 0; clientIndex < m_maxClients; ++clientIndex )
        {
            Allocator & clientAllocator = GetAllocator( SERVER_RESOURCE_PER_CLIENT, clientIndex );

            m_clientPacketFactory[clientIndex] = CreatePacketFactory( clientAllocator, SERVER_RESOURCE_PER_CLIENT, clientIndex );

            assert( m_clientPacketFactory[clientIndex] );

            assert( m_clientPacketFactory[clientIndex]->GetNumPacketTypes() == m_globalPacketFactory->GetNumPacketTypes() );

            m_clientReplayProtection[clientIndex] = YOJIMBO_NEW( clientAllocator, ReplayProtection );
        }

        if ( m_allocateConnections )
        {
            for ( int clientIndex = 0; clientIndex < m_maxClients; ++clientIndex )
            {
                Allocator & clientAllocator = GetAllocator( SERVER_RESOURCE_PER_CLIENT, clientIndex );

                m_clientMessageFactory[clientIndex] = CreateMessageFactory( clientAllocator, SERVER_RESOURCE_PER_CLIENT, clientIndex );
                
                assert( m_clientMessageFactory[clientIndex] );

                m_clientConnection[clientIndex] = YOJIMBO_NEW( clientAllocator, Connection, clientAllocator, *m_clientPacketFactory[clientIndex], *m_clientMessageFactory[clientIndex], m_config.connectionConfig );
               
                m_clientConnection[clientIndex]->SetListener( this );

                m_clientConnection[clientIndex]->SetClientIndex( clientIndex );
            }
        }

        for ( int clientIndex = 0; clientIndex < m_maxClients; ++clientIndex )
        {
            m_clientTransportContext[clientIndex].allocator = m_clientAllocator[clientIndex];
            m_clientTransportContext[clientIndex].packetFactory = m_clientPacketFactory[clientIndex];
            m_clientTransportContext[clientIndex].replayProtection = m_clientReplayProtection[clientIndex];

            if ( m_allocateConnections )
            {
                m_clientConnectionContext[clientIndex].connectionConfig = &m_config.connectionConfig;
                m_clientConnectionContext[clientIndex].messageFactory = m_clientMessageFactory[clientIndex];
                m_clientTransportContext[clientIndex].connectionContext = &m_clientConnectionContext[clientIndex];
            }
        }     

        SetEncryptedPacketTypes();

        OnStart( maxClients );
    }

    void Server::Stop()
    {
        if ( !IsRunning() )
            return;

        OnStop();

        DisconnectAllClients();

        m_transport->ClearContext();

        m_transport->Reset();

        for ( int clientIndex = 0; clientIndex < m_maxClients; ++clientIndex )
        {
            Allocator & clientAllocator = GetAllocator( SERVER_RESOURCE_PER_CLIENT, clientIndex );

            YOJIMBO_DELETE( clientAllocator, Connection, m_clientConnection[clientIndex] );

            YOJIMBO_DELETE( clientAllocator, MessageFactory, m_clientMessageFactory[clientIndex] );

            YOJIMBO_DELETE( clientAllocator, PacketFactory, m_clientPacketFactory[clientIndex] );

            YOJIMBO_DELETE( clientAllocator, ReplayProtection, m_clientReplayProtection[clientIndex] );
        }

        Allocator & globalAllocator = GetAllocator( SERVER_RESOURCE_GLOBAL );

        YOJIMBO_DELETE( globalAllocator, PacketFactory, m_globalPacketFactory );

        DestroyAllocators();

        m_maxClients = -1;
    }

    void Server::DisconnectClient( int clientIndex, bool sendDisconnectPacket )
    {
        assert( IsRunning() );
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        assert( m_numConnectedClients > 0 );
        assert( m_clientConnected[clientIndex] );

        OnClientDisconnect( clientIndex );

        if ( sendDisconnectPacket )
        {
            for ( int i = 0; i < m_config.numDisconnectPackets; ++i )
            {
                DisconnectPacket * packet = (DisconnectPacket*) CreateGlobalPacket( CLIENT_SERVER_PACKET_DISCONNECT );
                if ( packet )
                {
                    SendPacketToConnectedClient( clientIndex, packet, true );
                }
            }
        }

        m_transport->RemoveContextMapping( m_clientData[clientIndex].address );

#if YOJIMBO_INSECURE_CONNECT
        if ( !m_clientData[clientIndex].insecure )
#endif // #if YOJIMBO_INSECURE_CONNECT
        {
            m_transport->RemoveEncryptionMapping( m_clientData[clientIndex].address );
        }

        ResetClientState( clientIndex );

        m_counters[SERVER_COUNTER_CLIENT_DISCONNECTS]++;

        m_numConnectedClients--;
    }

    void Server::DisconnectAllClients( bool sendDisconnectPacket )
    {
        assert( IsRunning() );

        for ( int i = 0; i < m_maxClients; ++i )
        {
            if ( m_clientConnected[i] )
                DisconnectClient( i, sendDisconnectPacket );
        }
    }

    Message * Server::CreateMsg( int clientIndex, int type )
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        assert( m_clientMessageFactory[clientIndex] );
        return m_clientMessageFactory[clientIndex]->Create( type );
    }

    bool Server::CanSendMsg( int clientIndex ) const
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );

        if ( !IsRunning() )
            return false;

        if ( !IsClientConnected( clientIndex ) )
            return false;

        assert( m_clientMessageFactory );
        
        assert( m_clientConnection[clientIndex] );

        return m_clientConnection[clientIndex]->CanSendMsg();
    }

    void Server::SendMsg( int clientIndex, Message * message )
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        assert( m_clientMessageFactory[clientIndex] );

        if ( !m_clientConnected[clientIndex] )
        {
            m_clientMessageFactory[clientIndex]->Release( message );
            return;
        }

        assert( m_clientConnection[clientIndex] );

        m_clientConnection[clientIndex]->SendMsg( message );
    }

    Message * Server::ReceiveMsg( int clientIndex )
    {
        assert( m_clientMessageFactory );

        if ( !m_clientConnected[clientIndex] )
            return NULL;

        assert( m_clientConnection[clientIndex] );

        return m_clientConnection[clientIndex]->ReceiveMsg();
    }

    void Server::ReleaseMsg( int clientIndex, Message * message )
    {
        assert( message );
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        assert( m_clientMessageFactory[clientIndex] );
        m_clientMessageFactory[clientIndex]->Release( message );
    }

    MessageFactory & Server::GetMsgFactory( int clientIndex )
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        assert( m_clientMessageFactory[clientIndex] );
        return *m_clientMessageFactory[clientIndex];
    }

    Packet * Server::CreateGlobalPacket( int type )
    {
        return m_globalPacketFactory->CreatePacket( type );
    }

    Packet * Server::CreateClientPacket( int clientIndex, int type )
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        assert( m_clientPacketFactory[clientIndex] );
        return m_clientPacketFactory[clientIndex]->CreatePacket( type );
    }

    void Server::SendPackets()
    {
        if ( !IsRunning() )
            return;

        const double time = GetTime();

        for ( int clientIndex = 0; clientIndex < m_maxClients; ++clientIndex )
        {
            if ( !m_clientConnected[clientIndex] )
                continue;

            if ( m_clientData[clientIndex].fullyConnected )
            {
                if ( m_clientConnection[clientIndex] )
                {
                    ConnectionPacket * packet = m_clientConnection[clientIndex]->GeneratePacket();

                    if ( packet )
                    {
                        SendPacketToConnectedClient( clientIndex, packet );
                    }
                }
            }

            if ( m_clientData[clientIndex].lastPacketSendTime + m_config.connectionKeepAliveSendRate <= time )
            {
                KeepAlivePacket * packet = CreateKeepAlivePacket( clientIndex );

                if ( packet )
                {
                    SendPacketToConnectedClient( clientIndex, packet );

                    m_clientData[clientIndex].lastPacketSendTime = GetTime();

                    debug_printf( "server send keep alive packet to client %d - clientSalt = %" PRIx64 "\n", clientIndex, packet->clientSalt );
                }
            }
        }
    }

    void Server::ReceivePackets()
    {
        while ( true )
        {
            Address address;
            uint64_t sequence;
            Packet * packet = m_transport->ReceivePacket( address, &sequence );

            if ( !packet )
                break;

            if ( IsRunning() )
                ProcessPacket( packet, address, sequence );

            packet->Destroy();
        }
    }

    void Server::CheckForTimeOut()
    {
        if ( !IsRunning() )
            return;

        const double time = GetTime();

        for ( int clientIndex = 0; clientIndex < m_maxClients; ++clientIndex )
        {
            if ( !m_clientConnected[clientIndex] )
                continue;

            if ( m_clientData[clientIndex].lastPacketReceiveTime + m_config.connectionTimeOut < time )
            {
                OnClientError( clientIndex, SERVER_CLIENT_ERROR_TIMEOUT );

                m_counters[SERVER_COUNTER_CLIENT_TIMEOUTS]++;

                DisconnectClient( clientIndex, false );
            }
        }
    }

    void Server::AdvanceTime( double time )
    {
        assert( time >= m_time );

        m_time = time;

        // check for global allocator error, increase counter and clear error. nothing we can do but take note.

        if ( m_globalAllocator->GetError() )
        {
            m_counters[SERVER_COUNTER_GLOBAL_ALLOCATOR_ERRORS]++;

            m_globalAllocator->ClearError();
        }

        // check for global packet factory error, increase counter and clear error. nothing we can do but take note.

        if ( m_globalPacketFactory->GetError() )
        {
            m_counters[SERVER_COUNTER_GLOBAL_PACKET_FACTORY_ERRORS]++;

            m_globalPacketFactory->ClearError();
        }

        for ( int clientIndex = 0; clientIndex < m_maxClients; ++clientIndex )
        {
            if ( IsClientConnected( clientIndex ) )
            {
                // check for allocator error

                if ( m_clientAllocator[clientIndex]->GetError() )
                {
                    OnClientError( clientIndex, SERVER_CLIENT_ERROR_ALLOCATOR );

                    m_counters[SERVER_COUNTER_CLIENT_ALLOCATOR_ERRORS]++;

                    DisconnectClient( clientIndex, true );

                    continue;
                }

                // check for message factory error

                if ( m_clientMessageFactory[clientIndex] )
                {
                    if ( m_clientMessageFactory[clientIndex]->GetError() )
                    {
                        OnClientError( clientIndex, SERVER_CLIENT_ERROR_MESSAGE_FACTORY );

                        m_counters[SERVER_COUNTER_CLIENT_MESSAGE_FACTORY_ERRORS]++;

                        DisconnectClient( clientIndex, true );

                        continue;
                    }
                }

                // check for packet factory error

                if ( m_clientPacketFactory[clientIndex]->GetError() )
                {
                    OnClientError( clientIndex, SERVER_CLIENT_ERROR_PACKET_FACTORY );

                    m_counters[SERVER_COUNTER_CLIENT_PACKET_FACTORY_ERRORS]++;

                    DisconnectClient( clientIndex, true );

                    continue;
                }

                // check for connection error

                if ( m_clientConnection[clientIndex] )
                {
                    m_clientConnection[clientIndex]->AdvanceTime( time );

                    if ( m_clientConnection[clientIndex]->GetError() )
                    {
                        OnClientError( clientIndex, SERVER_CLIENT_ERROR_CONNECTION );

                        m_counters[SERVER_COUNTER_CLIENT_CONNECTION_ERRORS]++;

                        DisconnectClient( clientIndex, true );

                        continue;
                    }
                }
            }
        }
    }

    void Server::SetFlags( uint64_t flags )
    {
        m_flags = flags;
    }

    bool Server::IsRunning() const
    {
        return m_maxClients > 0;
    }

    int Server::GetMaxClients() const
    {
        return m_maxClients;
    }

    bool Server::IsClientConnected( int clientIndex ) const
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        return m_clientConnected[clientIndex];
    }

    const Address & Server::GetServerAddress() const
    {
        return m_serverAddress;
    }

    int Server::FindClientIndex( uint64_t clientId ) const
    {
        for ( int i = 0; i < m_maxClients; ++i )
        {   
            if ( m_clientConnected[i] && m_clientId[i] == clientId )
                return i;
        }

        return -1;
    }

    int Server::FindClientIndex( const Address & address ) const
    {
        if ( !address.IsValid() )
            return -1;

        for ( int i = 0; i < m_maxClients; ++i )
        {   
            if ( m_clientConnected[i] && m_clientAddress[i] == address )
                return i;
        }

        return -1;
    }

    uint64_t Server::GetClientId( int clientIndex ) const
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        return m_clientId[clientIndex];
    }

    const Address & Server::GetClientAddress( int clientIndex ) const
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        return m_clientAddress[clientIndex];
    }

    int Server::GetNumConnectedClients() const
    {
        return m_numConnectedClients;
    }

    uint64_t Server::GetCounter( int index ) const 
    {
        assert( index >= 0 );
        assert( index < NUM_SERVER_COUNTERS );
        return m_counters[index];
    }

    double Server::GetTime() const
    {
        return m_time;
    }

    uint64_t Server::GetFlags() const
    {
        return m_flags;
    }

    void Server::CreateAllocators()
    {
        assert( m_globalMemory == NULL );

        m_globalMemory = (uint8_t*) YOJIMBO_ALLOCATE( *m_allocator, m_config.serverGlobalMemory );

        m_globalAllocator = CreateAllocator( *m_allocator, m_globalMemory, m_config.serverGlobalMemory );

        for ( int i = 0; i < m_maxClients; ++i )
        {
            m_clientMemory[i] = (uint8_t*) YOJIMBO_ALLOCATE( *m_allocator, m_config.serverPerClientMemory );

            m_clientAllocator[i] = CreateAllocator( *m_allocator, m_clientMemory[i], m_config.serverPerClientMemory );
        }
    }

    void Server::DestroyAllocators()
    {
        assert( m_globalMemory );
        assert( m_globalAllocator );

        for ( int i = 0; i < m_maxClients; ++i )
        {
            assert( m_clientMemory[i] );
            assert( m_clientAllocator[i] );

            YOJIMBO_DELETE( *m_allocator, Allocator, m_clientAllocator[i] );
            YOJIMBO_FREE( *m_allocator, m_clientMemory[i] );
        }

        YOJIMBO_DELETE( *m_allocator, Allocator, m_globalAllocator );
        YOJIMBO_FREE( *m_allocator, m_globalMemory );
    }

    Allocator * Server::CreateAllocator( Allocator & allocator, void * memory, size_t bytes )
    {
        return YOJIMBO_NEW( allocator, TLSF_Allocator, memory, bytes );
    }

    Allocator & Server::GetAllocator( ServerResourceType type, int clientIndex )
    {
        if ( type == SERVER_RESOURCE_GLOBAL )
        {
            assert( m_globalAllocator );
            return *m_globalAllocator;
        }
        else
        {
            assert( clientIndex >= 0 );
            assert( clientIndex <= m_maxClients );
            assert( m_clientAllocator[clientIndex] );
            return *m_clientAllocator[clientIndex];
        }
    }

    void Server::SetEncryptedPacketTypes()
    {
        m_transport->EnablePacketEncryption();

        m_transport->DisableEncryptionForPacketType( CLIENT_SERVER_PACKET_CONNECTION_REQUEST );
    }

    PacketFactory * Server::CreatePacketFactory( Allocator & allocator, ServerResourceType /*type*/, int /*clientIndex*/ )
    {
        return YOJIMBO_NEW( allocator, ClientServerPacketFactory, allocator );
    }

    MessageFactory * Server::CreateMessageFactory( Allocator & /*allocator*/, ServerResourceType /*type*/, int /*clientIndex*/ )
    {
        assert( !"override Server::CreateMessageFactory if you want to use messages" );
        return NULL;
    }

    void Server::ResetClientState( int clientIndex )
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < MaxClients );

        m_clientConnected[clientIndex] = false;
        m_clientId[clientIndex] = 0;
        m_clientAddress[clientIndex] = Address();
        m_clientData[clientIndex] = ServerClientData();
        m_clientSequence[clientIndex] = 0;

        if ( m_clientAllocator[clientIndex] )
            m_clientAllocator[clientIndex]->ClearError();

        if ( m_clientPacketFactory[clientIndex] )
            m_clientPacketFactory[clientIndex]->ClearError();

        if ( m_clientMessageFactory[clientIndex] )
            m_clientMessageFactory[clientIndex]->ClearError();

        if ( m_clientConnection[clientIndex] )
            m_clientConnection[clientIndex]->Reset();

        if ( m_clientReplayProtection[clientIndex] )
            m_clientReplayProtection[clientIndex]->Reset();
    }

    int Server::FindFreeClientIndex() const
    {
        for ( int i = 0; i < m_maxClients; ++i )
        {
            if ( !m_clientConnected[i] )
                return i;
        }
        return -1;
    }

    bool Server::FindConnectTokenEntry( const uint8_t * mac )
    {
        for ( int i = 0; i < MaxConnectTokenEntries; ++i )
        {
            if ( memcmp( mac, m_connectTokenEntries[i].mac, MacBytes ) == 0 )
                return true;
        }

        return false;
    }

    bool Server::FindOrAddConnectTokenEntry( const Address & address, const uint8_t * mac )
    {
        // find the matching entry for the token mac, and the oldest token. constant time worst case. This is intentional!

        const double time = GetTime();

        assert( address.IsValid() );

        assert( mac );

        int matchingTokenIndex = -1;
        int oldestTokenIndex = -1;
        double oldestTokenTime = 0.0;
        for ( int i = 0; i < MaxConnectTokenEntries; ++i )
        {
            if ( memcmp( mac, m_connectTokenEntries[i].mac, MacBytes ) == 0 )
            {
                matchingTokenIndex = i;
            }

            if ( oldestTokenIndex == -1 || m_connectTokenEntries[i].time < oldestTokenTime )
            {
                oldestTokenTime = m_connectTokenEntries[i].time;
                oldestTokenIndex = i;
            }
        }

        // if no entry is found with the mac, replace the oldest entry with this (mac,address,time) and return true

        assert( oldestTokenIndex != -1 );

        if ( matchingTokenIndex == -1 )
        {
            m_connectTokenEntries[oldestTokenIndex].time = time;
            m_connectTokenEntries[oldestTokenIndex].address = address;
            memcpy( m_connectTokenEntries[oldestTokenIndex].mac, mac, MacBytes );
            return true;
        }

        // if an entry is found with the same mac *and* it has the same address, return true

        assert( matchingTokenIndex >= 0 );
        assert( matchingTokenIndex < MaxConnectTokenEntries );

        if ( m_connectTokenEntries[matchingTokenIndex].address == address )
            return true;

        // otherwise an entry exists with the same mac but a different address, somebody is trying to reuse the connect token as a replay attack!

        return false;
    }

    void Server::ConnectClient( int clientIndex, const Address & clientAddress, uint64_t clientId )
    {
        assert( IsRunning() );
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        assert( m_numConnectedClients >= 0 );
        assert( m_numConnectedClients < m_maxClients );
        assert( !m_clientConnected[clientIndex] );

        const double time = GetTime();

        m_counters[SERVER_COUNTER_CLIENT_CONNECTS]++;

        m_numConnectedClients++;

        m_clientConnected[clientIndex] = true;
        m_clientId[clientIndex] = clientId;
        m_clientAddress[clientIndex] = clientAddress;

        m_clientData[clientIndex].address = clientAddress;
        m_clientData[clientIndex].clientId = clientId;
        m_clientData[clientIndex].connectTime = time;
        m_clientData[clientIndex].lastPacketSendTime = time;
        m_clientData[clientIndex].lastPacketReceiveTime = time;
        m_clientData[clientIndex].fullyConnected = false;
#if YOJIMBO_INSECURE_CONNECT
        m_clientData[clientIndex].insecure = false;
#endif // #if YOJIMBO_INSECURE_CONNECT

        m_transport->AddContextMapping( clientAddress, m_clientTransportContext[clientIndex] );

        OnClientConnect( clientIndex );

        KeepAlivePacket * keepAlivePacket = CreateKeepAlivePacket( clientIndex );

        if ( keepAlivePacket )
        {
            SendPacketToConnectedClient( clientIndex, keepAlivePacket );
        }
    }

    void Server::SendPacket( const Address & address, Packet * packet, bool immediate )
    {
        assert( IsRunning() );

        m_transport->SendPacket( address, packet, ++m_globalSequence, immediate );

        OnPacketSent( packet->GetType(), address, immediate );
    }

    void Server::SendPacketToConnectedClient( int clientIndex, Packet * packet, bool immediate )
    {
        assert( IsRunning() );
        assert( packet );
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );
        assert( m_clientConnected[clientIndex] );

        const double time = GetTime();
        
        m_clientData[clientIndex].lastPacketSendTime = time;
        
        m_transport->SendPacket( m_clientAddress[clientIndex], packet, ++m_clientSequence[clientIndex], immediate );
        
        OnPacketSent( packet->GetType(), m_clientAddress[clientIndex], immediate );
    }

    void Server::ProcessConnectionRequest( const ConnectionRequestPacket & packet, const Address & address )
    {
        assert( IsRunning() );

        if ( m_flags & SERVER_FLAG_IGNORE_CONNECTION_REQUESTS )
        {
            debug_printf( "ignored connection request: flag is set\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_BECAUSE_FLAG_IS_SET, packet, address, ConnectToken() );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_BECAUSE_FLAG_IS_SET]++;
            return;
        }

        m_counters[SERVER_COUNTER_CONNECTION_REQUEST_PACKETS_RECEIVED]++;

        uint64_t timestamp = (uint64_t) ::time( NULL );

        if ( packet.connectTokenExpireTimestamp <= timestamp )
        {
            debug_printf( "ignored connection request: connect token has expired\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_CONNECT_TOKEN_EXPIRED, packet, address, ConnectToken() );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_CONNECT_TOKEN_EXPIRED]++;
            return;
        }

        ConnectToken connectToken;
        if ( !DecryptConnectToken( packet.connectTokenData, connectToken, packet.connectTokenNonce, m_privateKey, packet.connectTokenExpireTimestamp ) )
        {
            debug_printf( "ignored connection request: failed to decrypt connect token\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_DECRYPT_CONNECT_TOKEN, packet, address, ConnectToken() );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_FAILED_TO_DECRYPT_CONNECT_TOKEN]++;
            return;
        }

        bool serverAddressInConnectTokenWhiteList = false;

        for ( int i = 0; i < connectToken.numServerAddresses; ++i )
        {
            if ( m_serverAddress == connectToken.serverAddresses[i] )
            {
                serverAddressInConnectTokenWhiteList = true;
                break;
            }
        }

        if ( !serverAddressInConnectTokenWhiteList )
        {
            debug_printf( "ignored connection request: server address not in whitelist\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_SERVER_ADDRESS_NOT_IN_WHITELIST, packet, address, connectToken );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_SERVER_ADDRESS_NOT_IN_WHITELIST]++;
            return;
        }

        if ( connectToken.clientId == 0 )
        {
            debug_printf( "ignored connection request: client id is zero\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_CLIENT_ID_IS_ZERO, packet, address, connectToken );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_CLIENT_ID_IS_ZERO]++;
            return;
        }

        if ( FindClientIndex( address ) >= 0 )
        {
            debug_printf( "ignored connection request: address already connected\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_ADDRESS_ALREADY_CONNECTED, packet, address, connectToken );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_ADDRESS_ALREADY_CONNECTED]++;
            return;
        }

        if ( FindClientIndex( connectToken.clientId ) >= 0 )
        {
            debug_printf( "ignored connection request: client id already connected\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_CLIENT_ID_ALREADY_CONNECTED, packet, address, connectToken );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_CLIENT_ID_ALREADY_CONNECTED]++;
            return;
        }

        if ( !FindConnectTokenEntry( packet.connectTokenData ) )
        {
            if ( !m_transport->AddEncryptionMapping( address, connectToken.serverToClientKey, connectToken.clientToServerKey ) )
            {
                debug_printf( "ignored connection request: failed to add encryption mapping\n" );
                OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_ADD_ENCRYPTION_MAPPING, packet, address, connectToken );
                m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_FAILED_TO_ADD_ENCRYPTION_MAPPING]++;
                return;
            }
        }

        assert( m_numConnectedClients >= 0 );
        assert( m_numConnectedClients <= m_maxClients );

        if ( m_numConnectedClients == m_maxClients )
        {
            debug_printf( "denied connection request: server is full\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_DENIED_SERVER_IS_FULL, packet, address, connectToken );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_DENIED_SERVER_IS_FULL]++;
            ConnectionDeniedPacket * connectionDeniedPacket = (ConnectionDeniedPacket*) CreateGlobalPacket( CLIENT_SERVER_PACKET_CONNECTION_DENIED );
            if ( connectionDeniedPacket )
            {
                SendPacket( address, connectionDeniedPacket );
            }
            return;
        }

        if ( !FindOrAddConnectTokenEntry( address, packet.connectTokenData ) )
        {
            debug_printf( "ignored connection request: connect token already used\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_CONNECT_TOKEN_ALREADY_USED, packet, address, connectToken );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_CONNECT_TOKEN_ALREADY_USED]++;
            return;
        }

        ChallengeToken challengeToken;
        if ( !GenerateChallengeToken( connectToken, packet.connectTokenData, challengeToken ) )
        {
            debug_printf( "ignored connection request: failed to generate challenge token\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_GENERATE_CHALLENGE_TOKEN, packet, address, connectToken );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_FAILED_TO_GENERATE_CHALLENGE_TOKEN]++;
            return;
        }

        ChallengePacket * challengePacket = (ChallengePacket*) CreateGlobalPacket( CLIENT_SERVER_PACKET_CHALLENGE );
        if ( !challengePacket )
        {
            debug_printf( "ignored connection request: failed to allocate challenge packet\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_ALLOCATE_CHALLENGE_PACKET, packet, address, connectToken );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_FAILED_TO_ALLOCATE_CHALLENGE_PACKET]++;
            return;
        }

        memcpy( challengePacket->challengeTokenNonce, (uint8_t*) &m_challengeTokenNonce, NonceBytes );

        if ( !EncryptChallengeToken( challengeToken, challengePacket->challengeTokenData, NULL, 0, challengePacket->challengeTokenNonce, m_privateKey ) )
        {
            debug_printf( "ignored connection request: failed to encrypt challenge token\n" );
            OnConnectionRequest( SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_ENCRYPT_CHALLENGE_TOKEN, packet, address, connectToken );
            m_counters[SERVER_COUNTER_CONNECTION_REQUEST_IGNORED_FAILED_TO_ENCRYPT_CHALLENGE_TOKEN]++;
            return;
        }

        m_counters[SERVER_COUNTER_CONNECTION_REQUEST_CHALLENGE_PACKETS_SENT]++;

        SendPacket( address, challengePacket );

        OnConnectionRequest( SERVER_CONNECTION_REQUEST_CHALLENGE_PACKET_SENT, packet, address, connectToken );
    }

    void Server::ProcessChallengeResponse( const ChallengeResponsePacket & packet, const Address & address )
    {
        assert( IsRunning() );

        if ( m_flags & SERVER_FLAG_IGNORE_CHALLENGE_RESPONSES )
        {
            debug_printf( "ignored challenge response: flag is set\n" );
            OnChallengeResponse( SERVER_CHALLENGE_RESPONSE_IGNORED_BECAUSE_FLAG_IS_SET, packet, address, ChallengeToken() );
            m_counters[SERVER_COUNTER_CHALLENGE_RESPONSE_IGNORED_BECAUSE_FLAG_IS_SET]++;
            return;
        }

        m_counters[SERVER_COUNTER_CHALLENGE_RESPONSE_PACKETS_RECEIVED]++;

        ChallengeToken challengeToken;
        if ( !DecryptChallengeToken( packet.challengeTokenData, challengeToken, NULL, 0, packet.challengeTokenNonce, m_privateKey ) )
        {
            debug_printf( "ignored challenge response: failed to decrypt challenge token\n" );
            OnChallengeResponse( SERVER_CHALLENGE_RESPONSE_IGNORED_FAILED_TO_DECRYPT_CHALLENGE_TOKEN, packet, address, challengeToken );
            m_counters[SERVER_COUNTER_CHALLENGE_RESPONSE_IGNORED_FAILED_TO_DECRYPT_CHALLENGE_TOKEN]++;
            return;
        }

        if ( FindClientIndex( address ) >= 0 )
        {
            debug_printf( "ignored challenge response: address already connected\n" );
            OnChallengeResponse( SERVER_CHALLENGE_RESPONSE_IGNORED_ADDRESS_ALREADY_CONNECTED, packet, address, challengeToken );
            m_counters[SERVER_COUNTER_CHALLENGE_RESPONSE_IGNORED_ADDRESS_ALREADY_CONNECTED]++;
            return;
        }

        if ( FindClientIndex( challengeToken.clientId ) >= 0 )
        {
            debug_printf( "ignored challenge response: client id already connected\n" );
            OnChallengeResponse( SERVER_CHALLENGE_RESPONSE_IGNORED_CLIENT_ID_ALREADY_CONNECTED, packet, address, challengeToken );
            m_counters[SERVER_COUNTER_CHALLENGE_RESPONSE_IGNORED_CLIENT_ID_ALREADY_CONNECTED]++;
            return;
        }

        if ( m_numConnectedClients == m_maxClients )
        {
            debug_printf( "challenge response denied: server is full\n" );
            OnChallengeResponse( SERVER_CHALLENGE_RESPONSE_DENIED_SERVER_IS_FULL, packet, address, challengeToken );
            m_counters[SERVER_COUNTER_CHALLENGE_RESPONSE_DENIED_SERVER_IS_FULL]++;

            ConnectionDeniedPacket * connectionDeniedPacket = (ConnectionDeniedPacket*) CreateGlobalPacket( CLIENT_SERVER_PACKET_CONNECTION_DENIED );

            if ( connectionDeniedPacket )
            {
                SendPacket( address, connectionDeniedPacket );
            }

            return;
        }

        const int clientIndex = FindFreeClientIndex();

        assert( clientIndex != -1 );

        debug_printf( "challenge response accepted\n" );
        OnChallengeResponse( SERVER_CHALLENGE_RESPONSE_ACCEPTED, packet, address, challengeToken );
        m_counters[SERVER_COUNTER_CHALLENGE_RESPONSE_ACCEPTED]++;

        ConnectClient( clientIndex, address, challengeToken.clientId );
    }

    void Server::ProcessKeepAlive( const KeepAlivePacket & /*packet*/, const Address & address )
    {
        assert( IsRunning() );

        const int clientIndex = FindClientIndex( address );
        if ( clientIndex == -1 )
            return;

        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );

        const double time = GetTime();
        
        m_clientData[clientIndex].lastPacketReceiveTime = time;

        m_clientData[clientIndex].fullyConnected = true;
    }

    void Server::ProcessDisconnect( const DisconnectPacket & /*packet*/, const Address & address )
    {
        assert( IsRunning() );

        const int clientIndex = FindClientIndex( address );
        if ( clientIndex == -1 )
            return;

        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );

        m_counters[SERVER_COUNTER_CLIENT_CLEAN_DISCONNECTS]++;

        DisconnectClient( clientIndex, false );
    }

#if YOJIMBO_INSECURE_CONNECT

    void Server::ProcessInsecureConnect( const InsecureConnectPacket & packet, const Address & address )
    {
        assert( IsRunning() );

        if ( ( GetFlags() & SERVER_FLAG_ALLOW_INSECURE_CONNECT ) == 0 )
            return;

        debug_printf( "Server::ProcessInsecureConnect - clientSalt = %" PRIx64 "\n", packet.clientSalt );

        if ( m_numConnectedClients == m_maxClients )
        {
            debug_printf( "insecure connect denied. server is full\n" );
            m_counters[SERVER_COUNTER_CHALLENGE_RESPONSE_DENIED_SERVER_IS_FULL]++;
            ConnectionDeniedPacket * connectionDeniedPacket = (ConnectionDeniedPacket*) CreateGlobalPacket( CLIENT_SERVER_PACKET_CONNECTION_DENIED );
            if ( connectionDeniedPacket )
            {
                SendPacket( address, connectionDeniedPacket );
            }
            return;
        }

        if ( FindClientIndex( address ) != -1 )
        {
            debug_printf( "insecure connect ignored. address already connected\n" );
            return;
        }

        if ( FindClientIndex( packet.clientId ) != -1 )
        {
            debug_printf( "insecure connect ignored. client id %" PRIx64 " already connected\n" );
            return;
        }

        const int clientIndex = FindFreeClientIndex();

        assert( clientIndex != -1 );
        if ( clientIndex == -1 )
        {
            debug_printf( "insecure connect ignored. unexpectedly not able to find free client slot, even though server is not full?!\n" );
            return;
        }

        ConnectClient( clientIndex, address, packet.clientId );

        m_clientData[clientIndex].clientSalt = packet.clientSalt;

        m_clientData[clientIndex].insecure = true;
    }

#endif // #if YOJIMBO_INSECURE_CONNECT

    void Server::ProcessConnectionPacket( ConnectionPacket & packet, const Address & address )
    {
        const int clientIndex = FindClientIndex( address );
        if ( clientIndex == -1 )
            return;

        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );

        if ( m_clientConnection[clientIndex] )
            m_clientConnection[clientIndex]->ProcessPacket( &packet );

        m_clientData[clientIndex].lastPacketReceiveTime = GetTime();

        m_clientData[clientIndex].fullyConnected = true;
    }

    void Server::ProcessPacket( Packet * packet, const Address & address, uint64_t /*sequence*/ )
    {
        OnPacketReceived( packet->GetType(), address );
        
        switch ( packet->GetType() )
        {
            case CLIENT_SERVER_PACKET_CONNECTION_REQUEST:
                ProcessConnectionRequest( *(ConnectionRequestPacket*)packet, address );
                return;

            case CLIENT_SERVER_PACKET_CHALLENGE_RESPONSE:
                ProcessChallengeResponse( *(ChallengeResponsePacket*)packet, address );
                return;

            case CLIENT_SERVER_PACKET_KEEPALIVE:
                ProcessKeepAlive( *(KeepAlivePacket*)packet, address );
                return;

            case CLIENT_SERVER_PACKET_DISCONNECT:
                ProcessDisconnect( *(DisconnectPacket*)packet, address );
                return;

#if YOJIMBO_INSECURE_CONNECT
            case CLIENT_SERVER_PACKET_INSECURE_CONNECT:
                ProcessInsecureConnect( *(InsecureConnectPacket*)packet, address );
                return;
#endif // #if YOJIMBO_INSECURE_CONNECT

            case CLIENT_SERVER_PACKET_CONNECTION:
                ProcessConnectionPacket( *(ConnectionPacket*)packet, address );
                return;

            default:
                break;
        }

        const int clientIndex = FindClientIndex( address );

        if ( clientIndex == -1 )
            return;

        m_clientData[clientIndex].fullyConnected = true;

        if ( !ProcessUserPacket( clientIndex, packet ) )
            return;

        m_clientData[clientIndex].lastPacketReceiveTime = GetTime();
    }

    KeepAlivePacket * Server::CreateKeepAlivePacket( int clientIndex )
    {
        assert( clientIndex >= 0 );
        assert( clientIndex < m_maxClients );

        assert( m_clientConnected[clientIndex] );

        KeepAlivePacket * packet = (KeepAlivePacket*) CreateClientPacket( clientIndex, CLIENT_SERVER_PACKET_KEEPALIVE );

        if ( packet )
        {
            packet->clientIndex = clientIndex;
#if YOJIMBO_INSECURE_CONNECT
            packet->clientSalt = m_clientData[clientIndex].clientSalt;
#endif // #if YOJIMBO_INSECURE_CONNECT
        }

        return packet;
    }
}
