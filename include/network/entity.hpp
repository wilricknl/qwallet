#pragma once

#include <tl/expected.hpp>

#include "network/connection.hpp"
#include "network_messages/entity.h"

// ------------------------------------------------------------------------------------------------
/**
 * Query entity
 * @param connection The node to query
 * @param identity The identity of the entity to query
 * @return The entity or an error (yay)
 */
tl::expected<RespondedEntity, ConnectionError> GetEntity(
    const ConnectionPtr& connection,
    const std::string& identity);

// ------------------------------------------------------------------------------------------------
/**
 * Get balance of an entity
 * @param connection The node to query
 * @param identity The identity of the entity to query
 * @return The balance or an error
 */
tl::expected<unsigned long long, ConnectionError> GetBalance(
    const ConnectionPtr& connection,
    const std::string& identity);
