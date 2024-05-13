#include <catch.hpp>

#include "wallet.hpp"

// todo: remove
#include <iostream>

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

    // valid
    {
        std::string seed = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

        std::cout << "error -1" << std::endl;

        auto result = GenerateWallet(seed);


        std::cout << "error 0" << std::endl;
        REQUIRE(result.has_value() == true);

        std::cout << "error 1" << std::endl;
        auto wallet = result.value();

std::cout << "error 2" << std::endl;
        REQUIRE(wallet.seed == seed);
        std::cout << "error 3" << std::endl;
        REQUIRE(
            wallet.private_key == "cctwbaulwuyhybijykxrmxnyrvzbalwryiiahltfwanuafhyfhepcjjgvaec");
        
        std::cout << "error 4" << std::endl;
        REQUIRE(
            wallet.public_key == "bzbqfllbncxemglobhuvftluplvcpquassilfaboffbcadqssupnwlzbqexk");
       std::cout << "error 5" << std::endl;
        REQUIRE(wallet.identity == "BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK");
    }
}
