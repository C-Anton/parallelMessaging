#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <sys/types.h>
#include <sys/socket.h>

namespace Network {
    // Struct that packages a sockaddr_storage and its size
    struct AddressStorage {
        sockaddr_storage ss {};
        socklen_t size { sizeof(sockaddr_storage) };
    };

    void sendUsername(int fd);

    void handleReceives(int fd);

    void handleSends(int fd);
}

#endif