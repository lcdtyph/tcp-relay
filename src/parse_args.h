#ifndef __PARSE_ARGS_H__
#define __PARSE_ARGS_H__

#include <boost/asio.hpp>

#include "common.h"

void ParseArgs(int *argc, char **argv[],
               boost::asio::ip::tcp::endpoint *bind_ep,
               TargetInfo *target);

#endif

