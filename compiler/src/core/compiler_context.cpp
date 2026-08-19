#include "compiler_context.h"

namespace valuascript::compiler {

    void CompilerContext::handle_error(const ValuaScriptException &ex) {
        diagnostics.report_error(ex);

        if (settings.fail_fast) {
            throw ex;
        }
    }

    void CompilerContext::add_comment(const std::string &file_path, CommentToken comment) {
        comments.push_back(comment);
        comments_by_file[file_path].push_back(std::move(comment));
    }

    void CompilerContext::add_comments(const std::string &file_path, const std::vector<CommentToken> &new_comments) {
        comments.insert(comments.end(), new_comments.begin(), new_comments.end());
        auto &file_vec = comments_by_file[file_path];
        file_vec.insert(file_vec.end(), new_comments.begin(), new_comments.end());
    }

    const std::vector<CommentToken>& CompilerContext::get_comments(const std::string &file_path) const {
        static const std::vector<CommentToken> empty_comments{};
        auto it = comments_by_file.find(file_path);
        if (it != comments_by_file.end()) {
            return it->second;
        }
        return empty_comments;
    }
}