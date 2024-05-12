#include <catch.hpp>

#include "wallet.hpp"

// ------------------------------------------------------------------------------------------------
TEST_CASE("Seed validation", "[Wallet]")
{
    // invalid length
    {
        std::string seed = "abcde";
        std::string errorMessage;
        REQUIRE(IsValidSeed(errorMessage, seed) == false);
        REQUIRE(errorMessage == "The seed length is invalid");
        REQUIRE(IsValidSeed(seed) == false);
    }

    // invalid characters
    {
        std::string seed = "a!cdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcd1";
        std::string errorMessage;
        REQUIRE(IsValidSeed(errorMessage, seed) == false);
        REQUIRE(errorMessage == "Seed contains invalid characters");
        REQUIRE(IsValidSeed(seed) == false);
    }

    // valid
    {
        std::string seed = "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde";
        std::string errorMessage;
        REQUIRE(IsValidSeed(errorMessage, seed) == true);
        REQUIRE(errorMessage.empty());
        REQUIRE(IsValidSeed(seed) == true);
    }
}

// ------------------------------------------------------------------------------------------------
TEST_CASE("Seed generation", "[Wallet]")
{
    auto seed = GenerateSeed();
    REQUIRE(IsValidSeed(seed) == true);
}

// ------------------------------------------------------------------------------------------------
TEST_CASE("Wallet generation", "[Wallet]")
{
    // invalid length
    {
        std::string seed = "abcde";

        auto result = GenerateWallet(seed);
        REQUIRE(result.has_value() == false);

        auto wallet_error = result.error();
        REQUIRE(wallet_error.message == "The seed length is invalid");
    }

    // invalid characters
    {
        std::string seed = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa12345";

        auto result = GenerateWallet(seed);
        REQUIRE(result.has_value() == false);

        auto wallet_error = result.error();
        REQUIRE(wallet_error.message == "Seed contains invalid characters");
    }

    // valid
    {
        auto seed = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

        auto result = GenerateWallet(seed);
        REQUIRE(result.has_value());

        auto wallet = result.value();

        REQUIRE(wallet.seed == seed);
        REQUIRE(
            wallet.privateKey == "cctwbaulwuyhybijykxrmxnyrvzbalwryiiahltfwanuafhyfhepcjjgvaec");
        REQUIRE(
            wallet.publicKey == "bzbqfllbncxemglobhuvftluplvcpquassilfaboffbcadqssupnwlzbqexk");
        REQUIRE(wallet.identity == "BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK");
    }
}
