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

    return 0;
}