#pragma once

#include <tl/expected.hpp>

#include <memory>
#include <string>

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

private:
    long m_socket;
};

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
