#include <mutex>
#include <optional>

#include "voxel_engine/server/network/local_server_connection.h"

using namespace voxeng::network;

LocalServerConnection::LocalServerConnection(LocalTransport& transport): m_transport(transport) {}

    //TODO: should I use move semantics ???
void LocalServerConnection::pushEvent(ClientEvent event) {
    std::lock_guard<std::mutex> guard(m_transport.lock);
    m_transport.events.push(event);
}

std::optional<ServerRequest> LocalServerConnection::pollRequest() {
    std::lock_guard<std::mutex> guard(m_transport.lock);
    if (m_transport.requests.empty()) {
        return std::nullopt;
    }

    ServerRequest req = m_transport.requests.front();
    m_transport.requests.pop();
    return req;
}
