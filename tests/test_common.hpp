#pragma once

#include "gacs_fwk.hpp"

/* to be defined accordingly */
#define MAX_MESSAGE_SIZE 1024

enum class MessageTypes : uint32_t { ServerAccept, ServerDeny, ServerPing, MessageAll, ServerMessage };

struct ServerPing {
    gacs::message_header<MessageTypes> header = {.id = MessageTypes::ServerPing};
    union {
        std::chrono::system_clock::time_point timestamp;
        uint8_t bytes[1];
    } body;
};
