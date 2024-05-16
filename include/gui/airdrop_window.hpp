#pragma once

#include "gui/window.hpp"

// ------------------------------------------------------------------------------------------------
/**
 * Airdrop window
 */
class AirdropWindow final : public Window
{
public:
    /**
     * Constructor
     * @param name The name of the window
     * @param bShow Should show the window
     * @param bCanClose Can close the window
     */
    explicit AirdropWindow(std::string name, bool bShow, bool bCanClose = true);

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
     * Begin render
     * @param flags The window flags
     */
    bool Begin(ImGuiWindowFlags flags) override;

    /**
     * End render
     */
    void End() override;

private:
};
