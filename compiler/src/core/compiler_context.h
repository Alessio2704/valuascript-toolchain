#pragma once
#include <vector>
#include <unordered_map>
#include "compiler_settings.h"
#include "diagnostics_reporter.h"
#include "valuascript_exception.h"
#include "source_manager.h"
#include "token/comment_token.h"

namespace valuascript::compiler {

    class CompilerContext {
    public:
        CompilerSettings settings;
        DiagnosticReporter diagnostics;
        SourceManager source_manager;
        std::vector<CommentToken> comments;
        std::unordered_map<std::string, std::vector<CommentToken>> comments_by_file;

        void handle_error(const ValuaScriptException &ex);
        void add_comment(const std::string &file_path, CommentToken comment);
        void add_comments(const std::string &file_path, const std::vector<CommentToken> &new_comments);
        [[nodiscard]] const std::vector<CommentToken>& get_comments(const std::string &file_path) const;
    };
}
