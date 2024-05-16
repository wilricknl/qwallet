#include "network/airdrop.hpp"

#include "core/four_q.h"
#include "network/tick.hpp"
#include "network_messages/contract.h"
#include "network_messages/transactions.h"

// ------------------------------------------------------------------------------------------------
namespace
{

constexpr int QAIRDROP_CONTRACT_ID = 5;
constexpr int QAIRDROP_FUNCTION_Fees = 1;
constexpr int QAIRDROP_PROCEDURE_StartAirdrop = 1;
constexpr int QAIRDROP_PROCEDURE_DistributeToken = 2;

} // namespace
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
tl::expected<Fees_output, AirdropError> GetAirdropFees(const ConnectionPtr& connection)
{
    // construct packet
    struct
    {
        RequestResponseHeader header;
        RequestContractFunction payload;
    } packet;

    // init header
    packet.header.setSize<sizeof(packet)>();
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type);

    // init payload
    packet.payload.inputSize = 0;
    packet.payload.inputType = QAIRDROP_FUNCTION_Fees;
    packet.payload.contractIndex = QAIRDROP_CONTRACT_ID;

    // request airdrop fees
    if (!connection->Send((char*)&packet, packet.header.size()))
    {
        return tl::make_unexpected(AirdropError{"Failed to request airdrop fees"});
    }

    // receive response
    auto response = connection->ReceiveAs<Fees_output>(RespondContractFunction::type);
    if (response.has_value())
    {
        return response.value();
    }

    return tl::make_unexpected(AirdropError{response.error().message});
}

// ------------------------------------------------------------------------------------------------
tl::expected<StartAirdropResult, AirdropError> StartAirdrop(
    const ConnectionPtr& connection,
    const Wallet& issuer,
    std::string assetName,
    std::string unitOfMeasurement,
    int64_t numberOfUnits,
    char numberOfDecimalPlaces,
    unsigned int tickOffset)
{
    // - - - - - - - - - - - - - - - - -
    // Compute public keys from identity
    uint8_t subseed[32] = {0};
    if (!getSubseed((const unsigned char*)issuer.seed.data(), subseed))
    {
        return tl::make_unexpected(
            AirdropError{"Failed to compute subseed from seed: " + issuer.seed});
    }

    unsigned char senderPublicKey[32];
    if (!getPublicKeyFromIdentity((const unsigned char*)issuer.identity.data(), senderPublicKey))
    {
        return tl::make_unexpected(
            AirdropError{"Failed to compute public key from identity: " + issuer.identity});
    }

    unsigned char recipientPublicKey[32]{0};
    ((int*)recipientPublicKey)[0] = QAIRDROP_CONTRACT_ID;

    // - - - - - - - - -
    // Check asset name
    if (assetName.empty() || assetName.size() > 7)
    {
        return tl::make_unexpected(
            AirdropError{"Asset name has an invalid length: " + std::to_string(assetName.size())});
    }

    // - - - - - - - - - - - - -
    // Check unit of measurement
    if (unitOfMeasurement.empty() || unitOfMeasurement.size() > 7)
    {
        return tl::make_unexpected(AirdropError{
            "Unit of measurement has an invalid length: " + std::to_string(assetName.size())});
    }

    // - - - - - - - - -
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
            return tl::make_unexpected(AirdropError{response.error().message});
        }
    }

    // - - - - - - - - -
    // Construct packet
    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        StartAirdrop_input input;
        unsigned char signature[64];
    } packet;

    // init header
    packet.header.setSize<sizeof(packet)>();
    packet.header.setDejavu(0);
    packet.header.setType(BROADCAST_TRANSACTION);

    // init transaction
    memcpy((void*)&packet.transaction.sourcePublicKey, senderPublicKey, 32);
    memcpy((void*)&packet.transaction.destinationPublicKey, recipientPublicKey, 32);
    packet.transaction.amount = 1000000000;
    packet.transaction.tick = tick + tickOffset;
    packet.transaction.inputType = QAIRDROP_PROCEDURE_StartAirdrop;
    packet.transaction.inputSize = sizeof(StartAirdrop_input);

    // compute unit of measurement
    char measurement[8]{0};
    for (size_t i = 0; i < unitOfMeasurement.size(); ++i)
    {
        measurement[i] = static_cast<char>(unitOfMeasurement.at(i) - '0');
    }

    // init input
    memcpy(&packet.input.assetName, assetName.data(), assetName.size());
    memcpy(&packet.input.unitOfMeasurement, measurement, 8);
    packet.input.numberOfShares = numberOfUnits;
    packet.input.numberOfDecimalPlaces = numberOfDecimalPlaces;

    // compute digest
    unsigned char digest[32] = {0};
    KangarooTwelve(
        (unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(StartAirdrop_input),
        digest,
        32);

    // init signature
    unsigned char signature[64] = {0};
    sign(subseed, senderPublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);

    // - - - - - - - - - - -
    // Broadcast transaction
    if (!connection->Send((char*)&packet, packet.header.size()))
    {
        return tl::make_unexpected(AirdropError{"Failed to send transaction to the network"});
    }

    // Recompute digest for transaction hash
    KangarooTwelve(
        (unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(StartAirdrop_input) + 64,
        digest,
        32);

    // Compute transaction hash
    char hash[61] = "";
    getIdentity(digest, hash, true);

    // todo: this might take a while before it arrives, so it might time out before it is received
    auto response = connection->ReceiveAs<StartAirdrop_output>(RespondContractFunction::type);
    if (!response.has_value())
    {
        return tl::make_unexpected(AirdropError{
            "Failed to receive StartAirdrop_output (hash: " + std::string{hash} +
            ", tick: " + std::to_string(tick + tickOffset) + "): " + response.error().message});
    }

    return StartAirdropResult{response.value(), hash, tick + tickOffset};
}
