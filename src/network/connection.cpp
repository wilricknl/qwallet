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
bool Connection::Send(char* buffer, int bufferLength) const
{
    int totalSent = 0;
    int bytesLeft = bufferLength;

    while (totalSent < bufferLength)
    {
        int result = send(m_socket, buffer + totalSent, bytesLeft, 0);
        if (result == -1)
        {
            return false;
        }
        totalSent += result;
        bytesLeft -= result;
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
std::vector<char> Connection::Receive() const
{
    std::vector<char> buffer;
    buffer.resize(0);

    char temporary[1024];
    int bytes_received = recv(m_socket, temporary, sizeof(temporary), 0);

    while (bytes_received > 0)
    {
        buffer.resize(buffer.size() + bytes_received);
        memcpy(buffer.data() + buffer.size() - bytes_received, temporary, bytes_received);
        bytes_received = recv(m_socket, temporary, sizeof(temporary), 0);
    }

    return buffer;
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
    const std::string& ipAddress,
    unsigned short port)
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _MSC_VER
    size_t tv = 1000;
#else
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
#endif

    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    sockaddr_in serverAddress;
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddress.sin_addr) <= 0)
    {
        return tl::make_unexpected(ConnectionError{"Invalid ip-address: " + ipAddress});
    }

    if (connect(clientSocket, (const sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        return tl::make_unexpected(ConnectionError{"Failed to connect with: " + ipAddress});
    }

    Connection connection{clientSocket};
    return std::make_shared<Connection>(std::move(connection));
}

// ------------------------------------------------------------------------------------------------
bool IsValidIp(const std::string& ipAddress)
{
    sockaddr_in address;
    return inet_pton(AF_INET, ipAddress.c_str(), &address.sin_addr) == 1;
}
