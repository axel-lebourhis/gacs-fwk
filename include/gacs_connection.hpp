#pragma once

#include "gacs_common.hpp"
#include "gacs_message.hpp"
#include "gacs_tsqueue.hpp"

namespace gacs
{
    enum class ownerType
    {
        server,
        client
    };
    template<typename T>
    class owner
    {
    public:
        owner(ownerType type)
        {
            type_ = type;
        }

        virtual ~owner()
        {}

        ownerType get_type() const
        {
            return type_;
        }

        virtual bool validate_header(message<T>& msg) = 0;

    private:
        ownerType type_;
    };

    template<typename T>
    class connection : public std::enable_shared_from_this<connection<T>>
    {
    public:

        connection(owner<T>& parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
            : asioContext_(asioContext), socket_(std::move(socket)), inMessageQ_(qIn), owner_(parent)
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
            if(owner_.get_type() == ownerType::server)
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
            if(owner_.get_type() == ownerType::client)
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
                    /* we make the assumption that if the out message queue is not empty
                     * then ASIO is busy sending it */
                    bool isAlreadySending = !outMessageQ_.empty();
                    outMessageQ_.push_back(msg);
                    if(!isAlreadySending)
                    {
                        write_header();
                    }
                }
            );
        }

    private:
        bool validate_header(message<T>& msg)
        {
            return owner_.validate_header(msg);
        }

        void read_header()
        {
            asio::async_read(socket_, asio::buffer(&tempMessage_.header, sizeof(message_header<T>)),
                [this](std::error_code ec, std::size_t length)
                {
                    if(!ec)
                    {
                        /* Before allocating memory, we need to validate the header
                         * Bad or malicious client could make the server allocate a lot of memory
                         * this validation needs to be done by the owner (server) */
                        if(validate_header(tempMessage_))
                        {
                            if(tempMessage_.header.size > 0)
                            {
                                tempMessage_.body.resize(tempMessage_.header.size);
                                read_body();
                            }
                            else
                            {
                                add_to_incoming_message_queue();
                            }
                        }
                        else
                        {
                            /* Header is not valid, we reject the client */
                            std::cout << "[" << id_ << "]" << " Client is not valid, disconnect\n";
                            disconnect();
                        }
                    }
                    else
                    {
                        std::cout << "[" << id_ << "]" << " Read header failed\n";
                        socket_.close();
                    }
                }

            );
        }

        void read_body()
        {
            asio::async_read(socket_, asio::buffer(tempMessage_.body.data(), tempMessage_.body.size()),
                [this](std::error_code ec, std::size_t length)
                {
                    if(!ec)
                    {
                        add_to_incoming_message_queue();
                    }
                    else
                    {
                        std::cout << "[" << id_ << "]" << " Read body failed\n";
                        socket_.close();
                    }
                }
            );
        }

        void write_header()
        {
            asio::async_write(socket_, asio::buffer(&outMessageQ_.front().header, sizeof(message_header<T>)),
                [this](std::error_code ec, std::size_t length)
                {
                    if(!ec)
                    {
                        if(outMessageQ_.front().body.size() > 0)
                        {
                            write_body();
                        }
                        else
                        {
                            outMessageQ_.pop_front();

                            if(!outMessageQ_.empty())
                            {
                                write_header();
                            }
                        }
                    }
                    else
                    {
                        std::cout << "[" << id_ << "]" << " Write header failed\n";
                        socket_.close();
                    }
                }
            );
        }

        void write_body()
        {
            asio::async_write(socket_, asio::buffer(outMessageQ_.front().body.data(), outMessageQ_.front().body.size()),
                [this](std::error_code ec, std::size_t length)
                {
                    if(!ec)
                    {
                        outMessageQ_.pop_front();

                        if(!outMessageQ_.empty())
                        {
                            write_header();
                        }
                    }
                    else
                    {
                        std::cout << "[" << id_ << "]" << " Write body failed\n";
                        socket_.close();
                    }
                }
            );
        }

        void add_to_incoming_message_queue()
        {
            if(owner_.get_type() == ownerType::server)
            {
                inMessageQ_.push_back({ this->shared_from_this(), tempMessage_ });
            }
            else
            {
                inMessageQ_.push_back({ nullptr, tempMessage_ });
            }

            read_header();
        }

    protected:
        asio::ip::tcp::socket socket_;
        asio::io_context& asioContext_;
        tsqueue<message<T>> outMessageQ_;
        tsqueue<owned_message<T>>& inMessageQ_;
        message<T> tempMessage_;
        owner<T>& owner_;
        uint32_t id_ = 0;
    };
}