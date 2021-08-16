#pragma once

#include "gacs_common.hpp"
#include "gacs_message.hpp"

namespace gacs
{
    template<typename T>
    struct message_header
    {
        T id{};
        uint32_t size = 0U;
    };

    template<typename T>
    struct message
    {
        message_header<T> header{};
        std::vector<uint8_t> body;

        size_t size() const
        {
            return body.size();
        }

        friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
        {
            os << "ID:" << int(msg.header.id) << " Size: " << msg.header.size;
            return os;
        }

        template<typename DataType>
        friend message<T>& operator << (message<T>& msg, const DataType& data)
        {
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into the message.");

            size_t i = msg.body.size();

            /* Allocates the space needed for new data and copy it */
            msg.body.resize(msg.body.size() + sizeof(DataType));
            std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

            /* Update the message size in the header */
            msg.header.size = msg.size();

            return msg;
        }

        template<typename DataType>
        friend message<T>& operator >> (message<T>& msg, DataType& data)
        {
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into the message.");

            size_t i = msg.body.size() - sizeof(DataType);

            std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

            msg.body.resize(i);

            /* Update the message size in the header */
            msg.header.size = msg.size();

            return msg;
        }
    };

    template<typename T>
    class connection;

    template<typename T>
    struct owned_message
    {
        std::shared_ptr<connection<T>> remote = nullptr;
        message<T> msg;

        friend std::ostream& operator << (std::ostream& os, const owned_message<T>& owned_msg)
        {
            os << owned_msg.msg;
            return os;
        }
    };
}