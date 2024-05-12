#include "utility.hpp"

// ------------------------------------------------------------------------------------------------
std::string ToCommaSeparatedString(long long amount)
{
    auto amountCommaSeparated = std::to_string(amount);
    auto n = static_cast<int>(amountCommaSeparated.length()) - 3;

    const int offset = amount < 0 ? 1 : 0;

    while (n > offset)
    {
        amountCommaSeparated.insert(n, ",");
        n -= 3;
    }

    return amountCommaSeparated;
}

// ------------------------------------------------------------------------------------------------
std::string ToCommaSeparatedString(unsigned long long amount)
{
    auto amountCommaSeparated = std::to_string(amount);
    auto n = static_cast<int>(amountCommaSeparated.length()) - 3;

    while (n > 0)
    {
        amountCommaSeparated.insert(n, ",");
        n -= 3;
    }

    return amountCommaSeparated;
}
