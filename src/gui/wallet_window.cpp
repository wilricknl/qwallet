#include "gui/wallet_window.hpp"

#include <cstring>
#include <future>
#include <iostream>
#include <thread>

#include "core/four_q.h"
#include "network/connection.hpp"
#include "network_messages/entity.h"
#include "utility.hpp"

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

void HelpMarker(const std::string& description)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(description.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

} // namespace
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
WalletWindow::WalletWindow(std::string name, bool bShow, bool bCanClose)
    : Window(std::move(name), bShow, bCanClose)
{}

// ------------------------------------------------------------------------------------------------
WalletWindow::~WalletWindow()
{
    if (m_bWaitingForBruteForce)
    {
        // wait maximally 5 seconds for the brute force threads to clean up, if it is still running
        // while exiting
        m_stopBruteForce = true;
        m_bruteForceFuture.wait_for(std::chrono::seconds(5));
    }
}

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
        if (ImGui::BeginTabItem("Transaction"))
        {
            TransactionTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    End();
}

// ------------------------------------------------------------------------------------------------
/**
 * Thread to brute force a specific prefix
 * @param stop_brute_force Atomic boolean to cancel the brute force operation
 * @param prefix The prefix to look for
 * @param out_result The variable to write the result too
 * @param result_mutex Mutex to safely write the result
 */
void brute_force_wallet_thread(
    std::atomic<bool>& stop_brute_force,
    std::string prefix,
    Wallet& out_result,
    std::mutex& result_mutex)
{
    Wallet prefixed_wallet = {};
    while (!GenerateWalletWithPrefix(prefixed_wallet, prefix))
    {
        if (stop_brute_force)
        {
            return;
        }
    }

    // make other threads stop
    stop_brute_force = true;

    {
        std::lock_guard<std::mutex> guard(result_mutex);
        out_result = prefixed_wallet;
    }
}

// ------------------------------------------------------------------------------------------------
void WalletWindow::WalletGenerationTab()
{
    // todo (wilricknl): temporary to get a gui up and running
    static Wallet wallet;
    static bool bRequirePrefix = false;
    static bool bUseSeed = false;

    ImGui::SeparatorText("Generate wallet");

    if (ImGui::Checkbox("Require prefix", &bRequirePrefix))
    {
        // toggle off, if bRequirePrefix is turned on
        bUseSeed &= !bRequirePrefix;
    }

    ImGui::SameLine();
    HelpMarker("A prefix of more than 4 letters may take hours to compute");

    ImGui::SameLine();
    static char prefix[61] = "";
    ImGui::InputText("Prefix", prefix, 61, ImGuiInputTextFlags_CallbackCharFilter, UppercaseFilter);

    if (ImGui::Checkbox("Use seed", &bUseSeed))
    {
        // toggle off, if bUseSeed is turned on
        bRequirePrefix &= !bUseSeed;
    }
    ImGui::SameLine();
    static char seed[56] = "";
    ImGui::InputText(
        "Seed",
        seed,
        sizeof(seed),
        ImGuiInputTextFlags_CallbackCharFilter,
        LowercaseFilter);

    if (m_bWaitingForBruteForce)
    {
        if (m_bruteForceFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            wallet = m_bruteForceFuture.get();
            m_bWaitingForBruteForce = false;
        }

        if (ImGui::Button("Cancel"))
        {
            m_stopBruteForce = true;
            m_bWaitingForBruteForce = false;
        }
    }
    else
    {
        if (ImGui::Button("Generate"))
        {
            if (bUseSeed)
            {
                std::string seed_str{seed};
                if (seed_str.size() == 55)
                {
                    auto generated_wallet = GenerateWallet(seed);
                    if (generated_wallet.has_value())
                    {
                        wallet = generated_wallet.value();
                    }
                }
            }
            else if (bRequirePrefix)
            {
                m_bWaitingForBruteForce = true;

                // note: probably something smarter is possible, too busy and dumb to think about it
                // now
                m_bruteForceFuture = std::async(std::launch::async, [&]() -> Wallet {
                    auto max_threads = std::thread::hardware_concurrency();
                    if (max_threads > 1)
                    {
                        // leave 1 for main program/os
                        max_threads--;
                    }

                    Wallet result;
                    std::mutex result_mutex;

                    // start threads for bruteforce
                    std::vector<std::thread> threads;
                    for (unsigned int thread_id = 0; thread_id < max_threads; ++thread_id)
                    {
                        auto thread = std::thread(
                            brute_force_wallet_thread,
                            std::ref(m_stopBruteForce),
                            prefix,
                            std::ref(result),
                            std::ref(result_mutex));
                        threads.push_back(std::move(thread));
                    }

                    // join threads
                    for (auto& thread : threads)
                    {
                        thread.join();
                    }

                    // enforce reset stop brute force
                    m_stopBruteForce = false;

                    // return found result
                    return result;
                });
            }
            else
            {
                wallet = GenerateWallet();
            }
        }
    }

    ImGui::SeparatorText("Wallet");

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
    static char port[6] = "21841";
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

                // Receive response (todo: do something with connection error (maybe eat it))
                auto response = connection->ReceiveAs<RespondedEntity>(RESPOND_ENTITY);
                if (response.has_value())
                {
                    return response->entity.incomingAmount - response->entity.outgoingAmount;
                }
                std::cout << "Balance request failed: " << response.error().message << std::endl;
            }

            // todo: perhaps more interesting error reporting ;-)
            return -1;
        });
    }
}

