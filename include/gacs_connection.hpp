#pragma once

#include "gacs_common.hpp"
#include "gacs_message.hpp"
#include "gacs_tsqueue.hpp"

namespace gacs
{
    template<typename T>
    class connection : public std::enable_shared_from_this<connection<T>>
    {
    public:
        enum class owner
        {
            server,
            client
        };

        connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
            : asioContext_(asioContext), socket_(std::move(socket)), inMessageQ_(qIn)
        {

        }

        virtual ~connection()
        {}

        uint32_t get_id() const
        {
            return id_;
        }

        void connect_to_client(uint32_t uid = 0)
        {
            if(ownerType_ == owner::server)
            {
                if(socket_.is_open())
                {
                    id_ = uid;
                    read_header();
                }
            }
        }

        void connect_to_server(const asio::ip::tcp::resolver::results_type& endpoints)
        {
            if(ownerType_ == owner::client)
            {
                asio::async_connect(socket_, endpoints,
                    [this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
                    {
                        if(!ec)
                        {
                            read_header();
                        }
                    }
                );
            }
        }

        void disconnect()
        {
            if(is_connected())
            {
                asio::post(asioContext_,
                    [this]()
                    {
                        socket_.close();
                    }
                );
            }
        }

        bool is_connected() const
        {
            return socket_.is_open();
        }

        void send(const message<T>& msg)
        {
            asio::post(asioContext_,
                [this, msg]()
                {
                    bool was_not_empty = !outMessageQ_.empty();
                    outMessageQ_.push_back(msg);
                    if(!was_not_empty)
                    {
                        write_header();
                    }
                }
            );
        }

    private:
        asio::ip::tcp::socket socket_;
        asio::io_context& asioContext_;
        tsqueue<message<T>> outMessageQ_;
        tsqueue<owned_message<T>>& inMessageQ_;
        message<T> tempMessage_;
        owner ownerType_ = owner::server;
        uint32_t id_ = 0;

        void read_header()
        {

        }

        void read_body()
        {

        }

        void write_header()
        {

        }

        void write_body()
        {

        }
    };
}