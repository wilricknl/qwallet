#include "utility.hpp"

// ------------------------------------------------------------------------------------------------
std::string to_comma_separated_string(long long amount)
{
    auto amount_comma_separated = std::to_string(amount);
    auto n = static_cast<int>(amount_comma_separated.length()) - 3;

    const int offset = amount < 0 ? 1 : 0;

    while (n > offset)
    {
        amount_comma_separated.insert(n, ",");
        n -= 3;
    }

    return amount_comma_separated;
}

// ------------------------------------------------------------------------------------------------
std::string to_comma_separated_string(unsigned long long amount)
{
    auto amount_comma_separated = std::to_string(amount);
    auto n = static_cast<int>(amount_comma_separated.length()) - 3;

    while (n > 0)
    {
        amount_comma_separated.insert(n, ",");
        n -= 3;
    }

    return amount_comma_separated;
}
