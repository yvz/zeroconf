#include <iostream>
#include <vector>

#ifdef WIN32
#define thread_local __declspec(thread)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#endif

#include "zeroconf.hpp"

int main(int argc, char** argv)
{
#ifdef WIN32
    WSADATA wsa;
    int st = WSAStartup(0x202, &wsa);
    if (st != 0)
    {
        std::cout << "E: Unable to initialize WinSock as " << st << std::endl;
        return 1;
    }
#endif

    std::vector<Zeroconf::mdns_responce> result;
    Zeroconf::Resolve("_http._tcp.local.", 3, &result);

#ifdef WIN32
    WSACleanup();
#endif

    return 0;
}
