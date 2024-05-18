#include <iostream>

#include "network/airdrop.hpp"
#include "network/connection.hpp"
#include "network/tick.hpp"

// ------------------------------------------------------------------------------------------------
namespace
{
constexpr auto seed = "";
constexpr auto ipAddress = "57.129.19.155";
constexpr unsigned short port = 31843;
constexpr auto assetName = "QTOOL";
constexpr int tickOffset = 5;
} // namespace

// ------------------------------------------------------------------------------------------------
/**
 * Forward declarations
 */
void GetTickTest(const ConnectionPtr& connection);
void RequestFeesTest(const ConnectionPtr& connection);
void StartAirdropTest(const ConnectionPtr& connection, const Wallet& wallet);
void DistributeTokenTest(const ConnectionPtr& connection, const Wallet& wallet);
void TokenTransferTest(
    const ConnectionPtr& connection,
    const Wallet& wallet,
    std::string recipientIdentity);

// ------------------------------------------------------------------------------------------------
/**
 * QAirdrop test
 */
int main()
{
    InitializeConnection();

    // setup connection with test server
    auto result = CreateConnection(ipAddress, port);
    if (!result.has_value())
    {
        std::cout << "Failed to create connection: " << result.error().message << std::endl;
        return 1;
    }
    auto connection = result.value();

    // initialize issuer wallet
    Wallet wallet;
    {
        auto result = GenerateWallet(seed);
        if (!result)
        {
            std::cout << "Failed to create wallet" << std::endl;
            return 1;
        }
        wallet = result.value();
    }

    GetTickTest(connection);
    // RequestFeesTest(connection);
    // StartAirdropTest(connection, wallet);
    // DistributeTokenTest(connection, wallet);
    // TokenTransferTest(
    //     connection,
    //     wallet,
    //     "ZQUIXJQCPSLDFFJANLAKYZDVBYCAVHQIUTAGZBLRVFKPUZDECALJFFMEPFRH");

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
void StartAirdropTest(const ConnectionPtr& connection, const Wallet& wallet)
{
    auto airdrop = StartAirdrop(connection, wallet, assetName, "0000000", 21000000, '\0', 5);
    if (airdrop.has_value())
    {
        std::cout << "Succesfully created airdrop" << std::endl;
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
    auto distribution = DistributeToken(connection, wallet, assetName, tickOffset);
    if (distribution.has_value())
    {
        std::cout << "Succesfully distributed " << assetName << std::endl;
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
    auto transfer = TransferToken(connection, wallet, assetName, recipientIdentity, 69, tickOffset);
    if (transfer.has_value())
    {
        std::cout << "Succesfully transfered " << assetName << " to " << recipientIdentity
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
