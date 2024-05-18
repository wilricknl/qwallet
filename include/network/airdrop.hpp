#pragma once

#include <tl/expected.hpp>

#include <cstdint>
#include <optional>
#include <string>

#include "network/connection.hpp"
#include "wallet.hpp"

// ------------------------------------------------------------------------------------------------
struct AirdropError
{
    std::string message;
};

// ------------------------------------------------------------------------------------------------
struct Fees_output
{
    uint32_t airdropStartFee{}; // Amount of qus
    uint32_t transferFee{};     // Amount of qus
};

// ------------------------------------------------------------------------------------------------
struct StartAirdrop_input
{
    uint64_t assetName{};
    int64_t numberOfShares{};
    int8_t numberOfDecimalPlaces{};
    uint64_t unitOfMeasurement{};
};

// ------------------------------------------------------------------------------------------------
struct StartAirdrop_output
{
    int64_t issuedNumberOfShares{};
};

// ------------------------------------------------------------------------------------------------
struct DistributeToken_input
{
    uint8_t issuer[32]{};
    uint64_t assetName{};
};

// ------------------------------------------------------------------------------------------------
struct DistributeToken_output
{
    int64_t transferredAmount{};
};

// ------------------------------------------------------------------------------------------------
struct TransferToken_input
{
    uint8_t issuer[32]{};
    uint64_t assetName{};
    uint64_t amount{};
    uint8_t receiver[32]{};
};

// ------------------------------------------------------------------------------------------------
struct TransferToken_output
{
    int64_t transferredAmount{};
};

// ------------------------------------------------------------------------------------------------
/**
 * Always return hash and tick - and function output if no time-out
 * @tparam T Output type of Smart Contract function
 */
template<typename T>
struct AirdropResult
{
    std::optional<T> output;
    std::string hash;
    unsigned int tick;
};
typedef AirdropResult<StartAirdrop_output> StartAirdropResult;
typedef AirdropResult<DistributeToken_output> DistributeTokenResult;
typedef AirdropResult<TransferToken_output> TransferTokenResult;

// ------------------------------------------------------------------------------------------------
/**
 * Get the airdrop fees
 * @param connection The node to request the fees from
 * @return The fees or an error
 */
tl::expected<Fees_output, AirdropError> GetAirdropFees(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Start airdrop
 * @param connection The node to start the airdrop
 * @param issuer The wallet to issue the airdrop
 * @param assetName The name of the asset to airdrop (max 7 characters)
 * @param unitOfMeasurement The unit of measurement (digits only in string)
 * @param numberOfUnits The amount of tokens that should be created
 * @param numberOfDecimalPlaces The number of decimal places
 * @param tickOffset The offset of the tick to issue at
 * @return The result of the airdrop start or an error
 */
tl::expected<StartAirdropResult, AirdropError> StartAirdrop(
    const ConnectionPtr& connection,
    const Wallet& issuer,
    const std::string& assetName,
    const std::string& unitOfMeasurement,
    int64_t numberOfUnits,
    char numberOfDecimalPlaces,
    unsigned int tickOffset);

// ------------------------------------------------------------------------------------------------
/**
 * Distribute token
 * @param connection The node to start the airdrop
 * @param issuer The wallet to issue the airdrop
 * @param assetName The name of the asset to airdrop (max 7 characters)
 * @param tickOffset The offset of the tick to issue at
 * @return The result of the distribution or an error
 */
tl::expected<DistributeTokenResult, AirdropError> DistributeToken(
    const ConnectionPtr& connection,
    const Wallet& issuer,
    const std::string& assetName,
    unsigned int tickOffset);

// ------------------------------------------------------------------------------------------------
/**
 * Distribute token
 * @param connection The node to start the airdrop
 * @param issuer The wallet to issue the airdrop
 * @param assetName The name of the asset to airdrop (max 7 characters)
 * @param tick The tick to issue at
 * @return The result of the distribution or an error
 */
tl::expected<DistributeTokenResult, AirdropError> DistributeTokenAtTick(
    const ConnectionPtr& connection,
    const Wallet& issuer,
    const std::string& assetName,
    unsigned int tick);

// ------------------------------------------------------------------------------------------------
/**
 * Transfer airdrop tokens
 * @param connection The node to start the airdrop
 * @param issuer The wallet to issue the airdrop
 * @param assetName The name of the asset to airdrop (max 7 characters)
 * @param newOwner The identity of the wallet to receive the tokens
 * @param numberOfUnits The number of tokens to transfer
 * @param tickOffset The offset of the tick to issue at
 */
tl::expected<TransferTokenResult, AirdropError> TransferToken(
    const ConnectionPtr& connection,
    const Wallet& issuer,
    const std::string& assetName,
    const std::string& newOwner,
    long long numberOfUnits,
    uint32_t tickOffset);
