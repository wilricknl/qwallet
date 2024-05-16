#pragma once

#include <tl/expected.hpp>

#include <cstdint>
#include <string>

#include "network/connection.hpp"

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
