#include "wallet.hpp"

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>

#include <cstring>
#include <iostream>

#include "core/four_q.h"

// ------------------------------------------------------------------------------------------------
bool IsValidSeed(std::string& outErrorMessage, const std::string& seed)
{
    if (seed.length() != 55)
    {
        outErrorMessage = "The seed length is invalid";
        return false;
    }

    for (auto c : seed)
    {
        if (c < 'a' || c > 'z')
        {
            outErrorMessage = "Seed contains invalid characters";
            return false;
        }
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
bool IsValidSeed(const std::string& seed)
{
    std::string ignoredErrorMessage;
    return IsValidSeed(ignoredErrorMessage, seed);
}

// ------------------------------------------------------------------------------------------------
std::string GenerateSeed()
{
    std::string seed{};

    auto randomPool = CryptoPP::AutoSeededRandomPool();
    const auto min = CryptoPP::Integer(97);  // 'a'
    const auto max = CryptoPP::Integer(122); // 'z'

    for (int i = 0; i < 55; i++)
    {
        seed += static_cast<char>(CryptoPP::Integer(randomPool, min, max).ConvertToLong());
    }

    return seed;
}

// ------------------------------------------------------------------------------------------------
tl::expected<Wallet, WalletError> GenerateWallet(const std::string& seed)
{
    std::string errorMessage;
    if (!IsValidSeed(errorMessage, seed))
    {
        return tl::make_unexpected(WalletError{errorMessage});
    }

    uint8_t privateKey[32] = {0};
    uint8_t publicKey[32] = {0};
    uint8_t subseed[32] = {0};

    char privateKeyAscii[61] = {0};
    char publicKeyAscii[61] = {0};
    char publicIdentity[61] = {0};

    getSubseed((const uint8_t*)seed.data(), subseed);
    getPrivateKey(subseed, privateKey);
    getPublicKey(privateKey, publicKey);

    getIdentity(privateKey, privateKeyAscii, true);
    getIdentity(publicKey, publicKeyAscii, true);
    getIdentity(publicKey, publicIdentity, false);

    return Wallet{seed, publicKeyAscii, privateKeyAscii, publicIdentity};
}

// ------------------------------------------------------------------------------------------------
Wallet GenerateWallet() { return GenerateWallet(GenerateSeed()).value(); }

// ------------------------------------------------------------------------------------------------
bool GenerateWalletWithPrefix(Wallet& out_wallet, const std::string& prefix)
{
    auto seed = GenerateSeed();

    uint8_t privateKey[32] = {0};
    uint8_t publicKey[32] = {0};
    uint8_t subseed[32] = {0};

    getSubseed((const uint8_t*)seed.data(), subseed);
    getPrivateKey(subseed, privateKey);
    getPublicKey(privateKey, publicKey);

    char publicIdentity[61] = {0};
    getIdentity(publicKey, publicIdentity, false);

    // check prefix
    if (memcmp(publicIdentity, prefix.c_str(), prefix.length()) != 0)
    {
        return false;
    }

    // compute other identities
    char privateKeyAscii[61] = {0};
    char publicKeyAscii[61] = {0};
    getIdentity(privateKey, privateKeyAscii, true);
    getIdentity(publicKey, publicKeyAscii, true);

    out_wallet = Wallet{seed, publicKeyAscii, privateKeyAscii, publicIdentity};

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
