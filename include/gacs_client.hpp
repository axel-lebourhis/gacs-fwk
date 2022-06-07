#pragma once

#include "gacs_common.hpp"

namespace gacs {
template <typename T>
class client_interface : public owner<T> {
   public:
    client_interface() : owner<T>(ownerType::client) {}

    virtual ~client_interface() { disconnect(); }

    bool connect(const std::string &host, const uint16_t port) {
        try {
            asio::ip::tcp::resolver resolver(asioContext_);
            asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

            connection_ =
                std::make_unique<connection<T>>(*this, asioContext_, asio::ip::tcp::socket(asioContext_), inMessageQ_);

            connection_->connect_to_server(endpoints);

            threadContext_ = std::thread([this]() { asioContext_.run(); });
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            return false;
        }

        return true;
    }

    void disconnect() {
        if (is_connected()) {
            connection_->disconnect();
        }
    }

    bool is_connected() {
        if (connection_) {
            return connection_->is_connected();
        } else {
            return false;
        }
    }

    void send(const message<T> &msg) {
        if (is_connected()) {
            connection_->send(msg);
        }
    }

    tsqueue<owned_message<T>> &incoming() { return inMessageQ_; }

   protected:
    asio::io_context asioContext_;
    std::thread threadContext_;
    std::unique_ptr<connection<T>> connection_;

   private:
    tsqueue<owned_message<T>> inMessageQ_;

    bool validate_header(message<T> &msg) {
        /* We assume client doesn't need to validate server's packet */
        return true;
    }
};
}  // namespace gacs
