#include "network/transactions.hpp"

#include "core/four_q.h"
#include "network/tick.hpp"
#include "network_messages/transactions.h"

// ------------------------------------------------------------------------------------------------
std::string StatusToString(Receipt::Status status)
{

    if (status == Receipt::Confirming)
    {
        return "confirming";
    }
    if (status == Receipt::Success)
    {
        return "success";
    }
    return "failed";
}

// ------------------------------------------------------------------------------------------------
tl::expected<Receipt, ConnectionError> BroadcastTransaction(
    const ConnectionPtr& connection,
    const Wallet& wallet,
    const std::string& recipient,
    long long amount,
    unsigned int tickOffset)
{
    // Compute public keys from identity
    uint8_t subseed[32] = {0};
    if (!getSubseed((const unsigned char*)wallet.seed.data(), subseed))
    {
        return tl::make_unexpected(
            ConnectionError{"Failed to compute subseed from seed: " + wallet.seed});
    }

    unsigned char senderPublicKey[32];
    if (!getPublicKeyFromIdentity((const unsigned char*)wallet.identity.data(), senderPublicKey))
    {
        return tl::make_unexpected(
            ConnectionError{"Failed to compute public key from identity: " + wallet.identity});
    }

    unsigned char recipientPublicKey[32];
    if (!getPublicKeyFromIdentity((const unsigned char*)recipient.data(), recipientPublicKey))
    {
        return tl::make_unexpected(
            ConnectionError{"Failed to compute public key from identity: " + recipient});
    }

    // Get current tick
    unsigned int tick = 0;
    {
        auto response = GetTick(connection);
        if (response.has_value())
        {
            tick = response.value();
        }
        else
        {
            return tl::make_unexpected(response.error());
        }
    }

    // Create transaction
    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        unsigned char signature[64];
    } packet;

    // Init header
    packet.header.setSize<sizeof(packet.header) + sizeof(packet.transaction) + 64>();
    packet.header.setDejavu(0);
    packet.header.setType(BROADCAST_TRANSACTION);

    // Init transaction
    memcpy((void*)&packet.transaction.sourcePublicKey, senderPublicKey, 32);
    memcpy((void*)&packet.transaction.destinationPublicKey, recipientPublicKey, 32);
    packet.transaction.amount = amount;
    packet.transaction.tick = tick + tickOffset;
    packet.transaction.inputType = 0;
    packet.transaction.inputSize = 0;
    unsigned char digest[32] = {0};
    KangarooTwelve((unsigned char*)&packet.transaction, sizeof(packet.transaction), digest, 32);

    // Init signature
    unsigned char signature[64] = {0};
    sign(subseed, senderPublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);

    // Broadcast transaction
    if (!connection->Send((char*)&packet, packet.header.size()))
    {
        // todo: maybe interesting to log ip+port, but that is not stored in connection
        return tl::make_unexpected(ConnectionError{"Failed to send transaction to the network"});
    }

    // Recompute digest for transaction hash
    KangarooTwelve(
        (unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + 64,
        digest,
        32);

    // Compute transaction hash
    char hash[61] = "";
    getIdentity(digest, hash, true);

    // Return receipt
    return Receipt{
        wallet.identity,
        recipient,
        hash,
        amount,
        tick + tickOffset,
        Receipt::Confirming};
}
