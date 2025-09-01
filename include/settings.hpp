#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>

namespace Settings {
    constexpr std::string g_port { "3490" }; // port to connect to
    constexpr int g_pendingConnections { 10 }; // number of pending connecitons our host can listen to
    constexpr int g_messageMaxLength { 1024 }; // max length that a message can have
    constexpr int g_usernameMaxLength { 24 }; // max length of an username
}

#endif