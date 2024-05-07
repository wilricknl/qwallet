#pragma once

#include <tl/expected.hpp>

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
 * Wallet error
 */
struct WalletError
{
    std::string message;
};

// ------------------------------------------------------------------------------------------------
/**
 * Check seed upon validity
 * @param out_error_message Return a description of the error
 * @param  seed The seed to check
 * @return `true` if valid seed, else `false` 
 */
bool IsValidSeed(std::string& out_error_message, const std::string& seed);

// ------------------------------------------------------------------------------------------------
/**
 * Check seed upon validity (ignore error message)
 * @param  seed The seed to check
 * @return `true` if valid seed, else `false`
 */
bool IsValidSeed(const std::string& seed);

// ------------------------------------------------------------------------------------------------
/**
 * Generate 55-character random seed using Crypto++
 * @return The generated seed
 */
std::string GenerateSeed();

// ------------------------------------------------------------------------------------------------
/**
 * Generate a wallet with a given seed
 * @param seed 
 * @return The generated wallet
 */
tl::expected<Wallet, WalletError> GenerateWallet(const std::string& seed);

// ------------------------------------------------------------------------------------------------
/**
 * Generate a new wallet
 * @return The new wallet
 */
Wallet GenerateWallet();

// ------------------------------------------------------------------------------------------------
/**
 * Try to generate a wallet with a specific prefix
 * @param out_wallet Reference to where to save if one is found
 * @param prefix The prefix to try to find
 * @return `true` upon success, else `false`
 */
bool GenerateWalletWithPrefix(Wallet& out_wallet, const std::string& prefix);

// ------------------------------------------------------------------------------------------------
/**
 * Print identity, public key and private key belonging to the seed
 * @param seed The seed of the wallet
 */
void PrintWallet(const std::string& seed);
