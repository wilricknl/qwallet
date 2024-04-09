#include <iostream>

#include "network/connection.hpp"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <ip-address> <port>" << std::endl;
        return 1;
    }

    InitializeConnection();

    {
        auto result = CreateConnection(argv[1], atoi(argv[2]));
        if (!result.has_value())
        {
            std::cout << "Failed to create connection: " << result.error().message << std::endl;
            return 1;
        }

        auto connection = result.value();

        char message[]{ "QWallet client test" };
        if (!connection->Send(message, sizeof(message)))
        {
            std::cout << "Failed to send a message to " << argv[1] << ":" << argv[2] << std::endl;
        }
    }

    DestroyConnection();
}
