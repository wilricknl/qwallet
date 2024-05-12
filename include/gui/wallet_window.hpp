#pragma once

#include <atomic>
#include <future>

#include "gui/window.hpp"
#include "network/transactions.hpp"
#include "wallet.hpp"

// ------------------------------------------------------------------------------------------------
/**
 * Wallet window
 */
class WalletWindow final : public Window
{
public:
    /**
     * Constructor
     * @param name The name of the window
     * @param bShow Should show the window
     * @param bCanClose Can close the window
     */
    explicit WalletWindow(std::string name, bool bShow, bool bCanClose = true);

    /**
     * Destructor
     */
    ~WalletWindow();

    /**
     * Next program step
     * @param glfwWindow The glfw window
     * @param deltaTime Elapsed time since previous step
     */
    void Update(GLFWwindow* glfwWindow, double deltaTime) override;

    /**
     * Render the application
     */
    void Render() override;

protected:
    /**
     * Tab for wallet generation controls
     */
    void WalletGenerationTab();

    /**
     * Tab to request account balance
     */
    void AccountBalanceTab();

    /**
     * Tab to make transactions
     */
    void TransactionTab();

protected:
    /**
     * Begin render
     * @param flags The window flags
     */
    bool Begin(ImGuiWindowFlags flags) override;

    /**
     * End render
     */
    void End() override;

protected:
    /**
     * Helper function to verify the input of the transaction tab
     * @param seed The seed of the sender
     * @param recipient The identity of the recipient
     * @param ipAddress The ip-address of the node to broadcast to
     * @param port The port of the node to broadcast to
     * @param amount The amount to transact
     * @return `true` upon success, else an error message
     */
    tl::expected<bool, TransactionError> VerifyTransactionInput(
        const std::string& seed,
        const std::string& recipient,
        const std::string& ipAddress,
        const std::string& port,
        unsigned long long amount);

private:
    /// Is brute force running
    bool m_bWaitingForBruteForce = false;

    /// Future to receive brute force result
    std::future<Wallet> m_bruteForceFuture;

    /// Tell brute force threads to stop
    std::atomic<bool> m_stopBruteForce = false;
};
