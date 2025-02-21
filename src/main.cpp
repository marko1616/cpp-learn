#include <iostream>
#include <vector>
#include <format>
#include <chrono>

#include "cxxopts.hpp"
#include "SystemUtils.h"
#include "Packet.h"
#include "EthLayer.h"
#include "IPv4Layer.h"
#include "IPv6Layer.h"
#include "TcpLayer.h"
#include "UdpLayer.h"
#include "PcapFileDevice.h"

namespace fs = std::filesystem;
namespace wireana {
std::string timespecToUtcString(const timespec& ts) {
    using namespace std::chrono;

    const auto tp = system_clock::from_time_t(ts.tv_sec) + nanoseconds(ts.tv_nsec);
    const auto time_zone_utc = locate_zone("UTC");
    const auto zt = zoned_time(time_zone_utc, tp);
    return std::format("{:%Y-%m-%d %H%M%S}", zt);
}

int main(int argc, char *argv[], char *envp[]) {
    cxxopts::Options options("Wireana", "A learning project for network analysis");
    options.add_options()
        ("h,help", "Show help")
        ("v,version", "Show version")
        ("i,input", "Input file", cxxopts::value<std::string>());
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
        std::cerr << std::format("Input file {} does not exist\n", pcap_file_path.string());
        return -1;
    }
    
    if(fs::is_directory(pcap_file_path)) {
        std::cerr << std::format("Input file {} is a directory\n", pcap_file_path.string());
        return -1;
    }

    std::unique_ptr<pcpp::IFileReaderDevice> reader(pcpp::IFileReaderDevice::getReader(pcap_file_path.string()));
    if (reader == nullptr) {
        std::cerr << "Cannot determine reader for file type\n";
        return -1;
    }

    if (!reader->open()) {
        std::cerr << std::format("Cannot open file {}\n", pcap_file_path.string());
        return -1;
    }

    pcpp::RawPacket raw_packet;
    uint64_t count = 0;
    while (reader->getNextPacket(raw_packet)) {
        std::cout << std::format("frame_number:{} Time:{}\n", count, timespecToUtcString(raw_packet.getPacketTimeStamp()));
        pcpp::Packet parsedPacket(&raw_packet);
        pcpp::MacAddress ethSrc;
        pcpp::MacAddress ethDst;
        pcpp::IPAddress ip_src;
        pcpp::IPAddress ip_dst;
        uint16_t srcPort;
        uint16_t dstPort;
        bool eth_found = false;
        bool ip_found = false;
        bool tcp_found = false;
        for (auto* curLayer = parsedPacket.getFirstLayer(); curLayer != nullptr; curLayer = curLayer->getNextLayer()) {
            switch(curLayer->getProtocol()) {
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
                    ip_found = true;
                    break;
                }
                case pcpp::IPv6: {
                    auto ip_layer = dynamic_cast<pcpp::IPLayer*>(curLayer);
                    ip_src = ip_layer->getSrcIPAddress();
                    ip_dst = ip_layer->getDstIPAddress();
                    ip_found = true;
                    break;
                }
                case pcpp::TCP: {
                    auto tcpLayer = dynamic_cast<pcpp::TcpLayer*>(curLayer);
                    srcPort = tcpLayer->getSrcPort();
                    dstPort = tcpLayer->getDstPort();
                    tcp_found = true;
                    break;
                }
            }
        }
        std::cout << std::format("  Ethernet: Src={}, Dst={}\n", eth_found ? ethSrc.toString() : "N/A", eth_found ? ethDst.toString() : "N/A");
        std::cout << std::format("  IP: Src={}, Dst={}\n", ip_found ? ip_src.toString() : "N/A", ip_found ? ip_dst.toString() : "N/A");
        std::cout << std::format("  TCP: SrcPort={}, DstPort={}\n", tcp_found ? std::to_string(srcPort) : "N/A", tcp_found ? std::to_string(dstPort) : "N/A");
        count++;
    }
    return 0;
}
}