#include <iostream>

#include "network/airdrop.hpp"
#include "network/connection.hpp"
#include "network/tick.hpp"

// ------------------------------------------------------------------------------------------------
namespace g
{

static std::vector<std::string> seeds{
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

constexpr auto seed = "";
constexpr auto ipAddress = "57.129.19.155";
constexpr unsigned short port = 31843;
constexpr auto assetName = "QTEST";
constexpr int tickOffset = 100;
constexpr long long numberOfUnits = 1024LL * 600'000LL;
} // namespace g

// ------------------------------------------------------------------------------------------------
void GetTickTest(const ConnectionPtr& connection);
void RequestFeesTest(const ConnectionPtr& connection);
void StartAirdropTest(const ConnectionPtr& connection, const Wallet& wallet);
void DistributeTokenTest(const ConnectionPtr& connection, const Wallet& wallet);
void TokenTransferTest(
    const ConnectionPtr& connection,
    const Wallet& wallet,
    std::string recipientIdentity);

std::vector<Wallet> GenerateWallets(const std::vector<std::string>& seeds);
void StartAirdrop(const ConnectionPtr& connection, const std::vector<Wallet>& wallets);
void DistributeTokenAtTickTest(
    const ConnectionPtr& connection,
    const std::vector<Wallet>& wallets,
    unsigned int tick);

// ------------------------------------------------------------------------------------------------
/**
 * QAirdrop test
 */
int main()
{
    InitializeConnection();

    // setup connection with test server
    auto result = CreateConnection(g::ipAddress, g::port);
    if (!result.has_value())
    {
        std::cout << "Failed to create connection: " << result.error().message << std::endl;
        return 1;
    }
    auto connection = result.value();

    

    std::vector<Wallet> wallets = GenerateWallets(g::seeds);

    /**
     * Airdrop multiple tokens for testing
     */
    // StartAirdrop(connection, wallets);

    /**
     * Distribute multiple tokens at the same tick
     */
    auto tick = GetTick(connection);
    if (!tick.has_value())
    {
        std::cout << "Failed to get latest tick: " << tick.error().message << std::endl;
        return 1;
    }
    DistributeTokenAtTickTest(connection, wallets, tick.value() + g::tickOffset);

    DestroyConnection();

    return 0;
}

// ------------------------------------------------------------------------------------------------
void GetTickTest(const ConnectionPtr& connection)
{
    if (auto result = GetTick(connection); result.has_value())
    {
        std::cout << "Tick: " << result.value() << std::endl;
    }
    else
    {
        std::cout << "error: " << result.error().message << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------
void RequestFeesTest(const ConnectionPtr& connection)
{
    auto response = GetAirdropFees(connection);
    if (response)
    {
        auto fees = response.value();
        std::cout << "Qairdrop fees:" << std::endl;
        std::cout << "\tStart fee: " << fees.airdropStartFee << std::endl;
        std::cout << "\tTransfer fee: " << fees.transferFee << std::endl;
    }
    else
    {
        std::cout << "Failed to request Qairdrop fees" << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------
void StartAirdropTest(
    const ConnectionPtr& connection,
    const Wallet& wallet,
    uint64_t numberOfUnits = 1024LL * 600'000LL)
{
    auto airdrop =
        StartAirdrop(connection, wallet, g::assetName, "0000000", numberOfUnits, '\0', 5);
    if (airdrop.has_value())
    {
        std::cout << "Succesfully created airdrop" << std::endl;
        std::cout << "\tIssuer: " << wallet.identity << std::endl;
        std::cout << "\tHash: " << airdrop->hash << std::endl;
        std::cout << "\tTick: " << airdrop->tick << std::endl;
        if (airdrop->output)
        {
            std::cout << "\tIssued shares: " << airdrop->output->issuedNumberOfShares << std::endl;
        }
    }
    else
    {
        std::cout << "Encountered error: " << airdrop.error().message << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------
void DistributeTokenTest(const ConnectionPtr& connection, const Wallet& wallet)
{
    auto distribution = DistributeToken(connection, wallet, g::assetName, g::tickOffset);
    if (distribution.has_value())
    {
        std::cout << "Succesfully distributed " << g::assetName << std::endl;
        std::cout << "\tHash: " << distribution->hash << std::endl;
        std::cout << "\tTick: " << distribution->tick << std::endl;
        if (distribution->output)
        {
            std::cout << "\tTransfered amount: " << distribution->output->transferredAmount
                      << std::endl;
        }
    }
    else
    {
        std::cout << "Encountered error: " << distribution.error().message << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------
void TokenTransferTest(
    const ConnectionPtr& connection,
    const Wallet& wallet,
    std::string recipientIdentity)
{
    auto transfer =
        TransferToken(connection, wallet, g::assetName, recipientIdentity, 69, g::tickOffset);
    if (transfer.has_value())
    {
        std::cout << "Succesfully transfered " << g::assetName << " to " << recipientIdentity
                  << std::endl;
        std::cout << "\tHash: " << transfer->hash << std::endl;
        std::cout << "\tTick: " << transfer->tick << std::endl;
        if (transfer->output)
        {
            std::cout << "\tTransfered amount: " << transfer->output->transferredAmount
                      << std::endl;
        }
    }
    else
    {
        std::cout << "Encountered error: " << transfer.error().message << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------
std::vector<Wallet> GenerateWallets(const std::vector<std::string>& seeds)
{
    std::vector<Wallet> wallets;
    for (auto&& seed : seeds)
    {
        // ignore error checks
        wallets.push_back(GenerateWallet(seed).value());
    }
    return wallets;
}

// ------------------------------------------------------------------------------------------------
void PrintWallets(const std::vector<Wallet>& wallets)
{
    for (auto&& wallet : wallets)
    {
        PrintWallet(wallet.seed);
    }
}

// ------------------------------------------------------------------------------------------------
void StartAirdrop(const ConnectionPtr& connection, const std::vector<Wallet>& wallets)
{
    for (size_t i = 0; i < wallets.size(); ++i)
    {
        std::cout << i << ".) Airdropping " << g::assetName << std::endl;
        std::cout << "\tIdentity: " << wallets.at(i).identity << std::endl;
        auto airdrop = StartAirdrop(
            connection,
            wallets.at(i),
            g::assetName,
            "",
            g::numberOfUnits,
            '\0',
            g::tickOffset);

        std::cout << "\tStatus: " << std::boolalpha << airdrop.has_value() << std::endl;

        if (airdrop.has_value())
        {
            std::cout << "Hash: " << airdrop->hash << std::endl;
            std::cout << "Tick: " << airdrop->tick << std::endl;
            if (airdrop->output)
            {
                std::cout << "\tissuedNumberOfShares: " << airdrop->output->issuedNumberOfShares
                          << std::endl;
            }
        }
        else
        {
            std::cout << "Error: " << airdrop.error().message << std::endl;
        }
        std::cout << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------
void DistributeTokenAtTickTest(
    const ConnectionPtr& connection,
    const std::vector<Wallet>& wallets,
    unsigned int tick)
{
    for (size_t i = 0; i < wallets.size(); ++i)
    {
        std::cout << i << ".) Distributing " << g::assetName << std::endl;
        std::cout << "\tIdentity: " << wallets.at(i).identity << std::endl;

        auto distribution = DistributeTokenAtTick(connection, wallets.at(i), g::assetName, tick);
        std::cout << "\tStatus: " << std::boolalpha << distribution.has_value() << std::endl;

        if (distribution.has_value())
        {
            std::cout << "Hash: " << distribution->hash << std::endl;
            std::cout << "Tick: " << distribution->tick << std::endl;
        }
        else
        {
            std::cout << "Error: " << distribution.error().message << std::endl;
        }
        std::cout << std::endl;
    }
}
