#include "gui/wallet_window.hpp"

#include <cstring>
#include <future>
#include <iostream>

#include "core/four_q.h"
#include "network/connection.hpp"
#include "network_messages/entity.h"
#include "utility.hpp"
#include "wallet.hpp"

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

int LowercaseFilter(ImGuiInputTextCallbackData* data)
{
    if (data->EventChar >= 'A' && data->EventChar <= 'Z')
    {
        data->EventChar += 'a' - 'A';
    }
    return !(data->EventChar >= 'a' && data->EventChar <= 'z');
}

int UppercaseFilter(ImGuiInputTextCallbackData* data)
{
    if (data->EventChar >= 'a' && data->EventChar <= 'z')
    {
        data->EventChar -= 'a' - 'A';
    }
    return !(data->EventChar >= 'A' && data->EventChar <= 'Z');
}

int IpFilter(ImGuiInputTextCallbackData* data)
{
    return !(data->EventChar >= '0' && data->EventChar <= '9' || data->EventChar <= '.');
}

} // namespace

// ------------------------------------------------------------------------------------------------
void WalletWindow::AccountBalanceTab()
{
    static long long account_balance = 0;
    ImGui::Text("Account balance: %s", to_comma_separated_string(account_balance).c_str());

    // identity
    static char identity[61] = "";
    ImGui::InputText(
        "Identity",
        identity,
        61,
        ImGuiInputTextFlags_CallbackCharFilter,
        UppercaseFilter);

    // ip-address
    static auto text_color = IM_COL32(255, 0, 0, 255);
    static char ip_address[16] = "";

    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    bool bInput = ImGui::InputText(
        "Ip address",
        ip_address,
        16,
        ImGuiInputTextFlags_CallbackCharFilter,
        IpFilter);
    ImGui::PopStyleColor();

    if (bInput)
    {
        if (IsValidIp(ip_address))
        {
            text_color = IM_COL32(0, 255, 0, 255); // green
        }
        else
        {
            text_color = IM_COL32(255, 0, 0, 255); // red
        }
    }

    // port
    static char port[6] = "";
    ImGui::InputText("Port", port, 6, ImGuiInputTextFlags_CharsDecimal);

    // async balance request
    static bool bWaitingForBalance = false;
    static std::future<long long> balance_future;

    if (bWaitingForBalance)
    {
        if (balance_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            account_balance = balance_future.get();
            bWaitingForBalance = false;
        }
    }

    if (ImGui::Button("Get balance") && !bWaitingForBalance)
    {
        bWaitingForBalance = true;
        balance_future = std::async(std::launch::async, [&]() -> long long {
            // Create connection
            auto result = CreateConnection(ip_address, atoi(port));
            if (result.has_value())
            {
                auto connection = result.value();

                // Send request
                struct
                {
                    RequestResponseHeader header;
                    RequestedEntity request;
                } packet;
                packet.header.setSize<sizeof(packet)>();
                packet.header.randomizeDejavu();
                packet.header.setType(REQUEST_ENTITY);

                unsigned char public_key[32];
                getPublicKeyFromIdentity((const unsigned char*)identity, public_key);
                memcpy(&packet.request.publicKey, public_key, 32);

                connection->Send((char*)&packet, sizeof(packet));

                // Receive response (todo: think about error check)
                auto response = connection->ReceiveAs<RespondedEntity>(RESPOND_ENTITY);

                return response.entity.incomingAmount - response.entity.outgoingAmount;
            }

            // todo: perhaps more interesting error reporting ;-)
            return -1;
        });
    }
}

// ------------------------------------------------------------------------------------------------
bool WalletWindow::Begin(ImGuiWindowFlags flags) { return Window::Begin(flags); }

// ------------------------------------------------------------------------------------------------
void WalletWindow::End() { Window::End(); }
