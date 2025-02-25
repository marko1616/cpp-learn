#ifndef WIREANA_UTILS_HPP
#define WIREANA_UTILS_HPP

#include <filesystem>

#include "PcapLiveDevice.h"
#include "spdlog/spdlog.h"

// After pcap++ to prevent windows.h from being included before winsock2
#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#elif __linux__
#include <limits.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace wireana {
namespace utils {
fs::path get_executable_path();
std::string get_iface_display_name(pcpp::PcapLiveDevice *const &iface);
}  // namespace utils
}  // namespace wireana

#endif  // WIREANA_UTILS_HPP