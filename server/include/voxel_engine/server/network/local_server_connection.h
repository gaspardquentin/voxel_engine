#pragma once

#include <optional>
#include "voxel_engine/network/i_server_connection.h"
#include "voxel_engine/network/local_transport.h"
namespace voxeng::network {

class LocalServerConnection: public IServerConnection {
	LocalTransport& m_transport;

public:
	LocalServerConnection(LocalTransport& transport);
    //TODO: should I use move semantics ???
	void pushEvent(ClientEvent event);
	std::optional<ServerRequest> pollRequest();
};

}
