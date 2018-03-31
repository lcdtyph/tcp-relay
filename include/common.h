#ifndef __COMMON_H__
#define __COMMON_H__

#include <boost/asio.hpp>
#include <boost/variant.hpp>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "buffer.h"

class TargetInfo {
public:
    using IpAddress = boost::asio::ip::address;
    bool NeedResolve() const {
        return state_ == 2;
    }

    bool IsEmpty() const {
        return state_ == 0;
    }

    void SetTarget(IpAddress ip, uint16_t port) {
        address_ = std::move(ip);
        port_ = port;
        state_ = 1;
    }

    void SetTarget(std::string host, uint16_t port) {
        address_ = std::move(host);
        port_ = port;
        state_ = 2;
    }

    std::string GetHostname() const {
        return boost::get<std::string>(address_);
    }

    IpAddress GetIp() const {
        return boost::get<IpAddress>(address_);
    }

    uint16_t GetPort() const {
        return port_;
    }

private:
    uint32_t state_ = 0;
    boost::variant<IpAddress, std::string> address_;
    uint16_t port_;
};

struct Peer {
    Peer(boost::asio::ip::tcp::socket socket)
        : socket(std::move(socket)) {
    }

    Peer(boost::asio::io_context &ctx)
        : socket(ctx) {
    }

    void CancelAll() {
        if (socket.is_open()) {
            socket.cancel();
        }
    }

    boost::asio::ip::tcp::socket socket;
    Buffer buf;
};

#endif

