#pragma once

#include <tl/expected.hpp>

#include <cstdint>
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
    uint32_t airdropStartFee; // Amount of qus
    uint32_t transferFee;     // Amount of qus
};

// ------------------------------------------------------------------------------------------------
struct StartAirdrop_input
{
    uint64_t assetName;
    int64_t numberOfShares;
    int8_t numberOfDecimalPlaces;
    uint64_t unitOfMeasurement;
};

// ------------------------------------------------------------------------------------------------
struct StartAirdrop_output
{
    int64_t issuedNumberOfShares;
};

// ------------------------------------------------------------------------------------------------
struct StartAirdropResult
{
    StartAirdrop_output output;
    std::string hash;
    unsigned int tick;
};

// ------------------------------------------------------------------------------------------------
struct DistributeToken_input
{
    uint8_t issuer[32];
    uint64_t assetName;
};

// ------------------------------------------------------------------------------------------------
struct DistributeToken_output
{
    int64_t transferredAmount;
};

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
 */
tl::expected<StartAirdropResult, AirdropError> StartAirdrop(
    const ConnectionPtr& connection,
    const Wallet& issuer,
    std::string assetName,
    std::string unitOfMeasurement,
    int64_t numberOfUnits,
    char numberOfDecimalPlaces,
    unsigned int tickOffset);
