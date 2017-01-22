#include <string>
#include <iostream>
#include <vector>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "zeroconf.hpp"

namespace
{
    void* get_in_addr(sockaddr_storage* sa)
    {
        if (sa->ss_family == AF_INET)
            return &reinterpret_cast<sockaddr_in*>(sa)->sin_addr;

        if (sa->ss_family == AF_INET6)
            return &reinterpret_cast<sockaddr_in6*>(sa)->sin6_addr;

        return nullptr;
    }

    const std::string SeparatorLine(20, '-');
}

void PrintLog(Zeroconf::LogLevel level, const std::string& message)
{
    switch (level)
    {
        case Zeroconf::LogLevel::Error:
            std::cout << "E: " << message << std::endl;
            break;
        case Zeroconf::LogLevel::Warning:
            std::cout << "W: " << message << std::endl;
            break;
    }
}

void PrintResult(std::vector<Zeroconf::mdns_responce>& result)
{
    for (size_t i = 0; i < result.size(); i++)
    {
        auto& item = result[i];

        char buffer[INET6_ADDRSTRLEN + 1] = {0};
        inet_ntop(item.peer.ss_family, get_in_addr(&item.peer), buffer, INET6_ADDRSTRLEN);
        std::cout << "Peer: " << buffer << std::endl;

        if (!item.records.empty())
        {
            std::cout << "Answers:" << std::endl;
            for (size_t j = 0; j < item.records.size(); j++)
            {
                auto& rr = item.records[j];
                std::cout << " " << j;
                std::cout << ": type ";
                switch(rr.type)
                {
                    case  1: std::cout << "A"; break;
                    case 12: std::cout << "PTR"; break;
                    case 16: std::cout << "TXT"; break;
                    case 28: std::cout << "AAAA"; break;
                    case 33: std::cout << "SRV"; break;
                    default: std::cout << rr.type;
                }
                std::cout << ", size " << rr.len;
                std::cout << ", " << rr.name << std::endl;
            }
        }

        if (i != result.size() - 1)
            std::cout << SeparatorLine << std::endl;
    }
}

int main(int argc, char** argv)
{
#ifdef WIN32
    WSADATA wsa = {0};
    if (WSAStartup(0x202, &wsa) != 0)
    {
        std::cout << "E: Unable to initialize WinSock" << std::endl;
        return 1;
    }
#endif

    static const std::string MdnsQuery = "_http._tcp.local";
    std::cout << "Query: " << MdnsQuery << std::endl;

    Zeroconf::SetLogCallback(PrintLog);

    std::vector<Zeroconf::mdns_responce> result;
    bool st = Zeroconf::Resolve(MdnsQuery, /*scanTime*/ 3, &result);

    std::cout << SeparatorLine << std::endl;

    if (!st)
        std::cout << "MDNS query failed" << std::endl;
    else if (result.empty())
        std::cout << "No replies" << std::endl;
    else
        PrintResult(result);

    std::cout << SeparatorLine << std::endl;

#ifdef WIN32
    WSACleanup();
#endif

    return 0;
}
