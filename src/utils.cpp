#include "utils.hpp"

namespace fs = std::filesystem;

namespace wireana {
namespace utils {
#ifdef _WIN32
std::string extract_identifier(const std::string &devicePath) {
    std::string pathLower = devicePath;
    std::transform(pathLower.begin(), pathLower.end(), pathLower.begin(),
                   ::tolower);
    if (pathLower.find("npf_loopback") != std::string::npos) {
        return "Loopback";
    }
    size_t start = devicePath.find('{');
    size_t end = devicePath.find('}', start);
    if (start != std::string::npos && end != std::string::npos) {
        return devicePath.substr(start, end - start + 1);
    }
    return "";
}
#endif

fs::path get_executable_path() {
#ifdef _WIN32
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return fs::path(path);
#elif __APPLE__
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        return fs::path(path);
    }
#elif __linux__
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return fs::path(std::string(result, (count > 0) ? count : 0));
    }
#endif
    return {};
}

std::string get_iface_display_name(pcpp::PcapLiveDevice *const &iface) {
#ifdef _WIN32
    auto identifier = extract_identifier(iface->getName());
    const std::string base_path =
        "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-"
        "08002BE10318}";
    const std::string connection_subkey = "\\Connection";
    if (identifier == "Loopback") {
        return "Loopback";
    }

    HKEY hKey;
    std::string fullPath = base_path + "\\" + identifier + connection_subkey;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, fullPath.c_str(), 0, KEY_READ,
                      &hKey) != ERROR_SUCCESS) {
        spdlog::warn("Failed to open registry key: {}", fullPath);
        return "";
    }

    char name_buffer[256];
    DWORD buffer_size = sizeof(name_buffer);
    DWORD type;

    if (RegQueryValueExA(hKey, "Name", nullptr, &type, (LPBYTE)name_buffer,
                         &buffer_size) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        spdlog::warn("Failed to query registry value: {}", fullPath);
        return "";
    }

    RegCloseKey(hKey);
    return name_buffer;
#else
    return iface->getName();
#endif
}
}  // namespace utils
}  // namespace wireana
