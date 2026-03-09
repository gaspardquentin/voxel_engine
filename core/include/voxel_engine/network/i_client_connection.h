#pragma once

#include "client_event.h"
#include "server_request.h"
#include <optional>

namespace voxeng::network {

class IClientConnection {
public:
    virtual void pushRequest(ServerRequest request) = 0;
    virtual std::optional<ClientEvent> pollEvent() = 0;

    virtual ~IClientConnection() = default;
};

}
