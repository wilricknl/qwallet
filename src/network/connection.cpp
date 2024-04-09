#include "network/connection.hpp"

#ifdef _MSC_VER

#pragma comment(lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <Ws2tcpip.h>

#define close(x) closesocket(x)

#else

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

#endif

// ------------------------------------------------------------------------------------------------
Connection::Connection(long socket) noexcept
    : m_socket(socket)
{}

// ------------------------------------------------------------------------------------------------
Connection::Connection(Connection&& other) noexcept
    : Connection(other.m_socket)
{
    other.m_socket = -1;
}

// ------------------------------------------------------------------------------------------------
Connection& Connection::operator=(Connection&& other) noexcept
{
    if (this != &other)
    {
        m_socket = other.m_socket;
        other.m_socket = -1;
    }
    return *this;
}

// ------------------------------------------------------------------------------------------------
Connection::~Connection() { close(m_socket); }

// ------------------------------------------------------------------------------------------------
bool Connection::Send(char* buffer, int buffer_length) const
{
    int total_sent = 0;
    int bytes_left = buffer_length;

    while (total_sent < buffer_length)
    {
        int result = send(m_socket, buffer + total_sent, bytes_left, 0);
        if (result == -1)
        {
            return false;
        }
        total_sent += result;
        bytes_left -= result;
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
bool InitializeConnection()
{
#ifdef _MSC_VER
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true;
#endif
}

// ------------------------------------------------------------------------------------------------
bool DestroyConnection()
{
#ifdef _MSC_VER
    return WSACleanup() == 0;
#else
    return true;
#endif
}

// ------------------------------------------------------------------------------------------------
tl::expected<ConnectionPtr, ConnectionError> CreateConnection(
    const std::string& ip_address,
    unsigned short port)
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _MSC_VER
    size_t tv = 1000;
#else
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
#endif

    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    sockaddr_in server_address;
    memset((char*)&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_address.c_str(), &server_address.sin_addr) <= 0)
    {
        return tl::make_unexpected(ConnectionError{"Invalid ip-address: " + ip_address});
    }

    if (connect(client_socket, (const sockaddr*)&server_address, sizeof(server_address)) != 0)
    {
        return tl::make_unexpected(ConnectionError{"Failed to connect with: " + ip_address});
    }

    Connection connection{client_socket};
    return std::make_shared<Connection>(std::move(connection));
}

// ------------------------------------------------------------------------------------------------
bool IsValidIp(const std::string& ip_address)
{
    sockaddr_in address;
    return inet_pton(AF_INET, ip_address.c_str(), &address.sin_addr) == 1;
}
