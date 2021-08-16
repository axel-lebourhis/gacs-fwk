#pragma once

#include "gacs_common.hpp"
#include "gacs_tsqueue.hpp"
#include "gacs_connection.hpp"
#include "gacs_message.hpp"

namespace gacs
{
    template<typename T>
    class server_interface
    {
    public:
        server_interface(uint16_t port)
            : asioAcceptor_(asioContext_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
        {

        }

        virtual ~server_interface()
        {
            stop();
        }

        bool start()
        {
            try
            {
                /* Send work to ASIO context before running the thread so it doesn't exit immediately */
                wait_for_connection();

                /* Run ASIO thread */
                asioThread_ = std::thread(
                    [this]()
                    {
                        asioContext_.run();
                    }
                );
            }
            catch(const std::exception& e)
            {
                std::cerr << "[SERVER] Exception: " <<  e.what() << '\n';
                return false;
            }

            std::cout << "[SERVER] Server started !\n";
            return true;
        }

        void stop()
        {
            asioContext_.stop();
            if(asioThread_.joinable())
            {
                asioThread_.join();
            }
            std::cout << "[SERVER] Server stopped !\n";
        }

        void wait_for_connection()
        {
            asioAcceptor_.async_accept(
                [this](std::error_code ec, asio::ip::tcp::socket socket)
                {
                    if(!ec)
                    {
                        std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

                        std::shared_ptr<connection<T>> newconn =
                            std::make_shared<connection<T>>(
                                connection<T>::owner::server,
                                asioContext_,
                                std::move(socket),
                                inMessageQ_
                            );

                        if(on_client_connect(newconn))
                        {
                            deqConnections_.push_back(std::move(newconn));
                            deqConnections_.back()->connect_to_client(IDCounter_++);

                            std::cout << "[" << deqConnections_.back()->get_id() << "] Connection approved\n";
                        }
                        else
                        {
                            std::cout << "[-] Connection denied\n";
                        }
                    }
                    else
                    {
                        std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
                    }

                    wait_for_connection();
                }
            );
        }

        void message_client(std::shared_ptr<connection<T>> client, const message<T>& msg)
        {
            if(client && client->is_connected())
            {
                client->send(msg);
            }
            else
            {
                /* Connection is no more valid, we call for disconnect */
                on_client_disconnect(client);
                client.reset();
                deqConnections_.erase(std::remove(deqConnections_.begin(), deqConnections_.end(), client), deqConnections_.end());
            }
        }

        void message_all_clients(const message<T>& msg, std::shared_ptr<connection<T>> ignoreClient = nullptr)
        {
            bool invalidClientExists = false;

            for(auto& client : deqConnections_)
            {
                if(client && client->is_connected())
                {
                    if(client != ignoreClient)
                    {
                        client->send(msg);
                    }
                }
                else
                {
                    /* Connection is no more valid, we call for disconnect */
                    on_client_disconnect(client);
                    client.reset();
                    invalidClientExists = true;
                }
            }

            if(invalidClientExists)
            {
                deqConnections_.erase(std::remove(deqConnections_.begin(), deqConnections_.end(), nullptr), deqConnections_.end());
            }
        }

        void update(size_t nMaxMessages = -1, bool wait = false)
        {
            size_t count = 0;

            if(wait)
            {
                inMessageQ_.wait();
            }

            while(count < nMaxMessages && !inMessageQ_.empty())
            {
                auto ownedMsg = inMessageQ_.pop_front();

                on_message(ownedMsg.remote, ownedMsg.msg);

                count++;
            }
        }

    protected:

        virtual bool on_client_connect(std::shared_ptr<connection<T>> client)
        {
            /* This function can be overriden by the user in order to filter the incoming connections
             * By default, we accept any client (careful with security) */
            return true;
        }

        virtual void on_client_disconnect(std::shared_ptr<connection<T>> client)
        {

        }

        virtual void on_message(std::shared_ptr<connection<T>> client, message<T>& msg)
        {

        }

    protected:
        uint32_t IDCounter_ = 0;

        /* Thread safe queue for incoming messages */
        tsqueue<owned_message<T>> inMessageQ_;

        /* Queue of active connections */
        std::deque<std::shared_ptr<connection<T>>> deqConnections_;

        /* ASIO members */
        asio::io_context        asioContext_;
        std::thread             asioThread_;
        asio::ip::tcp::acceptor asioAcceptor_;
    };
}