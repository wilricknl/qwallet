#pragma once

#include <string>

// ------------------------------------------------------------------------------------------------
/**
 * Wallet
 */
struct Wallet
{
    std::string seed;
    std::string public_key;
    std::string private_key;
    std::string identity;
};

// ------------------------------------------------------------------------------------------------
/**
 * Generate 55-character random seed using Crypto++
 * @return The generated seed
 */
std::string GenerateSeed();

// ------------------------------------------------------------------------------------------------
/**
 * Generate a new wallet
 * @return The new wallet
 */
Wallet GenerateWallet();

// ------------------------------------------------------------------------------------------------
/**
 * Print identity, public key and private key belonging to the seed
 * @param seed The seed of the wallet
 */
void PrintWallet(const std::string& seed);
