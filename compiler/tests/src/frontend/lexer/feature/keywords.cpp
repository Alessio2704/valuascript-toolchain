#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "token/reserved_keyword_lookup.h"
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    class LexerKeywordTest : public LexerTestBase
    {
    };

    TEST_F(LexerKeywordTest, AllReservedKeywordsDynamic)
    {
        for (const auto& kw : valuascript::shared::RESERVED_KEYWORDS)
        {
            ExpectTokens(std::string(kw.name), {
                {
                    .type = kw.type,
                    .lexeme = std::string(kw.name),
                    .line = 1,
                    .column = 1,
                    .start_offset = 0,
                    .length = kw.name.length()
                }
            });
        }
    }

    TEST_F(LexerKeywordTest, AllKeywordsStreamSeparatedBySpaces)
    {
        std::string source;
        std::vector<ExpectedToken> expected;
        size_t current_offset = 0;
        size_t current_col = 1;

        for (const auto& kw : valuascript::shared::RESERVED_KEYWORDS)
        {
            if (!source.empty())
            {
                source += " ";
                current_offset += 1;
                current_col += 1;
            }
            expected.push_back({
                .type = kw.type,
                .lexeme = std::string(kw.name),
                .line = 1,
                .column = current_col,
                .start_offset = current_offset,
                .length = kw.name.length()
            });
            source += kw.name;
            current_offset += kw.name.length();
            current_col += kw.name.length();
        }

        ExpectTokens(source, expected);
    }

    TEST_F(LexerKeywordTest, AllKeywordsStreamSeparatedByNewlines)
    {
        std::string source;
        std::vector<ExpectedToken> expected;
        size_t current_offset = 0;
        size_t current_line = 1;

        for (const auto& kw : valuascript::shared::RESERVED_KEYWORDS)
        {
            if (!source.empty())
            {
                source += "\n";
                current_offset += 1;
                current_line += 1;
            }
            expected.push_back({
                .type = kw.type,
                .lexeme = std::string(kw.name),
                .line = current_line,
                .column = 1,
                .start_offset = current_offset,
                .length = kw.name.length()
            });
            source += kw.name;
            current_offset += kw.name.length();
        }

        ExpectTokens(source, expected);
    }

    TEST_F(LexerKeywordTest, KeywordBoundariesDynamic)
    {
        for (const auto& kw : valuascript::shared::RESERVED_KEYWORDS)
        {
            std::string name_with_suffix = std::string(kw.name) + "x";
            ExpectTokens(name_with_suffix, {
                {
                    .type = TokenType::Identifier,
                    .lexeme = name_with_suffix,
                    .line = 1,
                    .column = 1,
                    .start_offset = 0,
                    .length = name_with_suffix.length()
                }
            });
        }
    }

    TEST_F(LexerKeywordTest, KeywordsEmbeddedInIdentifiersDynamic)
    {
        for (const auto& kw : valuascript::shared::RESERVED_KEYWORDS)
        {
            std::string suffixed = std::string(kw.name) + "_val";
            ExpectTokens(suffixed, {
                {
                    .type = TokenType::Identifier,
                    .lexeme = suffixed,
                    .line = 1,
                    .column = 1,
                    .start_offset = 0,
                    .length = suffixed.length()
                }
            });

            std::string prefixed = "my_" + std::string(kw.name);
            ExpectTokens(prefixed, {
                {
                    .type = TokenType::Identifier,
                    .lexeme = prefixed,
                    .line = 1,
                    .column = 1,
                    .start_offset = 0,
                    .length = prefixed.length()
                }
            });
        }
    }
}
