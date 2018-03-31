#ifndef __SERVER_H__
#define __SERVER_H__

#include <unordered_map>
#include <boost/asio.hpp>

#include "common.h"

class Session;

class ForwardServer {
    typedef boost::asio::ip::tcp tcp;
public:
    ForwardServer(boost::asio::io_context &ctx, tcp::endpoint server, TargetInfo target)
        : acceptor_(ctx, std::move(server)),
          target_(new TargetInfo(std::move(target))) {
        LOG(INFO) << "Server running at " << acceptor_.local_endpoint();
        running_ = true;
        DoAccept();
    }

    void Stop();

    bool Stopped() const {
        return !running_;
    }

private:
    void DoAccept();
    void ReleaseSession(Session *ptr);

    bool running_;
    tcp::acceptor acceptor_;
    std::shared_ptr<TargetInfo> target_;
    std::unordered_map<Session *, std::weak_ptr<Session>> sessions_;
};

#endif

