#include <chrono>

#include "gacs_fwk.hpp"
#include "test_common.hpp"

class TestClient : public gacs::client_interface<MessageTypes> {
   public:
    void ping_server() {
        ServerPing ping{};
        gacs::message<MessageTypes> msg;

        ping.body.timestamp = std::chrono::system_clock::now();

        msg.header.id = ping.header.id;
        msg << ping.body;
        send(msg);
    }

    void message_to_all_clients() {
        gacs::message<MessageTypes> msg;
        msg.header.id = MessageTypes::MessageAll;
        send(msg);
    }
};

int main(void) {
    TestClient client;

    client.connect("127.0.0.1", 60000);

    bool exit = false;
    bool key[3] = {false, false, false};
    bool old_key[3] = {false, false, false};

    while (!exit) {
        if (GetForegroundWindow() == GetConsoleWindow()) {
            key[0] = GetAsyncKeyState('1') & 0x8000;
            key[1] = GetAsyncKeyState('2') & 0x8000;
            key[2] = GetAsyncKeyState('3') & 0x8000;
        }

        if (key[0] && !old_key[0]) client.ping_server();
        if (key[1] && !old_key[1]) client.message_to_all_clients();
        if (key[2] && !old_key[2]) exit = true;

        for (int i = 0; i < 3; i++) old_key[i] = key[i];

        if (client.is_connected()) {
            if (!client.incoming().empty()) {
                auto msg = client.incoming().pop_front().msg;

                switch (msg.header.id) {
                    case MessageTypes::ServerAccept:
                        std::cout << "Server accepted connection\n";
                        break;

                    case MessageTypes::ServerDeny:
                        std::cout << "Connection denied by server\n";
                        exit = true;
                        break;

                    case MessageTypes::ServerPing: {
                        std::cout << "Ping from server\n";
                        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point then;
                        msg >> then;
                        std::cout << "Ping:" << std::chrono::duration<double>(now - then).count() << "\n";
                    } break;

                    case MessageTypes::ServerMessage: {
                        uint32_t client_id;
                        msg >> client_id;
                        std::cout << "Hello from client [" << client_id << "]\n";
                    } break;

                    default:
                        std::cout << "Unknown message\n";
                        break;
                }
            }
        } else {
            std::cout << "Server is down\n";
            exit = true;
        }
    }

    // system("pause");

    return 0;
}