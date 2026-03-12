#pragma once

#include "client_event.h"
#include "server_request.h"
#include <optional>

namespace voxeng::network {

class IServerConnection {
public:
    virtual void pushEvent(ClientEvent event) = 0;
    virtual std::optional<ServerRequest> pollRequest() = 0;

    virtual ~IServerConnection() = default;
};

}
