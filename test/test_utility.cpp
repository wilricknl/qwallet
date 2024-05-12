#include <catch.hpp>

#include "utility.hpp"

// ------------------------------------------------------------------------------------------------
TEST_CASE("Signed comma seperated string", "[Utility]")
{
    long long amount = 1000;
    CHECK(ToCommaSeparatedString(amount) == "1,000");

    amount = -1000;
    CHECK(ToCommaSeparatedString(amount) == "-1,000");

    amount = 123456;
    CHECK(ToCommaSeparatedString(amount) == "123,456");

    amount = -123456;
    CHECK(ToCommaSeparatedString(amount) == "-123,456");

    amount = 1000000000;
    CHECK(ToCommaSeparatedString(amount) == "1,000,000,000");
    
    amount = -1000000000;
    CHECK(ToCommaSeparatedString(amount) == "-1,000,000,000");
}

// ------------------------------------------------------------------------------------------------
TEST_CASE("unsigned comma seperated string", "[Utility]")
{
    unsigned long long amount = 1000;
    CHECK(ToCommaSeparatedString(amount) == "1,000");

    amount = 1000000000;
    CHECK(ToCommaSeparatedString(amount) == "1,000,000,000");
}
