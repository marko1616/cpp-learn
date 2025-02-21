#include <chrono>
#include <format>
#include <iostream>
#include <vector>

#include "EthLayer.h"
#include "IPv4Layer.h"
#include "IPv6Layer.h"
#include "Packet.h"
#include "PcapFileDevice.h"
#include "SystemUtils.h"
#include "TcpLayer.h"
#include "UdpLayer.h"
#include "cxxopts.hpp"
#include "json.hpp"
#include "mmdb.hpp"
#include "packet.hpp"
#include "utils.hpp"

namespace fs = std::filesystem;
std::string timespecToUtcString(const timespec& ts) {
    using namespace std::chrono;

    const auto tp =
        system_clock::from_time_t(ts.tv_sec) + nanoseconds(ts.tv_nsec);
    const auto time_zone_utc = locate_zone("UTC");
    const auto zt = zoned_time(time_zone_utc, tp);
    return std::format("{:%Y-%m-%d %H%M%S}", zt);
}

int main(int argc, char* argv[], char* envp[]) {
    auto executable_dir = wireana::utils::get_executable_path().parent_path();
    cxxopts::Options options("Wireana",
                             "A learning project for network analysis");
    options.add_options()("h,help", "Show help")("v,version", "Show version")(
        "i,input", "Input file", cxxopts::value<std::string>());
    auto args = options.parse(argc, argv);
    if (args.count("help")) {
        std::cout << options.help() << std::flush;
        return 0;
    }

    if (args.count("version")) {
        std::cout << "Wireana v0.0.0\n" << std::flush;
        return 0;
    }

    if (args.count("input") != 1) {
        std::cerr << "Input file is required and only required one\n";
        return -1;
    }

    fs::path pcap_file_path(args["input"].as<std::string>());
    if (!fs::exists(pcap_file_path)) {
        std::cerr << std::format("Input file {} does not exist\n",
                                 pcap_file_path.string());
        return -1;
    }

    if (fs::is_directory(pcap_file_path)) {
        std::cerr << std::format("Input file {} is a directory\n",
                                 pcap_file_path.string());
        return -1;
    }

    std::unique_ptr<pcpp::IFileReaderDevice> reader(
        pcpp::IFileReaderDevice::getReader(pcap_file_path.string()));
    if (reader == nullptr) {
        std::cerr << "Cannot determine reader for file type\n";
        return -1;
    }

    if (!reader->open()) {
        std::cerr << std::format("Cannot open file {}\n",
                                 pcap_file_path.string());
        return -1;
    }

    pcpp::RawPacket raw_packet;
    uint64_t count = 0;
    auto geolite2db_city_ipv4 = wireana::mmdb::Geolite2CityDB(
        executable_dir / "assets" / "geolite2-city-ipv4.mmdb");
    auto geolite2db_city_ipv6 = wireana::mmdb::Geolite2CityDB(
        executable_dir / "assets" / "geolite2-city-ipv6.mmdb");
    auto geolite2db_asn_ipv4 = wireana::mmdb::Geolite2ASNDB(
        executable_dir / "assets" / "geolite2-asn-ipv4.mmdb");
    auto geolite2db_asn_ipv6 = wireana::mmdb::Geolite2ASNDB(
        executable_dir / "assets" / "geolite2-asn-ipv6.mmdb");
    using json = nlohmann::json;
    while (reader->getNextPacket(raw_packet)) {
        json packet_info;

        wireana::packet::Packet parsedPacket(&raw_packet);
        pcpp::MacAddress ethSrc;
        pcpp::MacAddress ethDst;
        pcpp::IPAddress ip_src;
        pcpp::IPAddress ip_dst;
        uint16_t srcPort;
        uint16_t dstPort;
        bool eth_found = false;
        bool ip_found = false;
        bool tcp_found = false;

        packet_info["info"] = parsedPacket.getInfo();
        packet_info["frame_number"] = count;
        packet_info["timestamp"] =
            timespecToUtcString(raw_packet.getPacketTimeStamp());
        for (auto* curLayer = parsedPacket.getFirstLayer(); curLayer != nullptr;
             curLayer = curLayer->getNextLayer()) {
            switch (curLayer->getProtocol()) {
                case pcpp::Ethernet: {
                    auto etherLayer = dynamic_cast<pcpp::EthLayer*>(curLayer);
                    ethSrc = etherLayer->getSourceMac();
                    ethDst = etherLayer->getDestMac();
                    eth_found = true;
                    break;
                }
                case pcpp::IPv4: {
                    auto ip_layer = dynamic_cast<pcpp::IPLayer*>(curLayer);
                    ip_src = ip_layer->getSrcIPAddress();
                    ip_dst = ip_layer->getDstIPAddress();
                    auto asn_src =
                        geolite2db_asn_ipv4.lookup_asn(ip_src.toString());
                    auto asn_dst =
                        geolite2db_asn_ipv4.lookup_asn(ip_dst.toString());
                    auto geo_src =
                        geolite2db_city_ipv4.lookup_city(ip_src.toString());
                    auto geo_dst =
                        geolite2db_city_ipv4.lookup_city(ip_dst.toString());

                    packet_info["src_ip"] = ip_src.toString();
                    packet_info["src_location"] =
                        geo_src.has_value() ? geo_src->city : "N/A";
                    packet_info["dst_ip"] = ip_dst.toString();
                    packet_info["dst_location"] =
                        geo_dst.has_value() ? geo_dst->city : "N/A";
                    packet_info["asn"] = asn_src.has_value()
                                             ? std::to_string(asn_src->asn)
                                             : "N/A";
                    packet_info["asn_org"] =
                        asn_src.has_value() ? asn_src->asn_org : "N/A";

                    ip_found = true;
                    break;
                }
                case pcpp::IPv6: {
                    auto ip_layer = dynamic_cast<pcpp::IPLayer*>(curLayer);
                    ip_src = ip_layer->getSrcIPAddress();
                    ip_dst = ip_layer->getDstIPAddress();
                    auto asn_src =
                        geolite2db_asn_ipv6.lookup_asn(ip_src.toString());
                    auto asn_dst =
                        geolite2db_asn_ipv6.lookup_asn(ip_dst.toString());
                    auto geo_src =
                        geolite2db_city_ipv6.lookup_city(ip_src.toString());
                    auto geo_dst =
                        geolite2db_city_ipv6.lookup_city(ip_dst.toString());

                    packet_info["src_ip"] = ip_src.toString();
                    packet_info["src_location"] =
                        geo_src.has_value() ? geo_src->city : "N/A";
                    packet_info["dst_ip"] = ip_dst.toString();
                    packet_info["dst_location"] =
                        geo_dst.has_value() ? geo_dst->city : "N/A";
                    packet_info["asn"] = asn_src.has_value()
                                             ? std::to_string(asn_src->asn)
                                             : "N/A";
                    packet_info["asn_org"] =
                        asn_src.has_value() ? asn_src->asn_org : "N/A";

                    ip_found = true;
                    break;
                }
                case pcpp::TCP: {
                    auto tcpLayer = dynamic_cast<pcpp::TcpLayer*>(curLayer);
                    srcPort = tcpLayer->getSrcPort();
                    dstPort = tcpLayer->getDstPort();
                    packet_info["src_port"] = srcPort;
                    packet_info["dst_port"] = dstPort;
                    packet_info["protocol"] = "TCP";
                    tcp_found = true;
                    break;
                }
            }
        }
        std::cout << packet_info.dump(4) << std::endl;
        count++;
    }
    return 0;
}
