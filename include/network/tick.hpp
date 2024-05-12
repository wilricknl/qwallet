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
tl::expected<CurrentTickInfo, ConnectionError> getCurrentTickInfo(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the current tick duration
 * @param connection The node to query
 * @return The current tick duration upon success, else a connection error
 */
tl::expected<unsigned short, ConnectionError> getTickDuration(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the current epoch
 * @param connection The node to query
 * @return The current epoch upon success, else a connection error
 */
tl::expected<unsigned short, ConnectionError> getEpoch(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the current tick number
 * @param connection The node to query
 * @return The current tick number upon success, else a connection error
 */
tl::expected<unsigned int, ConnectionError> getTick(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the number of aligned votes of the current tick
 * @param connection The node to query
 * @return The number of aligned votes upon success, else a connection error
 */
tl::expected<unsigned short, ConnectionError> getNumberOfAlignedVotes(
    const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the number of misaligned votes of the current tick
 * @param connection The node to query
 * @return The number of misaligned votes upon success, else a connection error
 */
tl::expected<unsigned short, ConnectionError> getNumberOfMisalignedVotes(
    const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get the initial tick of the epoch
 * @param connection The node to query
 * @return The initial tick of the epoch upon success, else a connection error
 */
tl::expected<unsigned int, ConnectionError> getInitialTick(const ConnectionPtr& connection);

// ------------------------------------------------------------------------------------------------
/**
 * Get tick data
 * @param connection The node to query
 * @param tick The tick to request
 * @return The tick data or a connection error
 */
tl::expected<BroadcastFutureTickData, ConnectionError> getTickData(
    const ConnectionPtr& connection,
    unsigned int tick);
