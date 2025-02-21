#include <optional>

#include "Packet.h"
#include "EthLayer.h"
#include "IPv4Layer.h"
#include "IPv6Layer.h"
#include "TcpLayer.h"
#include "UdpLayer.h"

namespace wireana {
struct IPSummary
{
    pcpp::IPAddress src_ip;
    pcpp::IPAddress dst_ip;
    std::optional<std::string> ip_country;
    std::optional<std::string> ip_asn;
};


struct PacketSummary
{
    std::optional<IPSummary> ip_summary;
};

class Packet: public pcpp::Packet {
public:
    PacketSummary getSummary(bool fast = false) {
        PacketSummary summary;
        pcpp::IPAddress src_ip;
        pcpp::IPAddress dst_ip;
        auto ip_layer = getLayerOfType<pcpp::IPLayer>();
        if (ip_layer) {
            src_ip = ip_layer->getSrcIPAddress();
            dst_ip = ip_layer->getDstIPAddress();
            summary.ip_summary = IPSummary{src_ip, dst_ip, std::nullopt, std::nullopt};
        }
        return summary;
    }
};
}