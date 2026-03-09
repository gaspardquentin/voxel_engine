#pragma once

#include "voxel_engine/network/i_client_connection.h"
#include "voxel_engine/network/local_transport.h"

#include <optional>

namespace voxeng::network {

class LocalClientConnection: public IClientConnection {
	LocalTransport& m_transport;

public:
	LocalClientConnection(LocalTransport& transport);
    void pushRequest(ServerRequest request);
    std::optional<ClientEvent> pollEvent();
};

}
