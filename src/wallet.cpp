#include "wallet.hpp"

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>

#include <cstring>
#include <iostream>

#include "core/four_q.h"

// ------------------------------------------------------------------------------------------------
bool IsValidSeed(std::string& out_error_message, const std::string& seed)
{
    if (seed.length() != 55)
    {
        out_error_message = "The seed length is invalid";
        return false;
    }

    for (auto c : seed)
    {
        if (c < 'a' || c > 'z')
        {
            out_error_message = "Seed contains invalid characters";
            return false;
        }
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
bool IsValidSeed(const std::string& seed)
{
    std::string ignored_error_message;
    return IsValidSeed(ignored_error_message, seed);
}

// ------------------------------------------------------------------------------------------------
std::string GenerateSeed()
{
    std::string seed{};

    auto random_pool = CryptoPP::AutoSeededRandomPool();
    const auto min = CryptoPP::Integer(97);  // 'a'
    const auto max = CryptoPP::Integer(122); // 'z'

    for (int i = 0; i < 55; i++)
    {
        seed += static_cast<char>(CryptoPP::Integer(random_pool, min, max).ConvertToLong());
    }

    return seed;
}

// ------------------------------------------------------------------------------------------------
tl::expected<Wallet, WalletError> GenerateWallet(const std::string& seed)
{
    std::string error_message;
    if (!IsValidSeed(error_message, seed))
    {
        return tl::make_unexpected(WalletError{error_message});
    }

    uint8_t private_key[32] = {0};
    uint8_t public_key[32] = {0};
    uint8_t subseed[32] = {0};

    char private_key_ascii[61] = {0};
    char public_key_ascii[61] = {0};
    char public_identity[61] = {0};

    getSubseed((const uint8_t*)seed.data(), subseed);
    getPrivateKey(subseed, private_key);
    getPublicKey(private_key, public_key);

    getIdentity(private_key, private_key_ascii, true);
    getIdentity(public_key, public_key_ascii, true);
    getIdentity(public_key, public_identity, false);

    return Wallet{seed, public_key_ascii, private_key_ascii, public_identity};
}

// ------------------------------------------------------------------------------------------------
Wallet GenerateWallet() { return GenerateWallet(GenerateSeed()).value(); }

// ------------------------------------------------------------------------------------------------
bool GenerateWalletWithPrefix(Wallet& out_wallet, const std::string& prefix)
{
    auto seed = GenerateSeed();

    uint8_t private_key[32] = {0};
    uint8_t public_key[32] = {0};
    uint8_t subseed[32] = {0};

    getSubseed((const uint8_t*)seed.data(), subseed);
    getPrivateKey(subseed, private_key);
    getPublicKey(private_key, public_key);

    char public_identity[61] = {0};
    getIdentity(public_key, public_identity, false);

    // check prefix
    if (memcmp(public_identity, prefix.c_str(), prefix.length()) != 0)
    {
        return false;
    }

    // compute other identities
    char private_key_ascii[61] = {0};
    char public_key_ascii[61] = {0};
    getIdentity(private_key, private_key_ascii, true);
    getIdentity(public_key, public_key_ascii, true);

    out_wallet = Wallet{seed, public_key_ascii, private_key_ascii, public_identity};

    return true;
}

// ------------------------------------------------------------------------------------------------
void PrintWallet(const std::string& seed)
{
    uint8_t privateKey[32] = {0};
    uint8_t publicKey[32] = {0};
    uint8_t subseed[32] = {0};

    char privateKeyQubicFormat[61] = {0};
    char publicKeyQubicFormat[61] = {0};
    char publicIdentity[61] = {0};

    if (!getSubseed((const uint8_t*)seed.data(), subseed))
    {
        std::cout << "Invalid seed: " << seed << std::endl;
        return;
    }

    getPrivateKey(subseed, privateKey);
    getPublicKey(privateKey, publicKey);

    getIdentity(publicKey, publicIdentity, false);
    getIdentity(privateKey, privateKeyQubicFormat, true);
    getIdentity(publicKey, publicKeyQubicFormat, true);

    std::cout << "Seed: " << seed << std::endl;
    std::cout << "Private key: " << privateKeyQubicFormat << std::endl;
    std::cout << "Public key: " << publicKeyQubicFormat << std::endl;
    std::cout << "Identity: " << publicIdentity << std::endl;
}
