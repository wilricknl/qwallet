#pragma once

#include <list>
#include <memory>
#include <string>

#include "window.hpp"

// ------------------------------------------------------------------------------------------------
/**
 * qwallet main window
 */
class QWallet final : public Window
{
public:
    /**
     * Constructor
     * @param name The application name
     */
    QWallet(std::string name);

    /**
     * Initialize application with glfw context
     * @param window The glfw window
     * @param glVersion The OpenGL version to use
     */
    bool Initialize(GLFWwindow* window, const std::string& glVersion = "#version 330");

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

    /**
     * Should application quit
     */
    bool Quit() const;

    /**
     * Destroy application resources
     */
    void Destroy() const;

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
    /// All sub-windows
    std::list<std::shared_ptr<Window>> m_windows;

    /// Check if application is finished
    bool m_bQuit = false;

#ifndef NDEBUG
    /// Should show demo?
    bool m_bShowDemo = false;
#endif
};
