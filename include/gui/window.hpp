#pragma once

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
// clang-format on

#include <functional>
#include <string>

// ----------------------------------------------------------------------------
/**
 * Window interface
 */
class Window
{
public:
    /**
     * Constructor
     * @param name The name of the window
     * @param bShow Should show the window
     */
    Window(std::string name, bool bShow = true, bool bCanClose = true);

    /**
     * Destructor
     */
    virtual ~Window() = default;

    /**
     * Get window name
     * @return The name
     */
    const std::string& GetName() const;

    /**
     * Update the window
     * @param glfwWindow The main window
     * @param deltaTime Elapsed time since last update
     */
    virtual void Update(GLFWwindow* glfwWindow, double deltaTime);

    /**
     * Render the window
     */
    virtual void Render() = 0;

    /**
     * Is window shown
     * @return `true` if shown, else `false`.
     */
    bool IsShow() const;

    /**
     * Set new window show state
     * @param bShow New state
     */
    void SetShow(bool bShow);

    /**
     * Access show boolean via pointer
     */
    bool* Show();

    /**
     * Can the window close
     * @return `true` if can close, else `false`.
     */
    bool IsCanClose() const;

    /**
     * Is window focused
     * @return `true` if focused, else `false`
     */
    bool IsFocused() const;

    /**
     * Is window hovered
     * @return `true` if hovered, else `false`
     */
    bool IsHovered() const;

    /**
     * Get mouse button callback
     * @return The callback
     */
    const std::function<void(GLFWwindow*, int, int, int)>& GetMouseButtonCallback() const;

    /**
     * Get cursor position callback
     * @return The callback
     */
    const std::function<void(GLFWwindow*, double, double)>& GetCursorCallback() const;

    /**
     * Get key callback
     * @return The callback
     */
    const std::function<void(GLFWwindow*, int, int, int, int)>& GetKeyCallback() const;

protected:
    /**
     * Begin of draw
     * @return `true` if visible, else `false`
     */
    virtual bool Begin(ImGuiWindowFlags flags);

    /**
     * End of draw
     */
    virtual void End();

protected:
    /// Name of the window
    std::string m_name;

    /// Should show the window
    bool m_bShow;

    /// Can close this window
    bool m_bCanClose;

    /// Is window focused
    bool m_bFocused;

    /// Is window focused
    bool m_bHovered;

    /// Mouse button callback
    std::function<void(GLFWwindow*, int, int, int)> m_mouseButtonCallback{};

    /// Mouse movement callback
    std::function<void(GLFWwindow*, double, double)> m_cursorPosCallback{};

    /// Key callback
    std::function<void(GLFWwindow*, int, int, int, int)> m_keyCallback{};
};
