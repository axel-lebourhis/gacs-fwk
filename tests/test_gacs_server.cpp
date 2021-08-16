#include "gacs_fwk.hpp"
#include "test_common.hpp"

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

private:
    bool validate_header(gacs::message<MessageTypes>& msg)
    {
        switch(msg.header.id)
        {
            case MessageTypes::ServerPing:
                if(msg.header.size > sizeof(ServerPing::body))
                {
                    /* Size is too large, this could be bad or malicious client */
                    return false;
                }
                break;

            case MessageTypes::MessageAll:
                if(msg.header.size > MAX_MESSAGE_SIZE)
                {
                    /* The size of the message is suspicious, client is not valid */
                    return false;
                }
                break;

            /* ServerAccept, ServerDeny, ServerMessage: can only be sent by the server, client is not valid
             * default: all other message ID are not valid
             */
            case MessageTypes::ServerAccept:
            case MessageTypes::ServerDeny:
            case MessageTypes::ServerMessage:
            default:
                return false;
        }

        return true;
    }
};

int main(void)
{
    TestServer server(60000);

    server.start();

    while(1)
    {
        server.update(-1, true);
    }

    return 0;
}