#include <gacs_fwk.hpp>

enum class MessageTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage
};

class TestClient : public gacs::client_interface<MessageTypes>
{
};

int main(void)
{
    TestClient client;

    client.connect("127.0.0.1", 60000);

    bool exit = false;
    while(!exit)
    {
        if(client.is_connected())
        {
            if(!client.incoming().empty())
            {
                auto msg = client.incoming().pop_front().msg;

                switch (msg.header.id)
                {
                case MessageTypes::ServerAccept:
                    std::cout << "Server Accepted Connection\n";
                    break;

                case MessageTypes::ServerDeny:
                    std::cout << "Connection denied by server\n";
                    exit = true;
                    break;

                case MessageTypes::ServerPing:
                    std::cout << "Ping from server\n";
                    break;

                case MessageTypes::ServerMessage:
                    break;

                default:
                    std::cout << "Unknown message\n";
                    break;
                }
            }
        }
        else
        {
            std::cout << "Server is down\n";
            exit = true;
        }
    }

    // system("pause");

    return 0;
}