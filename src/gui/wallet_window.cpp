#include "gui/wallet_window.hpp"

#include "wallet.hpp"

// todo: remove later
#include <iostream>

// ------------------------------------------------------------------------------------------------
WalletWindow::WalletWindow(std::string name, bool bShow, bool bCanClose)
    : Window(std::move(name), bShow, bCanClose)
{}

// ------------------------------------------------------------------------------------------------
void WalletWindow::Update(GLFWwindow* glfwWindow, double deltaTime) {}

// ------------------------------------------------------------------------------------------------
void WalletWindow::Render()
{
    if (!m_bShow || !Begin(ImGuiWindowFlags_None))
    {
        return;
    }

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("WalletTabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Account Balance"))
        {
            AccountBalanceTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Generation"))
        {
            WalletGenerationTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    End();
}

// ------------------------------------------------------------------------------------------------
void WalletWindow::WalletGenerationTab()
{
    // todo (wilricknl): temporary to get a gui up and running
    static Wallet wallet;

    if (ImGui::Button("Generate a new wallet"))
    {
        wallet = GenerateWallet();
    }

    ImGui::Text("Seed: %s", wallet.seed.c_str());
    ImGui::Text("Private key: %s", wallet.private_key.c_str());
    ImGui::Text("Public key: %s", wallet.public_key.c_str());
    ImGui::Text("Identity: %s", wallet.identity.c_str());

    if (ImGui::Button("Copy seed"))
    {
        ImGui::LogToClipboard();
        ImGui::LogText("%s", wallet.seed.c_str());
        ImGui::LogFinish();
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy identity"))
    {
        ImGui::LogToClipboard();
        ImGui::LogText("%s", wallet.identity.c_str());
        ImGui::LogFinish();
    }
}

// ------------------------------------------------------------------------------------------------
// todo: move to some GUI utility file or whatever
namespace
{

int PublicKeyFilter(ImGuiInputTextCallbackData* data)
{
    if (data->EventChar >= 'A' && data->EventChar <= 'Z')
    {
        data->EventChar += 'a' - 'A';
    }
    return !(data->EventChar >= 'a' && data->EventChar <= 'z');
}

int IpFilter(ImGuiInputTextCallbackData* data)
{
    return !(data->EventChar >= '0' && data->EventChar <= '9' || data->EventChar <= '.');
}

} // namespace

// ------------------------------------------------------------------------------------------------
void WalletWindow::AccountBalanceTab()
{
    ImGui::TextUnformatted("Account balance:");

    static char public_key[56] = "";
    ImGui::InputText(
        "Public Key",
        public_key,
        56,
        ImGuiInputTextFlags_CallbackCharFilter,
        PublicKeyFilter);

    static char ip_address[16] = "";
    ImGui::InputText(
        "Ip address",
        ip_address,
        16,
        ImGuiInputTextFlags_CallbackCharFilter,
        IpFilter);

    static char port[6] = "";
    ImGui::InputText("Port", port, 6, ImGuiInputTextFlags_CharsDecimal);

    if (ImGui::Button("Get balance"))
    {
        std::cout << "Implement getting the balance here" << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------
bool WalletWindow::Begin(ImGuiWindowFlags flags) { return Window::Begin(flags); }

// ------------------------------------------------------------------------------------------------
void WalletWindow::End() { Window::End(); }
