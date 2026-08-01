#include "source_manager.h"

namespace valuascript::compiler {

    std::string_view SourceManager::register_source(std::string_view file_path, std::string source) {
        auto [it, inserted] = sources_.insert_or_assign(std::string(file_path), std::move(source));
        return it->second;
    }

    void SourceManager::update_source(std::string_view file_path, std::string_view source) {
        sources_[std::string(file_path)] = std::string(source);
    }

    std::optional<std::string_view> SourceManager::get_source(std::string_view file_path) const noexcept {
        if (auto it = sources_.find(file_path); it != sources_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    const SourceRegistryMap& SourceManager::get_all_sources() const noexcept {
        return sources_;
    }

    bool SourceManager::contains(std::string_view file_path) const noexcept {
        return sources_.contains(file_path);
    }
}
