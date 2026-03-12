#include <mutex>
#include <optional>

#include "voxel_engine/client/network/local_client_connection.h"

using namespace voxeng::network;

LocalClientConnection::LocalClientConnection(LocalTransport& transport): m_transport(transport) {}

    //TODO: should I use move semantics ???
void LocalClientConnection::pushRequest(ServerRequest request) {
    std::lock_guard<std::mutex> guard(m_transport.lock);
    //TODO: should I use move semantics ???
    m_transport.requests.push(request);
}

std::optional<ClientEvent> LocalClientConnection::pollEvent() {
    std::lock_guard<std::mutex> guard(m_transport.lock);

    if (m_transport.events.empty()) {
        return std::nullopt;
    }

    ClientEvent evt = m_transport.events.front();
    m_transport.events.pop();
    return evt;
}
