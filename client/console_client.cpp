#include <iostream>
#include <regex>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include "RakNetworkFactory.h"
#include "RakClientInterface.h"
#include "PacketEnumerations.h"
#include "BitStream.h"
#include "PacketPriority.h"
#include "SAMP/SAMPRPC.h"

using namespace RakNet;

static RakClientInterface *rakClient = nullptr;
static std::atomic<bool> connected{false};
static std::string nickname;
static std::string serverIp;
static unsigned short serverPort = 0;

bool validateNickname(const std::string &name)
{
    static const std::regex pattern("^[A-Za-z]+_[A-Za-z]+$");
    return std::regex_match(name, pattern);
}

bool parseAddress(const std::string &addr, std::string &ip, unsigned short &port)
{
    std::size_t pos = addr.find(':');
    if (pos == std::string::npos)
        return false;
    ip = addr.substr(0, pos);
    try
    {
        int p = std::stoi(addr.substr(pos + 1));
        if (p < 0 || p > 65535)
            return false;
        port = static_cast<unsigned short>(p);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

void connectServer()
{
    if (connected)
    {
        std::cout << "Already connected.\n";
        return;
    }
    rakClient = RakNetworkFactory::GetRakClientInterface();
    if (!rakClient)
    {
        std::cout << "Failed to create client.\n";
        return;
    }
    rakClient->SetPassword("");
    if (rakClient->Connect(serverIp.c_str(), serverPort, 0, 0, 5))
    {
        connected = true;
        std::cout << "Connected to " << serverIp << ":" << serverPort << "\n";
    }
    else
    {
        std::cout << "Connection failed." << std::endl;
        RakNetworkFactory::DestroyRakClientInterface(rakClient);
        rakClient = nullptr;
    }
}

void disconnectServer()
{
    if (!connected)
        return;
    rakClient->Disconnect(0);
    RakNetworkFactory::DestroyRakClientInterface(rakClient);
    rakClient = nullptr;
    connected = false;
    std::cout << "Disconnected." << std::endl;
}

void sendChat(const std::string &text)
{
    if (!connected)
    {
        std::cout << "Not connected." << std::endl;
        return;
    }
    BitStream bs;
    unsigned char len = static_cast<unsigned char>(text.size());
    bs.Write(len);
    bs.Write(text.c_str(), len);
    rakClient->RPC(&RPC_Chat, &bs, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, nullptr);
    std::cout << "Message sent." << std::endl;
}

void statusPrinter()
{
    while (true)
    {
        std::cout << "\n[" << (connected ? "Connected" : "Disconnected") << "]\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void inputNickname()
{
    std::string name;
    do
    {
        std::cout << "Enter nickname (Name_Surname): ";
        std::getline(std::cin, name);
    } while (!validateNickname(name));
    nickname = name;
}

void inputServer()
{
    std::string addr;
    do
    {
        std::cout << "Enter server address (ip:port): ";
        std::getline(std::cin, addr);
    } while (!parseAddress(addr, serverIp, serverPort));
}

int main()
{
    inputNickname();
    inputServer();

    std::thread(statusPrinter).detach();
    while (true)
    {
        std::cout << "\n1. Connect\n2. Disconnect\n3. Chat\n4. Change nickname\n5. Change server\n6. Quit\nSelect option: ";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "1")
        {
            connectServer();
        }
        else if (choice == "2")
        {
            disconnectServer();
        }
        else if (choice == "3")
        {
            while (true)
            {
                std::cout << "Chat (0 to return): ";
                std::string msg;
                std::getline(std::cin, msg);
                if (msg == "0")
                    break;
                sendChat(msg);
            }
        }
        else if (choice == "4")
        {
            inputNickname();
        }
        else if (choice == "5")
        {
            inputServer();
        }
        else if (choice == "6")
        {
            disconnectServer();
            break;
        }
        else
        {
            std::cout << "Invalid option." << std::endl;
        }
    }
    return 0;
}

