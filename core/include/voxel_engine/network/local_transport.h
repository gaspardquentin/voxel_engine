#pragma once

#include "client_event.h"
#include "server_request.h"
#include <mutex>
#include <queue>

namespace voxeng::network {

struct LocalTransport {
    std::mutex lock;
    std::queue<ServerRequest> requests;
    std::queue<ClientEvent> events;
};

}
