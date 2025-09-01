#include "network.hpp"
#include "settings.hpp"

#include <string>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>

// Send the other host your username
void Network::sendUsername(int fd) {
    // Get the user, add as a prefix the "!username " command and send it to the server
    std::cout << "User name (can be re-set with !username <username>): ";
    std::string username {};
    std::getline(std::cin >> std::ws, username);
    username = std::string { "!username " } + username;

    send(fd, username.c_str(), username.size(), 0);

    std::cout << "User name sent!\n\n";
}

// Handles incoming message, such as printing them, changing usernames, etc.
void Network::handleReceives(int fd) {
    bool firstAccess { true }; // if the user just logged in, don't print their username change
    int bytesReceived {};
    std::string message {};

    std::string username {};
    username.resize(Settings::g_usernameMaxLength);

    do {
        message = ""; // empty the string from what recv put in
        message.resize(Settings::g_messageMaxLength);

        bytesReceived = recv(fd, message.data(), message.size(), 0);

        // If the user has used the !username command and typed in a new username, change their username and print it out
        if (message.starts_with("!username ") && message.length() > 10) {
            if (!firstAccess)
                std::cout << "\n\n" << username << " has changed their name to ";
            message.erase(0, 10); // remove the "!username " part, leaving the actual username
            
            username = message; // assign the username

            if (!firstAccess)
                std::cout << username << "\n\n";
            
            firstAccess = false;

            continue; // go to the next loop cycle
        }

        // If a message was sent, print it
        if (bytesReceived)
            std::cout << '\n' << username << ": " << message;

    } while (bytesReceived); // while the connection hasn't been terminated
}

// Handles sending messages, reports bytes sent
void Network::handleSends(int fd) {
    int bytesSent {};
    std::string message {};

    do {
        std::cout << "Send: ";
        std::getline(std::cin >> std::ws, message);

        bytesSent = send(fd, message.c_str(), message.size(), 0);
        std::cout << "Bytes sent: " << bytesSent << '\n';
    } while (bytesSent != -1);
}