#pragma once

#include "network/server_request.h"

namespace voxeng::network {

class IServerConnection {
    virtual void pushRequest(ServerRequest request) = 0;
    virtual  pollEvent(
};

}
