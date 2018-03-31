
#include <utility>

#include "server.h"

using boost::asio::ip::tcp;
namespace bsys = boost::system;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, std::shared_ptr<TargetInfo> target_info)
        : context_(socket.get_executor().context()),
          target_info_(std::move(target_info)),
          client_(std::move(socket)),
          target_(context_),
          resolver_(context_) {
    }

    ~Session() {
        VLOG(2) << "Session completed";
    }

    void Start() {
        VLOG(2) << "Session start: " << client_.socket.remote_endpoint();
        auto self(shared_from_this());
        uint16_t port = target_info_->GetPort();
        if (target_info_->NeedResolve()) {
            std::string hostname = target_info_->GetHostname();
            VLOG(1) << "connecting to " << hostname;
            DoResolveTarget(std::move(hostname), std::to_string(port));
        } else {
            auto ep = tcp::endpoint(target_info_->GetIp(), port);
            VLOG(1) << "connecting to " << ep;
            DoConnectTarget(std::array<tcp::endpoint, 1>{ ep });
        }
    }

private:

    void DoResolveTarget(std::string host, std::string port) {
        auto self(shared_from_this());
        VLOG(2) << "Resolving to " << host << ":" << port;
        resolver_.async_resolve(
            host, port,
            [this, self](bsys::error_code ec, tcp::resolver::results_type results) {
                if (ec) {
                    VLOG(1) << "Unable to resolve: " << ec;
                    client_.CancelAll();
                    return;
                }
                DoConnectTarget(std::move(results));
            }
        );
    }

    template<class EndpointSequence>
    void DoConnectTarget(const EndpointSequence &results) {
        auto self(shared_from_this());
        boost::asio::async_connect(
            target_.socket, results,
            [this, self](bsys::error_code ec, tcp::endpoint ep) {
                if (ec) {
                    if (ec == boost::asio::error::operation_aborted) {
                        VLOG(1) << "Connect canceled";
                        return;
                    }
                    LOG(INFO) << "Cannot connect to remote: " << ec;
                    client_.CancelAll();
                    return;
                }
                VLOG(1) << "Connected to remote " << ep;
                StartStream();
            }
        );
    }

    void StartStream() {
        VLOG(1) << "Start streaming";
        DoRelayStream(client_, target_);
        DoRelayStream(target_, client_);
    }

    void DoRelayStream(Peer &src, Peer &dest) {
        auto self(shared_from_this());
        src.socket.async_read_some(
            src.buf.GetBuffer(),
            [this, self, &src, &dest](bsys::error_code ec, size_t len) {
                if (ec) {
                    if (ec == boost::asio::error::misc_errors::eof) {
                        VLOG(2) << "Stream terminates normally";
                        src.CancelAll();
                        dest.CancelAll();
                        return;
                    } else if (ec == boost::asio::error::operation_aborted) {
                        VLOG(1) << "Read operation canceled";
                        return;
                    }
                    LOG(WARNING) << "Relay read unexcepted error: " << ec;
                    src.CancelAll();
                    dest.CancelAll();
                    return;
                }
                src.buf.Append(len);
                boost::asio::async_write(dest.socket,
                    src.buf.GetConstBuffer(),
                    [this, self, &src, &dest](bsys::error_code ec, size_t len) {
                        if (ec) {
                            if (ec == boost::asio::error::operation_aborted) {
                                VLOG(1) << "Write operation canceled";
                                return;
                            }
                            LOG(WARNING) << "Relay write unexcepted error: " << ec;
                            src.CancelAll();
                            dest.CancelAll();
                            return;
                        }
                        src.buf.Reset();
                        DoRelayStream(src, dest);
                    }
                );
            }
        );
    }

    boost::asio::io_context &context_;
    std::shared_ptr<TargetInfo> target_info_;
    Peer client_;
    Peer target_;
    tcp::resolver resolver_;
};

void ForwardServer::DoAccept() {
    acceptor_.async_accept([this](bsys::error_code ec, tcp::socket socket) {
        if (!ec) {
            VLOG(1) << "A new client accepted: " << socket.remote_endpoint();
            std::make_shared<Session>(std::move(socket), target_)->Start();
        }
        if (running_) {
            DoAccept();
        }
    });
}

