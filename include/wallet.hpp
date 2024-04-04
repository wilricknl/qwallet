#pragma once

#include <string>

// ------------------------------------------------------------------------------------------------
/**
 * Generate 55-character random seed using Crypto++
 * @return The generated seed
 */
std::string generateSeed();

// ------------------------------------------------------------------------------------------------
/**
 * Print identity, public key and private key belonging to the seed
 * @param seed The seed of the wallet
 */
void printWallet(const std::string& seed);
