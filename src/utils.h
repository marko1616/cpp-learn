#include <filesystem>

namespace fs = std::filesystem;

namespace wireana {
namespace utils {
fs::path get_executable_path();
}
}