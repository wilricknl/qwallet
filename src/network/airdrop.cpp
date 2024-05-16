#include "network/airdrop.hpp"

#include "network_messages/contract.h"

// ------------------------------------------------------------------------------------------------
namespace
{

constexpr int QAIRDROP_CONTRACT_ID = 5;
constexpr int QAIRDROP_FUNCTION_Fees = 1;
constexpr int QAIRDROP_PROCEDURE_StartAirdrop = 1;
constexpr int QAIRDROP_PROCEDURE_DistributeToken = 2;

} // namespace
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
tl::expected<Fees_output, AirdropError> GetAirdropFees(const ConnectionPtr& connection)
{
    // construct packet
    struct
    {
        RequestResponseHeader header;
        RequestContractFunction payload;
    } packet;

    // init header
    packet.header.setSize<sizeof(packet)>();
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type);

    // init payload
    packet.payload.inputSize = 0;
    packet.payload.inputType = QAIRDROP_FUNCTION_Fees;
    packet.payload.contractIndex = QAIRDROP_CONTRACT_ID;

    // request airdrop fees
    if (!connection->Send((char*)&packet, packet.header.size()))
    {
        return tl::make_unexpected(AirdropError{"Failed to request airdrop fees"});
    }

    // receive response
    auto response = connection->ReceiveAs<Fees_output>(RespondContractFunction::type);
    if (response.has_value())
    {
        return response.value();
    }

    return tl::make_unexpected(AirdropError{response.error().message});
}
