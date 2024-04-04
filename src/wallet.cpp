#include "wallet.hpp"

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>

#include "core/four_q.h"

// ------------------------------------------------------------------------------------------------
std::string generateSeed()
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
void printWallet(const std::string& seed)
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