// ------------------------------------------------------------------------------------------------
void WalletWindow::TransactionTab()
{
    ImGui::SeparatorText("Make transaction");

    static char seed[56] = "";
    ImGui::InputText("Seed", seed, 56, ImGuiInputTextFlags_CallbackCharFilter, LowercaseFilter);

    // recipient
    static char recipientIdentity[61] = "";
    ImGui::InputText(
        "Recipient",
        recipientIdentity,
        61,
        ImGuiInputTextFlags_CallbackCharFilter,
        UppercaseFilter);

    // amount
    static unsigned long long amount = 0;
    ImGui::InputScalar(
        "Amount",
        ImGuiDataType_U64,
        &amount,
        nullptr,
        nullptr,
        nullptr,
        ImGuiInputTextFlags_CharsDecimal);

    // tick offset
    static unsigned long long offset = 20;
    ImGui::InputScalar(
        "Tick offset",
        ImGuiDataType_U64,
        &offset,
        nullptr,
        nullptr,
        nullptr,
        ImGuiInputTextFlags_CharsDecimal);

    ImGui::SameLine();
    HelpMarker("Number of ticks in the future at which the transaction must be executed");

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
    static char port[6] = "21841";
    ImGui::InputText("Port", port, 6, ImGuiInputTextFlags_CharsDecimal);

    if (ImGui::Button("Send"))
    {
        ImGui::OpenPopup("Confirm transaction");
    }

    // transaction confirmation
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirm transaction"))
    {
        ImGui::TextWrapped(R"(Amount: "%s")", to_comma_separated_string(amount).c_str());
        ImGui::TextWrapped(
            R"(Recipient: "%.*s...%.*s")",
            5,
            recipientIdentity,
            5,
            recipientIdentity + 55);

        if (ImGui::Button("Send", ImVec2(120, 0)))
        {
            // todo: actual implementattion
            std::cout << "todo: Sending transaction to node" << std::endl;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            // todo: remove print
            std::cout << "Cancelling transaction" << std::endl;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::SeparatorText("Transaction history");

    // Transaction overview table
    if (ImGui::BeginTable("Transactions", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Sender");
        ImGui::TableSetupColumn("Recipient");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("Tick");
        ImGui::TableSetupColumn("Hash");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();

        // todo: row logic
        for (int i = 0; i < 4; ++i)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("AAA...BBB");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("CCC...DDD");
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("12345");
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("1245002");
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("0xABCDABCDABCDABCD");
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("Confirming");
        }

        ImGui::EndTable();
    }
}

// ------------------------------------------------------------------------------------------------
bool WalletWindow::Begin(ImGuiWindowFlags flags) { return Window::Begin(flags); }

// ------------------------------------------------------------------------------------------------
void WalletWindow::End() { Window::End(); }
