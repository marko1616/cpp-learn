#ifndef WIREANA_PACKET_HPP
#define WIREANA_PACKET_HPP

#include <optional>

#include "Packet.h"
#include "mmdb.hpp"

namespace wireana {
namespace packet {

class Packet : public pcpp::Packet {
   private:
    std::optional<wireana::mmdb::Geolite2CityData> geolite2CityData =
        std::nullopt;
    std::optional<wireana::mmdb::Geolite2ASNData> geolite2ASNData =
        std::nullopt;

   public:
    explicit Packet(pcpp::RawPacket* rawPacket);
    std::string getInfo();
    wireana::mmdb::Geolite2CityData getGeolite2CityData();
    wireana::mmdb::Geolite2ASNData getGeolite2ASNData();
};
}  // namespace packet
}  // namespace wireana

#endif