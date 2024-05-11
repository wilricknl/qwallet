#include <iostream>

#include "network_messages/all.h"
#include "network/connection.hpp"

// ------------------------------------------------------------------------------------------------
// Test Qubic Client
// ------------------------------------------------------------------------------------------------
/**
 * Example to interact with the Qubic through the Connection class
 *
 * Usage: test_qubic_client.exe <ip-address> <port>
 *  e.g.: test_qubic_client.exe 95.156.230.174 21841
 */
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <ip-address> <port>" << std::endl;
        return 1;
    }

    // Setup socket libraries
    InitializeConnection();

    {
        // Create connection
        auto result = CreateConnection(argv[1], atoi(argv[2]));
        if (!result.has_value())
        {
            std::cout << "Failed to create connection: " << result.error().message << std::endl;
            return 1;
        }
        auto connection = result.value();

        // Send request
        struct
        {
            RequestResponseHeader header;
        } packet;
        packet.header.setSize<sizeof(RequestResponseHeader)>();
        packet.header.randomizeDejavu();
        packet.header.setType(REQUEST_SYSTEM_INFO);

        if (!connection->Send((char*)&packet, sizeof(packet)))
        {
            std::cout << "Failed to send a message to " << argv[1] << ":" << argv[2] << std::endl;
        }

        // Receive response
        auto response = connection->ReceiveAs<RespondSystemInfo>(RESPOND_SYSTEM_INFO);
        if (response)
        {
            auto system_info = response.value();
            printf("Version:              %u\n", system_info.version);
            printf("Epoch:                %u\n", system_info.epoch);
            printf("Tick:                 %u\n", system_info.tick);
            printf("InitialTick:          %u\n", system_info.initialTick);
            printf("LatestCreatedTick:    %u\n", system_info.latestCreatedTick);
            printf("NumberOfEntities:     %u\n", system_info.numberOfEntities);
            printf("NumberOfTransactions: %u\n", system_info.numberOfTransactions);
        }
        else
        {
            std::cout << "error: " << response.error().message << std::endl;
        }
    }

    // Clean up socket libraries
    DestroyConnection();
}
