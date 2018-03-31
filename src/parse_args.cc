#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>

#include <gflags/gflags.h>

#include "parse_args.h"

using boost::asio::ip::tcp;

DEFINE_string(bind_address, "::", "");
DEFINE_uint32(bind_port, 0, "");
DEFINE_string(target, "", "");
DEFINE_uint32(target_port, 0, "");

static bool ValidatePort(const char *flagname, uint32_t value);

DEFINE_validator(bind_port, ValidatePort);
DEFINE_validator(target_port, ValidatePort);

void ParseArgs(int *argc, char **argv[], tcp::endpoint *bind_ep, TargetInfo *target) {
    google::ParseCommandLineFlags(argc, argv, true);
    boost::system::error_code ec;
    auto address = boost::asio::ip::make_address(FLAGS_bind_address, ec);
    if (ec) {
        std::cerr << "Invalid bind address" << std::endl;
        exit(-1);
    }
    *bind_ep = tcp::endpoint(address, FLAGS_bind_port);
    address = boost::asio::ip::make_address(FLAGS_target, ec);
    if (ec) {
        target->SetTarget(FLAGS_target, FLAGS_target_port);
    } else {
        target->SetTarget(address, FLAGS_target_port);
    }
}

bool ValidatePort(const char *flagname, uint32_t value) {
    if (value > 0 && value < 65536) {
        return true;
    }
    std::cerr << flagname << " not available" << std::endl;
    return false;
}

