#include "network/tick.hpp"

#include <cstring>

// ------------------------------------------------------------------------------------------------
tl::expected<CurrentTickInfo, ConnectionError> getCurrentTickInfo(const ConnectionPtr& connection)
{
    // construct request packet
    struct
    {
        RequestResponseHeader header;
    } packet;
    packet.header.setSize<sizeof(packet)>();
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_CURRENT_TICK_INFO);

    // send request
    connection->Send((char*)&packet, sizeof(packet));

    // receive response
    return connection->ReceiveAs<CurrentTickInfo>(RESPOND_CURRENT_TICK_INFO);
}

// ------------------------------------------------------------------------------------------------
tl::expected<unsigned short, ConnectionError> getTickDuration(const ConnectionPtr& connection)
{
    auto info = getCurrentTickInfo(connection);
    if (info)
    {
        return info->tickDuration;
    }
    return tl::make_unexpected(info.error());
}

// ------------------------------------------------------------------------------------------------
tl::expected<unsigned short, ConnectionError> getEpoch(const ConnectionPtr& connection)
{
    auto info = getCurrentTickInfo(connection);
    if (info)
    {
        return info->epoch;
    }
    return tl::make_unexpected(info.error());
}

// ------------------------------------------------------------------------------------------------
tl::expected<unsigned, ConnectionError> getTick(const ConnectionPtr& connection)
{
    auto info = getCurrentTickInfo(connection);
    if (info)
    {
        return info->tick;
    }
    return tl::make_unexpected(info.error());
}

// ------------------------------------------------------------------------------------------------
tl::expected<unsigned short, ConnectionError> getNumberOfAlignedVotes(
    const ConnectionPtr& connection)
{
    auto info = getCurrentTickInfo(connection);
    if (info)
    {
        return info->numberOfAlignedVotes;
    }
    return tl::make_unexpected(info.error());
}

// ------------------------------------------------------------------------------------------------
tl::expected<unsigned short, ConnectionError> getNumberOfMisalignedVotes(
    const ConnectionPtr& connection)
{
    auto info = getCurrentTickInfo(connection);
    if (info)
    {
        return info->numberOfMisalignedVotes;
    }
    return tl::make_unexpected(info.error());
}

// ------------------------------------------------------------------------------------------------
tl::expected<unsigned, ConnectionError> getInitialTick(const ConnectionPtr& connection)
{
    auto info = getCurrentTickInfo(connection);
    if (info)
    {
        return info->initialTick;
    }
    return tl::make_unexpected(info.error());
}

// ------------------------------------------------------------------------------------------------
tl::expected<BroadcastFutureTickData, ConnectionError> getTickData(
    const ConnectionPtr& connection,
    unsigned tick)
{
    // Construct request packet
    struct
    {
        RequestResponseHeader header;
        RequestTickData payload;
    } packet;

    // Init header
    packet.header.setSize<sizeof(packet)>();
    packet.header.randomizeDejavu();
    packet.header.setType(packet.payload.type);

    // Init request tick data
    packet.payload.requestedTickData.tick = tick;

    // Send request
    connection->Send((char*)&packet, sizeof(packet));

    // Receive response
    return connection->ReceiveAs<BroadcastFutureTickData>(BroadcastFutureTickData::type);
}
