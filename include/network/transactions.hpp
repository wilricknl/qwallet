#pragma once

#include <tl/expected.hpp>

#include <string>

#include "network/connection.hpp"
#include "wallet.hpp"

// ------------------------------------------------------------------------------------------------
/**
 * Receipt of a transaction
 */
struct Receipt
{
    /// Identity of the sender
    std::string sender;

    /// Identity of the recipient
    std::string recipient;

    /// Hash of the transaction
    std::string hash;

    /// The amount to transfer
    long long amount;

    /// The tick at which the transaction should be confirmed
    unsigned int tick;

    /// The current status of the transaction
    enum Status
    {
        Confirming,
        Success,
        Failed
    } status;
};

// ------------------------------------------------------------------------------------------------
/**
 * Convert status to a string
 * @param status The status of a receipt
 * @return String representation
 */
std::string StatusToString(Receipt::Status status);

// ------------------------------------------------------------------------------------------------
/**
 * Struct to wrap transaction error mesasge
 */
struct TransactionError
{
    std::string message;
};

// ------------------------------------------------------------------------------------------------
/**
 * Broadcast a transaction to the Qubic network
 * @param connection The node to broadcast to
 * @param wallet The wallet of the sender
 * @param recipient The identity of the recipient
 * @param amount The amount to send
 * @param tickOffset The number of ticks in the future to schedule the transaction
 * @return The receipt upon success, else the error that occurred
 */
tl::expected<Receipt, TransactionError> BroadcastTransaction(
    const ConnectionPtr& connection,
    const Wallet& wallet,
    const std::string& recipient,
    long long amount,
    unsigned int tickOffset);
