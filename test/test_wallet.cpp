#include <catch.hpp>

#include "wallet.hpp"

// ------------------------------------------------------------------------------------------------
TEST_CASE("Seed validation", "[Wallet]")
{
    // invalid length
    {
        std::string seed = "abcde";
        std::string error_message;
        REQUIRE(IsValidSeed(error_message, seed) == false);
        REQUIRE(error_message == "The seed length is invalid");
        REQUIRE(IsValidSeed(seed) == false);
    }

    // invalid characters
    {
        std::string seed = "a!cdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcd1";
        std::string error_message;
        REQUIRE(IsValidSeed(error_message, seed) == false);
        REQUIRE(error_message == "Seed contains invalid characters");
        REQUIRE(IsValidSeed(seed) == false);
    }

    // valid
    {
        std::string seed = "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde";
        std::string error_message;
        REQUIRE(IsValidSeed(error_message, seed) == true);
        REQUIRE(error_message.empty());
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

    // // valid
    // {
    //     std::string seed = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

    //     auto result = GenerateWallet(seed);
    //     REQUIRE(result.has_value() == true);

    //     auto wallet = result.value();

    //     REQUIRE(wallet.seed == seed);
    //     REQUIRE(
    //         wallet.private_key == "cctwbaulwuyhybijykxrmxnyrvzbalwryiiahltfwanuafhyfhepcjjgvaec");
    //     REQUIRE(
    //         wallet.public_key == "bzbqfllbncxemglobhuvftluplvcpquassilfaboffbcadqssupnwlzbqexk");
    //     REQUIRE(wallet.identity == "BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK");
    // }
}
