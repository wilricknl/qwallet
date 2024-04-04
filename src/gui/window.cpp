#include "gui/window.hpp"

// ------------------------------------------------------------------------------------------------
Window::Window(std::string name, bool bShow, bool bCanClose)
    : m_name(std::move(name))
    , m_bShow(bShow)
    , m_bCanClose(bCanClose)
    , m_bFocused(false)
    , m_bHovered(false)
{}

// ------------------------------------------------------------------------------------------------
const std::string& Window::GetName() const { return m_name; }

// ------------------------------------------------------------------------------------------------
void Window::Update(GLFWwindow* glfwWindow, double deltaTime) {}

// ------------------------------------------------------------------------------------------------
bool Window::IsShow() const { return m_bShow; }

// ------------------------------------------------------------------------------------------------
void Window::SetShow(bool bShow) { m_bShow = bShow; }

// ------------------------------------------------------------------------------------------------
bool* Window::Show() { return &m_bShow; }

// ------------------------------------------------------------------------------------------------
bool Window::IsCanClose() const { return m_bCanClose; }

// ------------------------------------------------------------------------------------------------
bool Window::IsFocused() const { return m_bFocused; }

// ------------------------------------------------------------------------------------------------
bool Window::IsHovered() const { return m_bHovered; }

// ------------------------------------------------------------------------------------------------
const std::function<void(GLFWwindow*, int, int, int)>& Window::GetMouseButtonCallback() const
{
    return m_mouseButtonCallback;
}

// ------------------------------------------------------------------------------------------------
const std::function<void(GLFWwindow*, double, double)>& Window::GetCursorCallback() const
{
    return m_cursorPosCallback;
}

// ------------------------------------------------------------------------------------------------
const std::function<void(GLFWwindow*, int, int, int, int)>& Window::GetKeyCallback() const
{
    return m_keyCallback;
}

// ------------------------------------------------------------------------------------------------
bool Window::Begin(ImGuiWindowFlags flags)
{
    if (ImGui::Begin(m_name.c_str(), m_bCanClose ? &m_bShow : nullptr, flags))
    {
        m_bFocused = ImGui::IsWindowFocused();
        m_bHovered = ImGui::IsWindowHovered();
        return true;
    }

    ImGui::End();
    m_bFocused = m_bHovered = false;
    return m_bFocused;
}

// ------------------------------------------------------------------------------------------------
void Window::End() { ImGui::End(); }
