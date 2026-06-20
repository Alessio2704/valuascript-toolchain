#include "reserved_keyword_lookup.h"

namespace valuascript::shared
{
    const std::unordered_map<std::string, TokenType>& get_reserved_keywords()
    {
        static const std::unordered_map<std::string, TokenType> kReservedKeywords = {
            {"import", TokenType::Import},
            {"let", TokenType::Let},
            {"if", TokenType::If},
            {"then", TokenType::Then},
            {"else", TokenType::Else},
            {"true", TokenType::True},
            {"false", TokenType::False},
            {"and", TokenType::And},
            {"or", TokenType::Or},
            {"not", TokenType::Not},
            {"mod", TokenType::Mod},
            {"func", TokenType::Func},
            {"struct", TokenType::Struct},
            {"enum", TokenType::Enum},
            {"switch", TokenType::Switch},
            {"case", TokenType::Case},
            {"default", TokenType::Default},
            {"self", TokenType::Self},
            {"typealias", TokenType::Typealias},
            {"extension", TokenType::Extension},
            {"return", TokenType::Return},
        };
        return kReservedKeywords;
    }

    std::optional<TokenType> get_keyword_type(const std::string& lexeme)
    {
        auto& keywords = get_reserved_keywords();
        if (const auto it = keywords.find(lexeme); it != keywords.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    bool is_reserved_keyword(const Token& token)
    {
        if (const auto res = get_keyword_type(token.lexeme); res.has_value())
        {
            return true;
        }
        return false;
    }

    std::vector<std::string> get_all_reserved_keyword_strings()
    {
        auto& keywords_map = get_reserved_keywords();
        std::vector<std::string> keywords;
        keywords.reserve(keywords_map.size());
        for (const auto& [text, type] : keywords_map)
        {
            keywords.push_back(text);
        }
        return keywords;
    }
}
