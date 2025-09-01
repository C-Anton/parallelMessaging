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

int main(int argc, char *argv[]) {
    std::string hostname {};

    if (argc == 1) {
        std::cout << "Write an hostname: ";
        std::cin >> hostname;
    }

    else if (argc > 2) {
        std::cerr << "ERROR: Too many arguments. Use either 'msgclient' or 'msgclient <hostname>\n'";
        return 1;
    }

    else
        hostname = argv[1];
    
    addrinfo hints {};

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* results {};

    int status { getaddrinfo(hostname.c_str(), Settings::g_port.c_str(), &hints, &results) };

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

        std::cout << "Server found, connecting...\n";

        if (connect(fdSocket, res->ai_addr, res->ai_addrlen) == -1) {
            std::cout << "Error: couldn't connect to server\n";
            fdSocket = -1;
            continue;
        }

        break;
    }

    if (fdSocket == -1) {
        std::cerr << "ERROR socket(): " << strerror(errno) << '\n';
        return 2;
    }

    std::cout << "Connected to server!\n";

    Network::sendUsername(fdSocket);

    int pid {};
    if (!fork()) { // child pid
        Network::handleSends(fdSocket);
        exit(0);
    } else { // parent pid
        Network::handleReceives(fdSocket);

        // Tell the server to close the connection
        send(fdSocket, 0, 0, 0);

        std::cout << "User has quit. Press anything to close the server.\n";
        wait(NULL);
    }

    close(fdSocket);

    freeaddrinfo(results);

    std::cout << "Disconnected from the server\n";

    return 0;
}