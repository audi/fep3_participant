/**
 * @file
 * @copyright AUDI AG
 *            All right reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include "find_free_port.h"

#ifdef WIN32
    #include "winsock2.h"
    #define closeSocket(fd_socket) closesocket(fd_socket)
#else
    #include <iostream>
    #include <sys/socket.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <errno.h>
    #define SOCKET int
    #define closeSocket(fd_socket) close(fd_socket)
#endif

namespace fep3
{
namespace helper
{

class WindowsStartClean
{
private:
    WindowsStartClean()
    {
    }
    void init()
    {
#ifdef WIN32
        if (_result_startup == 0)
        {
            _result_startup = WSAStartup(MAKEWORD(2, 2), &_wsaData);
        }
#else
        _result_startup = 0;
#endif // WIN32
    }
    ~WindowsStartClean()
    {
        #ifdef WIN32
        if (_result_startup == 0)
        {
            WSACleanup();
        }
        #else   
        #endif
    }
public:
    static void initialize()
    {
        static WindowsStartClean wsa_initializer;
        wsa_initializer.init();
    }
    int _result_startup{-1};
    #ifdef WIN32
        WSADATA _wsaData = { 0 };
    #endif
};

int findFreeSocketPort(int begin_port, int count)
{
    WindowsStartClean::initialize();

    if (begin_port <= 0 || count < 0)
    {
        return -1;
    }

    struct sockaddr_in serv_addr;

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    opt = 1;
    #ifdef WIN32
    setsockopt(sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&opt, sizeof(opt));
    #endif 
    int found_port = -1;
    for (int currentport = begin_port;
         currentport <= currentport + count && found_port == -1;
         ++currentport)
    {
        unsigned short port_use = static_cast<unsigned short>(currentport);
        serv_addr.sin_port = htons(port_use);
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0)
        {
            //do not care which error occurs
        }
        else
        {
            found_port = currentport;
        }
    }

    if (closeSocket(sockfd) < 0)
    {
        return -1;
    }

    return found_port;
}

}
}
