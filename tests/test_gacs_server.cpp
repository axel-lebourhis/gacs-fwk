#include <gacs_fwk.hpp>

enum class MessageTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage
};

class TestServer : public gacs::server_interface<MessageTypes>
{
public:
    TestServer(uint16_t port)
        : gacs::server_interface<MessageTypes>(port)
    {

    }

    virtual void on_message(std::shared_ptr<gacs::connection<MessageTypes>> client, gacs::message<MessageTypes>& msg)
    {
        switch(msg.header.id)
        {
        case MessageTypes::ServerPing:
            std::cout << "[" << client->get_id() << "] Ping from client\n";
            client->send(msg);
            break;

        case MessageTypes::MessageAll:
        {
            std::cout << "[" << client->get_id() << "] Message to all clients\n";

            gacs::message<MessageTypes> msg;
            msg.header.id = MessageTypes::ServerMessage;
            msg << client->get_id();
            message_all_clients(msg, client);
        }
            break;
        }
    }

    virtual void on_client_disconnect(std::shared_ptr<gacs::connection<MessageTypes>> client)
    {
        std::cout << "Client [" << client->get_id() << "] disconnected\n";
    }

    virtual bool on_client_connect(std::shared_ptr<gacs::connection<MessageTypes>> client)
    {
        gacs::message<MessageTypes> msg;
        msg.header.id = MessageTypes::ServerAccept;
        client->send(msg);
        return true;
    }
};

int main(void)
{
    TestServer server(60000);

    server.start();

    while(1)
    {
        server.update();
    }

    return 0;
}