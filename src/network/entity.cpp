#include "network/entity.hpp"

#include "core/four_q.h"

// ------------------------------------------------------------------------------------------------
tl::expected<RespondedEntity, ConnectionError> GetEntity(
    const ConnectionPtr& connection,
    const std::string& identity)
{
    // Check identity length
    if (identity.size() != 60)
    {
        return tl::make_unexpected(ConnectionError{"Invalid identity with invalid length: " + identity});
    }

    // Construct packet
    struct
    {
        RequestResponseHeader header;
        RequestedEntity request;
    } packet;

    // Init header
    packet.header.setSize<sizeof(packet)>();
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_ENTITY);

    // Init public key
    unsigned char public_key[32];
    if (!getPublicKeyFromIdentity((const unsigned char*)identity.data(), public_key))
    {
        return tl::make_unexpected(ConnectionError{"Failed to compute public key from identity: " + identity});
    }
    memcpy(&packet.request.publicKey, public_key, 32);

    // Send request
    if (!connection->Send((char*)&packet, sizeof(packet)))
    {
        return tl::make_unexpected(ConnectionError{"Failed to send identity request"});
    }

    // Return response
    return connection->ReceiveAs<RespondedEntity>(RESPOND_ENTITY);
}

// ------------------------------------------------------------------------------------------------
tl::expected<unsigned long long, ConnectionError> GetBalance(
    const ConnectionPtr& connection,
    const std::string& identity)
{
    auto response = GetEntity(connection, identity);
    if (response.has_value())
    {
        auto entity = response.value();
        // todo: do i really have to check that incoming is > than outgoing?
        return entity.entity.incomingAmount - entity.entity.outgoingAmount;
    }
    return tl::make_unexpected(ConnectionError{"Get balance failed: " + response.error().message});
}
