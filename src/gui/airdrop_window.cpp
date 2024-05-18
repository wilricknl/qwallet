#include "gui/airdrop_window.hpp"

// ------------------------------------------------------------------------------------------------
AirdropWindow::AirdropWindow(std::string name, bool bShow, bool bCanClose)
    : Window(std::move(name), bShow, bCanClose)
{}

// ------------------------------------------------------------------------------------------------
void AirdropWindow::Update(GLFWwindow* glfwWindow, double deltaTime)
{
    Window::Update(glfwWindow, deltaTime);
}

// ------------------------------------------------------------------------------------------------
void AirdropWindow::Render()
{
    if (!m_bShow || !Begin(ImGuiWindowFlags_None))
    {
        return;
    }

    End();
}

// ------------------------------------------------------------------------------------------------
bool AirdropWindow::Begin(ImGuiWindowFlags flags) { return Window::Begin(flags); }

// ------------------------------------------------------------------------------------------------
void AirdropWindow::End() { Window::End(); }
