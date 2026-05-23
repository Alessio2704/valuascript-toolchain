#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

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
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                IsDict({
                    {"<error>", {}, IsNumber("1")},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictMissingComma", "{ x: 1 y: 2 }",
                {
                    {E::ExpectedCommaSeparatorInDictionaryLiteral, 1, 8, 1, 9}
                },
                IsDict({
                    {"x", {}, IsNumber("1")},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictMissingExpressionValue", "{ x: , y: 2 }",
                {
                    {E::InvalidExpression, 1, 6, 1, 7}
                },
                IsDict({
                    {"x", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictBrokenExpressionValues", "{ x: *, y: *, z: 3 }",
                {
                    {E::InvalidExpression, 1, 6, 1, 7},
                    {E::InvalidExpression, 1, 12, 1, 13},
                },
                IsDict({
                    {"x", {}, IsNull()},
                    {"y", {}, IsNull()},
                    {"z", {}, IsNumber("3")}
                })
            );

            reg("GarbageBetweenPairs", "{ x: 1, +, y: 2 }",
                {
                    {E::ExpectedDictionaryKey, 1, 9, 1, 10}
                },
                IsDict({
                    {"x", {}, IsNumber("1")},
                    {"<error>", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictEmptyComma", "{ , }",
                {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                IsDict({
                    {"<error>", {}, IsNull()}
                })
            );

            reg("DictEmptyGarbage", "{ * }",
                {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                IsDict({
                    {"<error>", {}, IsNull()}
                })
            );

            reg("MissingBothKeyAndValue", "{ :, :, z: 3}",
                {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4},
                    {E::InvalidExpression, 1, 4, 1, 5},
                    {E::ExpectedDictionaryKey, 1, 6, 1, 7},
                    {E::InvalidExpression, 1, 7, 1, 8},
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
                    {E::ExpectedDictionaryKey, 2, 1, 2, 2},
                    {E::InvalidExpression, 2, 2, 2, 3},
                    {E::ExpectedDictionaryKey, 3, 1, 3, 2},
                    {E::InvalidExpression, 3, 2, 3, 3},
                },
                IsDict({
                    {"<error>", {}, IsNull()},
                    {"<error>", {}, IsNull()},
                    {"z", {}, IsNumber("3")},
                })
            );

            reg("DoubleCommaBetweenPairs", "{ x: 1,, y: 2 }",
                {
                    {E::ExpectedDictionaryKey, 1, 8, 1, 9}
                },
                IsDict({
                    {"x", {}, IsNumber("1")},
                    {"<error>", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            );

            reg("DictKeyIsString", "{ \"x\": 1 }",
                {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 6}
                },
                IsDict({
                    {"<error>", {}, IsNumber("1")},
                })
            );

            reg("DictKeyIsNumber", "{ 1: 1 }",
                {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                IsDict({
                    {"<error>", {}, IsNumber("1")},
                })
            );

            reg("DictMissingColon", "{ x 1, y: 2 }",
                {
                    {E::ExpectedColonAfterDictionaryKey, 1, 5, 1, 6}
                },
                IsDict({
                    {"<error>", {}, IsNull()},
                    {"y", {}, IsNumber("2")},
                })
            );

            return true;
        }();
    }
}
