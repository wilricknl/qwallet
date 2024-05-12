#include "gui/wallet_window.hpp"

#include <cstring>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>

#include "core/four_q.h"
#include "network/connection.hpp"
#include "network/entity.hpp"
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

void TextPopUp(const std::string& label, const std::string& warning)
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowSize(ImVec2{300.0f, 90.0f}, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(label.c_str()))
    {
        ImGui::TextWrapped("%s", warning.c_str());

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
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
 * @param stopBruteForce Atomic boolean to cancel the brute force operation
 * @param prefix The prefix to look for
 * @param outResult The variable to write the result too
 * @param resultMutex Mutex to safely write the result
 */
void BruteForceWalletThread(
    std::atomic<bool>& stopBruteForce,
    std::string prefix,
    Wallet& outResult,
    std::mutex& resultMutex)
{
    Wallet prefixed_wallet = {};
    while (!GenerateWalletWithPrefix(prefixed_wallet, prefix))
    {
        if (stopBruteForce)
        {
            return;
        }
    }

    // make other threads stop
    stopBruteForce = true;

    {
        std::lock_guard<std::mutex> guard(resultMutex);
        outResult = prefixed_wallet;
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
        if (ImGui::Button("Cancel"))
        {
            m_stopBruteForce = true;
            m_bWaitingForBruteForce = false;
        }
        else if (m_bruteForceFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            wallet = m_bruteForceFuture.get();
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
                    auto maxThreads = std::thread::hardware_concurrency();
                    if (maxThreads > 1)
                    {
                        // leave 1 for main program/os
                        maxThreads--;
                    }

                    Wallet result;
                    std::mutex resultMutex;

                    // start threads for bruteforce
                    std::vector<std::thread> threads;
                    for (unsigned int thread_id = 0; thread_id < maxThreads; ++thread_id)
                    {
                        auto thread = std::thread(
                            BruteForceWalletThread,
                            std::ref(m_stopBruteForce),
                            prefix,
                            std::ref(result),
                            std::ref(resultMutex));
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
    ImGui::Text("Private key: %s", wallet.privateKey.c_str());
    ImGui::Text("Public key: %s", wallet.publicKey.c_str());
    ImGui::Text("Identity: %s", wallet.identity.c_str());

    if (ImGui::Button("Copy seed"))
    {
        ImGui::LogToClipboard();
        ImGui::LogText("%s", wallet.seed.c_str());
        ImGui::LogFinish();
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy private key"))
    {
        ImGui::LogToClipboard();
        ImGui::LogText("%s", wallet.privateKey.c_str());
        ImGui::LogFinish();
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy public key"))
    {
        ImGui::LogToClipboard();
        ImGui::LogText("%s", wallet.publicKey.c_str());
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
    static long long accountBalance = 0;
    ImGui::Text("Account balance: %s", ToCommaSeparatedString(accountBalance).c_str());

    // identity
    static char identity[61] = "";
    ImGui::InputText(
        "Identity",
        identity,
        61,
        ImGuiInputTextFlags_CallbackCharFilter,
        UppercaseFilter);

    // ip-address
    static auto textColor = IM_COL32(255, 0, 0, 255);
    static char ipAddress[16] = "";

    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    bool bInput = ImGui::InputText(
        "Ip address",
        ipAddress,
        16,
        ImGuiInputTextFlags_CallbackCharFilter,
        IpFilter);
    ImGui::PopStyleColor();

    if (bInput)
    {
        if (IsValidIp(ipAddress))
        {
            textColor = IM_COL32(0, 255, 0, 255); // green
        }
        else
        {
            textColor = IM_COL32(255, 0, 0, 255); // red
        }
    }

    // port
    static char port[6] = "21841";
    ImGui::InputText("Port", port, 6, ImGuiInputTextFlags_CharsDecimal);

    // async balance request
    static bool bWaitingForBalance = false;
    static std::future<long long> balanceFuture;

    if (bWaitingForBalance)
    {
        if (balanceFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            accountBalance = balanceFuture.get();
            bWaitingForBalance = false;
        }
    }

    if (ImGui::Button("Get balance") && !bWaitingForBalance)
    {
        bWaitingForBalance = true;
        balanceFuture = std::async(std::launch::async, [&]() -> long long {
            // Create connection
            auto result = CreateConnection(ipAddress, atoi(port));
            if (result.has_value())
            {
                auto connection = result.value();
                auto response = GetBalance(connection, identity);
                if (response.has_value())
                {
                    return response.value();
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
    static int offset = 10;
    ImGui::SliderInt("Tick offset", &offset, 3, 100, "%d", ImGuiSliderFlags_AlwaysClamp);

    ImGui::SameLine();
    HelpMarker("Number of ticks in the future at which the transaction must be executed");

    // ip-address
    static auto textColor = IM_COL32(255, 0, 0, 255);
    static char ipAddress[16] = "";

    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    bool bInput = ImGui::InputText(
        "Ip address",
        ipAddress,
        16,
        ImGuiInputTextFlags_CallbackCharFilter,
        IpFilter);
    ImGui::PopStyleColor();

    if (bInput)
    {
        if (IsValidIp(ipAddress))
        {
            textColor = IM_COL32(0, 255, 0, 255); // green
        }
        else
        {
            textColor = IM_COL32(255, 0, 0, 255); // red
        }
    }

    // port
    static char port[6] = "21841";
    ImGui::InputText("Port", port, 6, ImGuiInputTextFlags_CharsDecimal);

    static std::string warningLabel{"Warning"};
    static std::string warningText;
    if (ImGui::Button("Send"))
    {
        auto result = VerifyTransactionInput(seed, recipientIdentity, ipAddress, port, amount);
        if (result.has_value())
        {
            ImGui::OpenPopup("Confirm transaction");
        }
        else
        {
            ImGui::OpenPopup(warningLabel.c_str());
            warningText = result.error().message;
        }
    }

    // Pop up for warnings
    TextPopUp(warningLabel, warningText);

    // transaction confirmation
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirm transaction"))
    {
        ImGui::TextWrapped(R"(Amount: "%s")", ToCommaSeparatedString(amount).c_str());
        ImGui::TextWrapped(
            R"(Recipient: "%.*s...%.*s")",
            5,
            recipientIdentity,
            5,
            recipientIdentity + 55);

        if (ImGui::Button("Send", ImVec2(120, 0)))
        {
            // todo: actual transaction logic goes here

            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
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

// ------------------------------------------------------------------------------------------------
tl::expected<bool, TransactionError> WalletWindow::VerifyTransactionInput(
    const std::string& seed,
    const std::string& recipient,
    const std::string& ipAddress,
    const std::string& port,
    unsigned long long amount)
{
    ConnectionPtr connection;
    {
        auto result = CreateConnection(ipAddress, atoi(port.c_str()));
        if (!result.has_value())
        {
            return tl::make_unexpected(TransactionError{result.error().message});
        }
        connection = result.value();
    }

    if (seed.size() != 55)
    {
        return tl::make_unexpected(TransactionError{"Seed is incomplete"});
    }

    if (recipient.size() != 60)
    {
        return tl::make_unexpected(TransactionError{"Recipient is incomplete"});
    }

    Wallet wallet;
    {
        auto result = GenerateWallet(seed);
        if (!result.has_value())
        {
            return tl::make_unexpected(TransactionError{result.error().message});
        }
        wallet = result.value();
    }

    // kinda slow to do it here, but at least it's proper :]
    // probably need to let a user "sign in" somewhere else, and periodically request balance,
    // or put this function in a future and open some popup with loading icon idk.
    unsigned long long balance = 0;
    {
        auto result = GetBalance(connection, wallet.identity);
        if (!result.has_value())
        {
            return tl::make_unexpected(TransactionError{result.error().message});
        }
        balance = result.value();
    }

    if (balance < amount)
    {
        return tl::make_unexpected(TransactionError{
            "Requested amount is too big. Your account has " + ToCommaSeparatedString(balance) + " qus."});
    }

    return true;
}
