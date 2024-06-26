#pragma once

#include <tl/expected.hpp>

#include "network/connection.hpp"
#include "network_messages/tick.h"

// ------------------------------------------------------------------------------------------------
/**
 * Get current tick info from a node
 * @param connection The node to query
 * @return The tick info upon success, else a connection error
 */
tl::expected<CurrentTickInfo, ConnectionError> GetCurrentTickInfo(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the current tick duration
 * @param connection The node to query
 * @return The current tick duration upon success, else a connection error
 */
tl::expected<unsigned short, ConnectionError> GetTickDuration(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the current epoch
 * @param connection The node to query
 * @return The current epoch upon success, else a connection error
 */
tl::expected<unsigned short, ConnectionError> GetEpoch(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the current tick number
 * @param connection The node to query
 * @return The current tick number upon success, else a connection error
 */
tl::expected<unsigned int, ConnectionError> GetTick(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the number of aligned votes of the current tick
 * @param connection The node to query
 * @return The number of aligned votes upon success, else a connection error
 */
tl::expected<unsigned short, ConnectionError> GetNumberOfAlignedVotes(
    const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the number of misaligned votes of the current tick
 * @param connection The node to query
 * @return The number of misaligned votes upon success, else a connection error
 */
tl::expected<unsigned short, ConnectionError> GetNumberOfMisalignedVotes(
    const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the initial tick of the epoch
 * @param connection The node to query
 * @return The initial tick of the epoch upon success, else a connection error
 */
tl::expected<unsigned int, ConnectionError> GetInitialTick(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get tick data
 * @param connection The node to query
 * @param tick The tick to request
 * @return The tick data or a connection error
 */
tl::expected<BroadcastFutureTickData, ConnectionError> GetTickData(
    const ConnectionPtr& connection,
    unsigned int tick);

// ------------------------------------------------------------------------------------------------
/**
 * Check if a tick contains a specific transaction
 * @param data The tick data to check
 * @param hash The hash of the transaction
 * @return `true` if the tick contains the transaction, else `false`
 */
bool ContainsTransaction(const TickData& data, const std::string& hash);
