#include "reserved_keyword_lookup.h"

namespace valuascript::shared {
    std::optional<TokenType> get_keyword_type(const std::string &lexeme) {
        if (const auto it = kReservedKeywords.find(lexeme); it != kReservedKeywords.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool is_reserved_keyword(const Token &token) {
        if (const auto res = get_keyword_type(token.lexeme); res.has_value()) {
            return true;
        }
        return false;
    }

    std::vector<std::string> get_all_reserved_keyword_strings() {
        std::vector<std::string> keywords;
        keywords.reserve(kReservedKeywords.size());
        for (const auto &[text, type]: kReservedKeywords) {
            keywords.push_back(text);
        }
        return keywords;
    }
}
