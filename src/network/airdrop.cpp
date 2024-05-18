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
constexpr int QAIRDROP_PROCEDURE_TokenTransfer = 3;

bool IsValidAssetNameLength(const std::string& assetName)
{
    return !assetName.empty() && assetName.size() < 8;
}

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
    const std::string& assetName,
    const std::string& unitOfMeasurement,
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
    if (!IsValidAssetNameLength(assetName))
    {
        return tl::make_unexpected(
            AirdropError{"Asset name has an invalid length: " + std::to_string(assetName.size())});
    }

    // - - - - - - - - - - - - -
    // Check unit of measurement
    if (unitOfMeasurement.size() > 7)
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

    std::optional<StartAirdrop_output> output = std::nullopt;

    auto response = connection->ReceiveAs<StartAirdrop_output>(RespondContractFunction::type);
    if (response.has_value())
    {
        output = response.value();
    }
    // note: receiving response time-out, maybe log it

    return StartAirdropResult{output, hash, tick + tickOffset};
}

// ------------------------------------------------------------------------------------------------
tl::expected<DistributeTokenResult, AirdropError> DistributeToken(
    const ConnectionPtr& connection,
    const Wallet& issuer,
    const std::string& assetName,
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
    if (IsValidAssetNameLength(assetName))
    {
        return tl::make_unexpected(
            AirdropError{"Asset name has an invalid length: " + std::to_string(assetName.size())});
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
        DistributeToken_input input;
        unsigned char signature[64];
    } packet;

    // init header
    packet.header.setSize<sizeof(packet)>();
    packet.header.setDejavu(0);
    packet.header.setType(BROADCAST_TRANSACTION);

    // init transaction
    memcpy((void*)&packet.transaction.sourcePublicKey, senderPublicKey, 32);
    memcpy((void*)&packet.transaction.destinationPublicKey, recipientPublicKey, 32);
    packet.transaction.amount = 1000000;
    packet.transaction.tick = tick + tickOffset;
    packet.transaction.inputType = QAIRDROP_PROCEDURE_DistributeToken;
    packet.transaction.inputSize = sizeof(DistributeToken_input);

    // init distribute token input
    memcpy((void*)&packet.input.issuer, senderPublicKey, 32);
    memcpy(&packet.input.assetName, assetName.data(), assetName.size());

    // compute digest
    unsigned char digest[32] = {0};
    KangarooTwelve(
        (unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(DistributeToken_input),
        digest,
        32);

    // init signature
    unsigned char signature[64] = {0};
    sign(subseed, senderPublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);

    // Recompute digest for transaction hash
    KangarooTwelve(
        (unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(DistributeToken_input) + 64,
        digest,
        32);

    // Compute transaction hash
    char hash[61] = "";
    getIdentity(digest, hash, true);

    // - - - - - - - - - - -
    // Broadcast transaction
    if (!connection->Send((char*)&packet, packet.header.size()))
    {
        return tl::make_unexpected(
            AirdropError{"Failed to send distribution request to the network"});
    }

    std::optional<DistributeToken_output> output = std::nullopt;

    auto response = connection->ReceiveAs<DistributeToken_output>(RespondContractFunction::type);
    if (response.has_value())
    {
        output = response.value();
    }
    // note: receiving response time-out, maybe log it

    return DistributeTokenResult{output, hash, tick + tickOffset};
}

// ------------------------------------------------------------------------------------------------
tl::expected<TransferTokenResult, AirdropError> TransferToken(
    const ConnectionPtr& connection,
    const Wallet& issuer,
    const std::string& assetName,
    const std::string& newOwner,
    long long int numberOfUnits,
    uint32_t tickOffset)
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

    unsigned char newOwnerPublicKey[32];
    if (!getPublicKeyFromIdentity((const unsigned char*)newOwner.data(), newOwnerPublicKey))
    {
        return tl::make_unexpected(
            AirdropError{"Failed to compute public key from identity: " + newOwner});
    }

    // - - - - - - - - -
    // Check asset name
    if (IsValidAssetNameLength(assetName))
    {
        return tl::make_unexpected(
            AirdropError{"Asset name has an invalid length: " + std::to_string(assetName.size())});
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
        TransferToken_input input;
        unsigned char signature[64];
    } packet;

    // init header
    packet.header.setSize<sizeof(packet)>();
    packet.header.setDejavu(0);
    packet.header.setType(BROADCAST_TRANSACTION);

    // init transaction
    memcpy((void*)&packet.transaction.sourcePublicKey, senderPublicKey, 32);
    memcpy((void*)&packet.transaction.destinationPublicKey, recipientPublicKey, 32);
    packet.transaction.amount = 1000000;
    packet.transaction.tick = tick + tickOffset;
    packet.transaction.inputType = QAIRDROP_PROCEDURE_TokenTransfer;
    packet.transaction.inputSize = sizeof(TransferToken_input);

    // init input
    memcpy((void*)&packet.input.issuer, senderPublicKey, 32);
    memcpy(&packet.input.assetName, assetName.data(), assetName.size());
    packet.input.amount = numberOfUnits;
    memcpy((void*)&packet.input.receiver, newOwnerPublicKey, 32);

    // - - - - - - - -
    // compute digest
    unsigned char digest[32] = {0};
    KangarooTwelve(
        (unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(TransferToken_input),
        digest,
        32);

    // init signature
    unsigned char signature[64] = {0};
    sign(subseed, senderPublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);

    // - - - - - - - - - - - - - - - - - - -
    // Recompute digest for transaction hash
    KangarooTwelve(
        (unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(TransferToken_input) + 64,
        digest,
        32);

    // Compute transaction hash
    char hash[61] = "";
    getIdentity(digest, hash, true);

    // - - - - - - - - - - -
    // Broadcast transaction
    if (!connection->Send((char*)&packet, packet.header.size()))
    {
        return tl::make_unexpected(AirdropError{"Failed to send token transfer to the network"});
    }

    std::optional<TransferToken_output> output = std::nullopt;

    auto response = connection->ReceiveAs<TransferToken_output>(RespondContractFunction::type);
    if (response.has_value())
    {
        output = response.value();
    }
    // note: else receiving response time-out, maybe log it

    return TransferTokenResult{output, hash, tick + tickOffset};
}
