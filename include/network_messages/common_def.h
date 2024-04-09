#pragma once

#define SIGNATURE_SIZE 64
#define NUMBER_OF_TRANSACTIONS_PER_TICK 1024 // Must be 2^N
#define MAX_NUMBER_OF_CONTRACTS 1024 // Must be 1024
#define NUMBER_OF_COMPUTORS 676
#define NUMBER_OF_EXCHANGED_PEERS 4
#define SPECTRUM_DEPTH 24 // Defines SPECTRUM_CAPACITY (1 << SPECTRUM_DEPTH)

#define MAX_INPUT_SIZE 1024ULL
#define ISSUANCE_RATE 1000000000000LL
#define MAX_AMOUNT (ISSUANCE_RATE * 1000ULL)

#include "../core/m256.h"

typedef union IPv4Address
{
#ifdef _MSC_VER
    unsigned __int8     u8[4];
    unsigned __int32    u32;
#else
    uint8_t             u8[4];
    uint32_t            u32;
#endif
} IPv4Address;

static_assert(sizeof(IPv4Address) == 4, "Unexpected size!");

static inline bool operator==(const IPv4Address& a, const IPv4Address& b)
{
    return a.u32 == b.u32;
}

static inline bool operator!=(const IPv4Address& a, const IPv4Address& b)
{
    return a.u32 != b.u32;
}
