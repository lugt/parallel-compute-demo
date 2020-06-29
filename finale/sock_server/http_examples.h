//
// Created by xc5 on 6/24/20.
//

#ifndef MEMORY_HTTP_EXAMPLES_H
#define MEMORY_HTTP_EXAMPLES_H

// System / Library Inclusion

#include "client_http.hpp"
#include "server_http.hpp"
#include <future>
// Added for the json-example
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
// Added for the default_resource example
#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#include "crypto.hpp"
#include "mmm_defs.h"

#define BUFFER_SIZE 4096000
#define LENGTH_OF_LISTEN_QUEUE 10

static const int SERVER_SOCKER_PORT = 8000;

enum ACTION_TYPE{
    ACTION_OCCUPY,
    ACTION_SAVE,
    ACTION_READ,
};

enum PROTO_DIRECTION{
    DIRECTION_SERVER_2_CLIENT,
    DIRECTION_CLIENT_2_SERVER,
};

struct PROTO_OBJ {
    PROTO_DIRECTION direction: 32;
    ACTION_TYPE action       : 32;
    UINT32 mem_id            : 32;
    UINT32 data_len          : 32;
    UINT64 occupy_len        : 64;
    char data[0];
};

using namespace boost::property_tree;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

using RESPONSE = shared_ptr<HttpServer::Response>;
using REQUEST = shared_ptr<HttpServer::Request>;

// Client group
void testing_bandwidth_with_timer(ELET_OFST length, const char *func_name, void (*func)(SPTR), SPTR span);

// Server group
INT32 tracedQuickSort(ELET *arr, ELET_OFST length, INT32 parallel_level,SPTR span);

#endif //MEMORY_HTTP_EXAMPLES_H
