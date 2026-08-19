#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "ast/ast.h"

namespace valuascript::compiler {
    struct ResolvedProjectArtifact {
        std::string entry_file_path = {};
        std::unordered_map<std::string, std::shared_ptr<Program>> modules = {};
        std::vector<std::string> topological_order = {};
        std::unordered_map<std::string, std::vector<std::string>> reverse_imports = {};
    };
}
