// std
#include <iostream>
#include <string>
#include <cstring> // for memset and strlen

// network unix headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h> // for inet_ntop
#include <sys/wait.h> // for wait()

// Local headers
#include "settings.hpp"
#include "network.hpp"

int main() {
    addrinfo hints {};

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* results {};

    int status { getaddrinfo(NULL, Settings::g_port.c_str(), &hints, &results) };

    if (status) {
        std::cerr << "ERROR getaddrinfo(): " << gai_strerror(status) << '\n';
        return 1;
    }


    int fdSocket { -1 };
    // search through all the addresses found in the linked list, get the first one that is valid (can make a socket fd and bind it)
    for (addrinfo* res { results }; res != NULL; res = res->ai_next) {
        fdSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fdSocket == -1)
            continue;


        int optVal {1}; // value for setsockopt()
        if (setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) == -1) {
            fdSocket = -1;
            continue;
        }

        if (bind(fdSocket, res->ai_addr, res->ai_addrlen) == -1) {
            fdSocket = -1;
            continue;
        }

        std::cout << "Setting up server on address "<< 1 << " ...\n";
        
        if (listen(fdSocket, Settings::g_pendingConnections) == -1) {
            std::cout << "Setup failed\n";
            fdSocket = -1;
            continue;
        }

        break;
    }

    if (results == NULL) {
        std::cout << "ERROR nothing found with the hostname given\n";
        return 2;
    }

    if (fdSocket == -1) {
        std::cerr << "ERROR socket(): " << strerror(errno) << '\n';
        return 2;
    }

    std::cout << "Server setup!\n";

    freeaddrinfo(results); // dont need results anymore

    Network::AddressStorage userAddress {};

    int fdUser {};
    
    std::cout << "Waiting for user to connect...\n";
    do {
        fdUser = accept(fdSocket, (sockaddr*)&userAddress.ss, &userAddress.size);
    } while (fdUser == -1);

    close(fdSocket); // Don't need the socket fd anymore

    std::cout << "User connected!\n";

    Network::sendUsername(fdSocket);

    int pid {};
    if (!fork()) { // child pid
        Network::handleSends(fdUser);
        exit(0);
    } else { // parent pid
        Network::handleReceives(fdUser);
        std::cout << "User has quit. Press anything to close the server.\n";
        wait(NULL);
    }

    close(fdUser); // Don't need the connecting users fd anymore

    std::cout << "Connection terminated\n";

    return 0;
}