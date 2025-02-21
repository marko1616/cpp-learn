#ifndef WIREANA_PACKET_HPP
#define WIREANA_PACKET_HPP

#include <iostream>
#include <optional>

#include "Packet.h"

namespace wireana {
namespace packet {

class Packet : public pcpp::Packet {
   public:
    explicit Packet(pcpp::RawPacket* rawPacket);
    std::string getInfo();
};
}  // namespace packet
}  // namespace wireana

#endif