#pragma once

#include <tl/expected.hpp>

#include <cstring>
#include <memory>
#include <string>

#include "network_messages/header.h"

// ------------------------------------------------------------------------------------------------
/**
 * Connection smart pointer
 */
typedef std::shared_ptr<class Connection> ConnectionPtr;

// ------------------------------------------------------------------------------------------------
/**
 * Connection error message
 */
struct ConnectionError
{
    std::string message;
};

// ------------------------------------------------------------------------------------------------
/**
 * Socket wrapper
 */
class Connection
{
private:
    /**
     * Hidden constructor
     * @param socket The socket descriptor
     */
    Connection(long socket) noexcept;

public:
    /**
     * Factory function
     */
    friend tl::expected<ConnectionPtr, ConnectionError> CreateConnection(
        const std::string& ipAddress,
        unsigned short port);

    Connection(Connection&& other) noexcept;
    Connection(const Connection&) = delete;
    Connection& operator=(Connection&& other) noexcept;
    Connection& operator=(const Connection&) = delete;
    virtual ~Connection();

    /**
     * Send data
     * @param buffer The data to be send
     * @param bufferLength The length of the buffer
     * @return `true` upon success, else `false`
     */
    bool Send(char* buffer, int bufferLength) const;

    /**
     * Receive data
     * @return The received data
     */
    std::vector<char> Receive() const;

    /**
     * Receive data as a specific type
     * @param headerType The type (index) of the header that should contain the data
     * @return The data or encountered error
     */
    template <typename T>
    tl::expected<T, ConnectionError> ReceiveAs(unsigned char headerType) const;

private:
    long m_socket;
};

// ------------------------------------------------------------------------------------------------
template <typename T>
tl::expected<T, ConnectionError> Connection::ReceiveAs(unsigned char headerType) const
{
    std::vector<char> buffer = Receive();
    if (buffer.empty())
    {
        return tl::make_unexpected(ConnectionError{"Connection failed to get a response"});
    }

    for (int offset = 0; offset < buffer.size();)
    {
        auto* header = reinterpret_cast<RequestResponseHeader*>(buffer.data() + offset);
        if (header->type() == headerType)
        {
            // todo: there's also a function to check min/max, which packets have dynamic size?
            if (header->checkPayloadSize(sizeof(T)))
            {
                T result = *header->getPayload<T>();
                return result;
            }
            return tl::make_unexpected(ConnectionError{
                "Response of type " + std::to_string(header->type()) +
                " had the size: " + std::to_string(header->getPayloadSize()) +
                " instead of expected size: " + std::to_string(sizeof(T))});
        }

        offset += header->size();
    }

    return tl::make_unexpected(ConnectionError{
        "Response did not contain header type: " + std::to_string((int)headerType)});
}

// ------------------------------------------------------------------------------------------------
/**
 * Initialize WinSockets - nothing happens on Linux
 */
bool InitializeConnection();

// ------------------------------------------------------------------------------------------------
/**
 * Clean up WinSockets - nothing happens on Linux
 */
bool DestroyConnection();

// ------------------------------------------------------------------------------------------------
/**
 * Create a new connection
 */
tl::expected<ConnectionPtr, ConnectionError> CreateConnection(
    const std::string& ipAddress,
    unsigned short port);

// ------------------------------------------------------------------------------------------------
/**
 * Check if string contains a valid ip-address
 * @param ipAddress The string to check
 * @return `true` if valid, else `false`
 */
bool IsValidIp(const std::string& ipAddress);
