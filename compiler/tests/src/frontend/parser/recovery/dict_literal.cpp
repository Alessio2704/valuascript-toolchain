#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ExprVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("DictMissingKey", "{ : 1, y: 2 }",
                {
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                IsDict({
                    {"<error>", {}, IsNumber("1")},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictMissingComma", "{ x: 1 y: 2 }",
                {
                    {ValuascriptErrorCode::ExpectedCommaSeparatorInDictionaryLiteral, 1, 8, 1, 9}
                },
                IsDict({
                    {"x", {}, IsNumber("1")},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictMissingExpressionValue", "{ x: , y: 2 }",
                {
                    {ValuascriptErrorCode::InvalidExpression, 1, 6, 1, 7}
                },
                IsDict({
                    {"x", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictBrokenExpressionValue", "{ x: *, y: 2 }",
                {
                    {ValuascriptErrorCode::InvalidExpression, 1, 6, 1, 7}
                },
                IsDict({
                    {"x", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("GarbageBetweenPairs", "{ x: 1, +, y: 2 }",
                {
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 9, 1, 10}
                },
                IsDict({
                    {"x", {}, IsNumber("1")},
                    {"<error>", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictEmptyComma", "{ , }",
                {
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                IsDict({
                    {"<error>", {}, IsNull()}
                })
            );

            reg("MissingBothKeyAndValue", "{ :, :, z: 3}",
                {
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 3, 1, 4},
                    {ValuascriptErrorCode::InvalidExpression, 1, 4, 1, 5},
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 6, 1, 7},
                    {ValuascriptErrorCode::InvalidExpression, 1, 7, 1, 8},
                },
                IsDict({
                    {"<error>", {}, IsNull()},
                    {"<error>", {}, IsNull()},
                    {"z", {}, IsNumber("3")},
                })
            );

            reg("MissingBothKeyAndValueVertical",
                "{\n"
                ":,\n"
                ":,\n"
                "z: 3\n}",
                {
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 2, 1, 2, 2},
                    {ValuascriptErrorCode::InvalidExpression, 2, 2, 2, 3},
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 3, 1, 3, 2},
                    {ValuascriptErrorCode::InvalidExpression, 3, 2, 3, 3},
                },
                IsDict({
                    {"<error>", {}, IsNull()},
                    {"<error>", {}, IsNull()},
                    {"z", {}, IsNumber("3")},
                })
            );

            reg("DoubleCommaBetweenPairs", "{ x: 1,, y: 2 }",
                {
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 8, 1, 9}
                },
                IsDict({
                    {"x", {}, IsNumber("1")},
                    {"<error>", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictKeyIsString", "{ \"x\": 1 }",
                {
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 3, 1, 6}
                },
                IsDict({
                    {"<error>", {}, IsNumber("1")},
                })
            );

            reg("DictKeyIsNumber", "{ 1: 1 }",
                {
                    {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                IsDict({
                    {"<error>", {}, IsNumber("1")},
                })
            );

            return true;
        }();
    }
}
