#ifndef WIREANA_UTILS_HPP
#define WIREANA_UTILS_HPP

#include <filesystem>

namespace fs = std::filesystem;

namespace wireana {
namespace utils {
fs::path get_executable_path();
}  // namespace utils
}  // namespace wireana

#endif  // WIREANA_UTILS_HPP