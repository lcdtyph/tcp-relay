
#include <stdlib.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "common.h"
#include "server.h"
#include "parse_args.h"

int main(int argc, char *argv[]) {
    boost::asio::ip::tcp::endpoint ep;
    TargetInfo target;

    ParseArgs(&argc, &argv, &ep, &target);

    google::InitGoogleLogging(argv[0]);

    boost::asio::io_context ctx;

    ForwardServer server(ctx, ep, target);

    boost::asio::signal_set signals(ctx, SIGINT, SIGTERM);

    signals.async_wait(
        [&server](boost::system::error_code ec, int sig) {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }
            LOG(INFO) << "Signal: " << sig << " received";
            server.Stop();
        }
    );

    ctx.run();

    return 0;
}

