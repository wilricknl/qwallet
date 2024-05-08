#pragma once

#include <atomic>
#include <future>
#include <mutex>

#include "gui/window.hpp"
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

private:
    /// Is brute force running
    bool m_bWaitingForBruteForce = false;

    /// Future to receive brute force result
    std::future<Wallet> m_bruteForceFuture;

    /// Tell brute force threads to stop
    std::atomic<bool> m_stopBruteForce = false;
};
