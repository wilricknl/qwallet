#pragma once

#include <string>

// ------------------------------------------------------------------------------------------------
/**
 * Convert signed integer to comma separated string
 * @param amount The amount to convert to string
 * @return The comma separated string representation 
 */
std::string ToCommaSeparatedString(long long amount);

// ------------------------------------------------------------------------------------------------
/**
 * Convert unsigned integer to comma separated string
 * @param amount The amount to convert to string
 * @return The comma separated string representation
 */
std::string ToCommaSeparatedString(unsigned long long amount);
