#include <atomic>
#include <chrono>
#include <format>
#include <iostream>
#include <vector>

#include "EthLayer.h"
#include "IPv4Layer.h"
#include "IPv6Layer.h"
#include "Packet.h"
#include "PcapFileDevice.h"
#include "PcapLiveDevice.h"
#include "PcapLiveDeviceList.h"
#include "SystemUtils.h"
#include "TcpLayer.h"
#include "UdpLayer.h"
#include "cxxopts.hpp"
#include "json.hpp"
#include "mmdb.hpp"
#include "packet.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

namespace fs = std::filesystem;

std::string timespecToUtcString(const timespec& ts) {
    using namespace std::chrono;

    const auto tp = system_clock::from_time_t(ts.tv_sec) +
                    std::chrono::nanoseconds(ts.tv_nsec);
    const auto time_zone_utc = locate_zone("UTC");
    const auto zt = zoned_time(time_zone_utc, tp);
    return std::format("{:%Y-%m-%d %H%M%S}", zt);
}

void packet_callback(pcpp::RawPacket* raw_packet, pcpp::PcapLiveDevice* dev,
                     void* cookie) {
    auto frame_number = reinterpret_cast<uint64_t*>(cookie);
    using json = nlohmann::json;
    json packet_info;

    wireana::packet::Packet parsedPacket(raw_packet);
    pcpp::MacAddress eth_src;
    pcpp::MacAddress eth_dst;
    pcpp::IPAddress ip_src;
    pcpp::IPAddress ip_dst;
    uint16_t srcPort;
    uint16_t dstPort;
    bool eth_found = false;
    bool ip_found = false;
    bool tcp_found = false;

    packet_info["info"] = parsedPacket.getInfo();
    packet_info["frame_number"] = *frame_number;
    packet_info["timestamp"] =
        timespecToUtcString(raw_packet->getPacketTimeStamp());
    for (auto* curLayer = parsedPacket.getFirstLayer(); curLayer != nullptr;
         curLayer = curLayer->getNextLayer()) {
        switch (curLayer->getProtocol()) {
            case pcpp::Ethernet: {
                auto etherLayer = dynamic_cast<pcpp::EthLayer*>(curLayer);
                eth_src = etherLayer->getSourceMac();
                eth_dst = etherLayer->getDestMac();
                eth_found = true;
                break;
            }
            case pcpp::IPv4: {
                auto ip_layer = dynamic_cast<pcpp::IPLayer*>(curLayer);
                ip_src = ip_layer->getSrcIPAddress();
                ip_dst = ip_layer->getDstIPAddress();
                auto asn_src = wireana::mmdb::geolite2db_asn_ipv4.lookup_asn(
                    ip_src.toString());
                auto asn_dst = wireana::mmdb::geolite2db_asn_ipv4.lookup_asn(
                    ip_dst.toString());
                auto geo_src = wireana::mmdb::geolite2db_city_ipv4.lookup_city(
                    ip_src.toString());
                auto geo_dst = wireana::mmdb::geolite2db_city_ipv4.lookup_city(
                    ip_dst.toString());

                packet_info["src_ip"] = ip_src.toString();
                packet_info["src_location"] =
                    geo_src.has_value() ? geo_src->city : "N/A";
                packet_info["dst_ip"] = ip_dst.toString();
                packet_info["dst_location"] =
                    geo_dst.has_value() ? geo_dst->city : "N/A";
                packet_info["src_asn"] =
                    asn_src.has_value() ? std::to_string(asn_src->asn) : "N/A";
                packet_info["src_asn_org"] =
                    asn_src.has_value() ? asn_src->asn_org : "N/A";
                packet_info["dst_asn"] =
                    asn_dst.has_value() ? std::to_string(asn_dst->asn) : "N/A";
                packet_info["dst_asn_org"] =
                    asn_dst.has_value() ? asn_dst->asn_org : "N/A";

                ip_found = true;
                break;
            }
            case pcpp::IPv6: {
                auto ip_layer = dynamic_cast<pcpp::IPLayer*>(curLayer);
                ip_src = ip_layer->getSrcIPAddress();
                ip_dst = ip_layer->getDstIPAddress();
                auto asn_src = wireana::mmdb::geolite2db_asn_ipv6.lookup_asn(
                    ip_src.toString());
                auto asn_dst = wireana::mmdb::geolite2db_asn_ipv6.lookup_asn(
                    ip_dst.toString());
                auto geo_src = wireana::mmdb::geolite2db_city_ipv6.lookup_city(
                    ip_src.toString());
                auto geo_dst = wireana::mmdb::geolite2db_city_ipv6.lookup_city(
                    ip_dst.toString());

                packet_info["src_ip"] = ip_src.toString();
                packet_info["src_location"] =
                    geo_src.has_value() ? geo_src->city : "N/A";
                packet_info["dst_ip"] = ip_dst.toString();
                packet_info["dst_location"] =
                    geo_dst.has_value() ? geo_dst->city : "N/A";
                packet_info["src_asn"] =
                    asn_src.has_value() ? std::to_string(asn_src->asn) : "N/A";
                packet_info["src_asn_org"] =
                    asn_src.has_value() ? asn_src->asn_org : "N/A";
                packet_info["dst_asn"] =
                    asn_dst.has_value() ? std::to_string(asn_dst->asn) : "N/A";
                packet_info["dst_asn_org"] =
                    asn_dst.has_value() ? asn_dst->asn_org : "N/A";

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
    (*frame_number)++;
    spdlog::info("{}", packet_info.dump());
}

int main(int argc, char* argv[], char* envp[]) {
    fs::path executable_dir =
        wireana::utils::get_executable_path().parent_path();
    spdlog::info("Wireana at:{}", executable_dir.string());
    cxxopts::Options options("Wireana",
                             "A learning project for network analysis");
    options.add_options()("version", "Show version")("h,help", "Show help")(
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
        spdlog::critical("Input file is required and only required one");
        return -1;
    }

    fs::path pcap_file_path(args["input"].as<std::string>());
    if (!fs::exists(pcap_file_path)) {
        spdlog::critical("Input file {} does not exist",
                         pcap_file_path.string());
        return -1;
    }

    if (fs::is_directory(pcap_file_path)) {
        spdlog::critical("Input file {} is a directory",
                         pcap_file_path.string());
        return -1;
    }

    std::unique_ptr<pcpp::IFileReaderDevice> reader(
        pcpp::IFileReaderDevice::getReader(pcap_file_path.string()));
    if (reader == nullptr) {
        spdlog::critical("Cannot determine reader for file type");
        return -1;
    }

    if (!reader->open()) {
        spdlog::critical("Cannot open file {}", pcap_file_path.string());
        return -1;
    }

    uint64_t iface_index = 0;
    auto& pIfaceDeviceList = pcpp::PcapLiveDeviceList::getInstance();
    for (auto& iface : pIfaceDeviceList.getPcapLiveDevicesList()) {
#ifdef _WIN32
        spdlog::info("Iface: {} Name: {} Display name: {}", iface_index,
                     iface->getName(),
                     wireana::utils::get_iface_display_name(iface));
#else
        spdlog::info("Iface: {} Name: {}", iface_index,
                     wireana::utils::get_iface_display_name(iface));
#endif
        iface_index++;
    }

    uint64_t selected_iface = 0;
    std::cout << "Select interface: ";
    std::cin >> selected_iface;
    if (selected_iface >= pIfaceDeviceList.getPcapLiveDevicesList().size()) {
        spdlog::critical("Invalid interface selected");
        return -1;
    }
    auto* dev = pIfaceDeviceList.getPcapLiveDevicesList()[selected_iface];
    if (!dev->open()) {
        spdlog::critical("Cannot open interface");
        return -1;
    }

    pcpp::RawPacket raw_packet;
    uint64_t count = 0;
    dev->startCapture(packet_callback, &count);
    std::cin.get();
    std::cin.get();
    dev->stopCapture();
    spdlog::info("{} packets processed", count);
    return 0;
}
