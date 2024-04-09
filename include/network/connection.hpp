#pragma once

#include <tl/expected.hpp>

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
        const std::string& ip_address,
        unsigned short port);

    Connection(Connection&& other) noexcept;
    Connection(const Connection&) = delete;
    Connection& operator=(Connection&& other) noexcept;
    Connection& operator=(const Connection&) = delete;
    virtual ~Connection();

    /**
     * Send data
     * @param buffer The data to be send
     * @param buffer_length The length of the buffer
     * @return `true` upon success, else `false`
     */
    bool Send(char* buffer, int buffer_length) const;

    /**
     * Receive data
     * @return The received data
     */
    std::vector<char> Receive() const;

    /**
     * Receive data as a specific type
     * @param header_type The type (index) of the header that should contain the data
     * @return The data
     */
    template<typename T> T ReceiveAs(unsigned char header_type) const;

private:
    long m_socket;
};

// ------------------------------------------------------------------------------------------------
// todo (wilricknl): possibly add a check that `header->size() == sizeof(T)`
// todo (wilricknl): return an unexpected value, rather than zeroed out struct.
template <typename T>
T Connection::ReceiveAs(unsigned char header_type) const
{
    std::vector<char> buffer = Receive();

    T result;
    memset(&result, 0, sizeof(T));

    for (int offset = 0; offset < buffer.size();)
    {
        auto* header = reinterpret_cast<RequestResponseHeader*>(buffer.data() + offset);

        if (header->type() == header_type)
        {
            result = *(T*)(buffer.data() + offset + sizeof(RequestResponseHeader));
        }

        offset += header->size();
    }

    return result;
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
    const std::string& ip_address,
    unsigned short port);

// ------------------------------------------------------------------------------------------------
/**
 * Check if string contains a valid ip-address
 * @param ip_address The string to check
 * @return `true` if valid, else `false`
 */
bool IsValidIp(const std::string& ip_address);
