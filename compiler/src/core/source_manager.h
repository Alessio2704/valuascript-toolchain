#pragma once
#include <map>
#include <string>
#include <string_view>
#include <optional>

namespace valuascript::compiler {
    using SourceRegistryMap = std::map<std::string, std::string, std::less<>>;

    class SourceManager {
    private:
        SourceRegistryMap sources_;

    public:
        SourceManager() = default;

        std::string_view register_source(std::string_view file_path, std::string source);
        void update_source(std::string_view file_path, std::string_view source);
        [[nodiscard]] std::optional<std::string_view> get_source(std::string_view file_path) const noexcept;
        [[nodiscard]] const SourceRegistryMap& get_all_sources() const noexcept;
        [[nodiscard]] bool contains(std::string_view file_path) const noexcept;
    };
}
