#include "packet.hpp"

namespace wireana {
namespace packet {
Packet::Packet(pcpp::RawPacket* rawPacket) : pcpp::Packet(rawPacket) {}
std::string Packet::getInfo() {
    std::string info;
    for (auto* curLayer = getFirstLayer(); curLayer != nullptr;
         curLayer = curLayer->getNextLayer()) {
        if (curLayer->getProtocol() != pcpp::UnknownProtocol &&
            curLayer->getProtocol() != pcpp::GenericPayload) {
            info = curLayer->toString();
        }
    }
    return info;
}
}  // namespace packet
}  // namespace wireana