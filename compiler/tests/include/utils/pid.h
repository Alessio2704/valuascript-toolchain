#pragma once

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include <string>
#include <filesystem>
#include <system_error>

namespace valuascript::compiler::test {
    inline int get_current_pid() {
#ifdef _WIN32
        return _getpid();
#else
        return getpid();
#endif
    }

    inline std::filesystem::path generate_test_workspace(const std::string &prefix, uintptr_t instance_ptr) {
        std::filesystem::path raw_path = std::filesystem::temp_directory_path() /
                                         (prefix + "_" +
                                          std::to_string(get_current_pid()) + "_" +
                                          std::to_string(instance_ptr));

        std::error_code ec;
        std::filesystem::create_directories(raw_path, ec);

        return std::filesystem::absolute(raw_path);
    }

    inline void cleanup_test_workspace(const std::filesystem::path &path) {
        if (path.empty()) return;
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
}
