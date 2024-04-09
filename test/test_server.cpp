#include <iostream>

#ifdef _MSC_VER

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "Winsock initialization failed" << std::endl;
        return 1;
    }

    SOCKET server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create a server socket" << std::endl;
        return 1;
    }

    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));

    if (bind(server_socket, (SOCKADDR*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind the server" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR)
    {
        std::cerr << "Failed to start listening" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    sockaddr_in client_address;
    int client_address_size = sizeof(client_address);
    SOCKET client_socket = accept(server_socket, (SOCKADDR*)&client_address, &client_address_size);
    if (client_socket == INVALID_SOCKET)
    {
        std::cout << "Failed to accept client socket" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    char buffer[1024]{0};
    if (recv(client_socket, buffer, sizeof(buffer), 0) != -1)
    {
        std::cout << "Received the following message: " << buffer << std::endl;
    }
    else
    {
        std::cout << "An error occured while receiving data" << std::endl;
    }

    char message[]{"Hello, from the Windows test server!"};
    send(client_socket, message, sizeof(message), 0);

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}

#else

#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Failed to create a server socket" << std::endl;
        return 1;
    }

    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
    {
        std::cerr << "Failed to bind the server" << std::endl;
        std::cerr << "Error: " << strerror(errno) << std::endl;
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) == -1)
    {
        std::cerr << "Failed to start listening" << std::endl;
        close(server_socket);
        return 1;
    }

    sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_socket =
        accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);
    if (client_socket == -1)
    {
        std::cout << "Failed to accept client socket" << std::endl;
        close(server_socket);
        return 1;
    }

    char buffer[1024]{0};
    if (recv(client_socket, buffer, sizeof(buffer), 0) != -1)
    {
        std::cout << "Received the following message: " << buffer << std::endl;
    }
    else
    {
        std::cout << "An error occured while receiving data" << std::endl;
    }

    char message[]{"Hello, from the Linux test server!"};
    send(client_socket, message, sizeof(message), 0);

    close(client_socket);
    close(server_socket);

    return 0;
}

#endif
