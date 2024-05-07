#pragma once

#include <string>

// ------------------------------------------------------------------------------------------------
/**
 * Convert signed integer to comma separated string
 * @param amount The amount to convert to string
 * @return The comma separated string representation 
 */
std::string to_comma_separated_string(long long amount);

// ------------------------------------------------------------------------------------------------
/**
 * Convert unsigned integer to comma separated string
 * @param amount The amount to convert to string
 * @return The comma separated string representation
 */
std::string to_comma_separated_string(unsigned long long amount);
