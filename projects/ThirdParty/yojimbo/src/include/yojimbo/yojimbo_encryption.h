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

#ifndef YOJIMBO_ENCRYPTION_H
#define YOJIMBO_ENCRYPTION_H

#include "yojimbo_config.h"
#include "yojimbo_network.h"
#include "yojimbo_replay_protection.h"

#include <stdint.h>

namespace yojimbo
{
    extern void GenerateKey( uint8_t * key );

    extern void RandomBytes( uint8_t * data, int bytes );

    extern bool Encrypt( const uint8_t * message, int messageLength, 
                         uint8_t * encryptedMessage, int & encryptedMessageLength, 
                         const uint8_t * nonce, const uint8_t * key );
 
    extern bool Decrypt( const uint8_t * encryptedMessage, int encryptedMessageLength, 
                         uint8_t * decryptedMessage, int & decryptedMessageLength, 
                         const uint8_t * nonce, const uint8_t * key );

    extern bool Encrypt_AEAD( const uint8_t * message, uint64_t messageLength, 
                              uint8_t * encryptedMessage, uint64_t & encryptedMessageLength,
                              const uint8_t * additional, uint64_t additionalLength,
                              const uint8_t * nonce,
                              const uint8_t * key );

    extern bool Decrypt_AEAD( const uint8_t * encryptedMessage, uint64_t encryptedMessageLength, 
                              uint8_t * decryptedMessage, uint64_t & decryptedMessageLength,
                              const uint8_t * additional, uint64_t additionalLength,
                              const uint8_t * nonce,
                              const uint8_t * key );

    class EncryptionManager
    {
    public:

        EncryptionManager();

        bool AddEncryptionMapping( const Address & address, const uint8_t * sendKey, const uint8_t * receiveKey, double time );

        bool RemoveEncryptionMapping( const Address & address, double time );

        void ResetEncryptionMappings();

        int FindEncryptionMapping( const Address & address, double time );

        const uint8_t * GetSendKey( int index ) const ;

        const uint8_t * GetReceiveKey( int index ) const;

    private:

        int m_numEncryptionMappings;

        double m_encryptionMappingTimeout;

        double m_lastAccessTime[MaxEncryptionMappings];
        
        Address m_address[MaxEncryptionMappings];
        
        uint8_t m_sendKey[KeyBytes*MaxEncryptionMappings];
        
        uint8_t m_receiveKey[KeyBytes*MaxEncryptionMappings];
    };
}

#endif // #ifndef YOJIMBO_ENCRYPTION_H
