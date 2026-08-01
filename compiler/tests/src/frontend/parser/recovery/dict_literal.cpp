#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ExprVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "DictMissingKey",
                .code = "{ : 1, y: 2 }",
                .errors = {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                .verifier = IsDict({
                    {"<error>", {}, IsNumber("1")},
                    {"y", {}, IsNumber("2")}
                })
            });

            reg({
                .name = "DictMissingComma",
                .code = "{ x: 1 y: 2 }",
                .errors = {
                    {E::ExpectedCommaSeparatorInDictionaryLiteral, 1, 8, 1, 9}
                },
                .verifier = IsDict({
                    {"x", {}, IsNumber("1")},
                    {"y", {}, IsNumber("2")}
                })
            });

            reg({
                .name = "DictMissingExpressionValue",
                .code = "{ x: , y: 2 }",
                .errors = {
                    {E::InvalidExpression, 1, 6, 1, 7}
                },
                .verifier = IsDict({
                    {"x", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            });

            reg({
                .name = "DictBrokenExpressionValues",
                .code = "{ x: *, y: *, z: 3 }",
                .errors = {
                    {E::InvalidExpression, 1, 6, 1, 7},
                    {E::InvalidExpression, 1, 12, 1, 13}
                },
                .verifier = IsDict({
                    {"x", {}, IsNull()},
                    {"y", {}, IsNull()},
                    {"z", {}, IsNumber("3")}
                })
            });

            reg({
                .name = "GarbageBetweenPairs",
                .code = "{ x: 1, +, y: 2 }",
                .errors = {
                    {E::ExpectedDictionaryKey, 1, 9, 1, 10}
                },
                .verifier = IsDict({
                    {"x", {}, IsNumber("1")},
                    {"<error>", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            });

            reg({
                .name = "DictEmptyComma",
                .code = "{ , }",
                .errors = {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                .verifier = IsDict({
                    {"<error>", {}, IsNull()}
                })
            });

            reg({
                .name = "DictEmptyGarbage",
                .code = "{ * }",
                .errors = {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                .verifier = IsDict({
                    {"<error>", {}, IsNull()}
                })
            });

            reg({
                .name = "MissingBothKeyAndValue",
                .code = "{ :, :, z: 3}",
                .errors = {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4},
                    {E::InvalidExpression, 1, 4, 1, 5},
                    {E::ExpectedDictionaryKey, 1, 6, 1, 7},
                    {E::InvalidExpression, 1, 7, 1, 8}
                },
                .verifier = IsDict({
                    {"<error>", {}, IsNull()},
                    {"<error>", {}, IsNull()},
                    {"z", {}, IsNumber("3")}
                })
            });

            reg({
                .name = "MissingBothKeyAndValueVertical",
                .code = "{\n"
                ":,\n"
                ":,\n"
                "z: 3\n}",
                .errors = {
                    {E::ExpectedDictionaryKey, 2, 1, 2, 2},
                    {E::InvalidExpression, 2, 2, 2, 3},
                    {E::ExpectedDictionaryKey, 3, 1, 3, 2},
                    {E::InvalidExpression, 3, 2, 3, 3}
                },
                .verifier = IsDict({
                    {"<error>", {}, IsNull()},
                    {"<error>", {}, IsNull()},
                    {"z", {}, IsNumber("3")}
                })
            });

            reg({
                .name = "DoubleCommaBetweenPairs",
                .code = "{ x: 1,, y: 2 }",
                .errors = {
                    {E::ExpectedDictionaryKey, 1, 8, 1, 9}
                },
                .verifier = IsDict({
                    {"x", {}, IsNumber("1")},
                    {"<error>", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            });

            reg({
                .name = "DictKeyIsString",
                .code = "{ \"x\": 1 }",
                .errors = {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 6}
                },
                .verifier = IsDict({
                    {"<error>", {}, IsNumber("1")}
                })
            });

            reg({
                .name = "DictKeyIsNumber",
                .code = "{ 1: 1 }",
                .errors = {
                    {E::ExpectedDictionaryKey, 1, 3, 1, 4}
                },
                .verifier = IsDict({
                    {"<error>", {}, IsNumber("1")}
                })
            });

            reg({
                .name = "DictMissingColon",
                .code = "{ x 1, y: 2 }",
                .errors = {
                    {E::ExpectedColonAfterDictionaryKey, 1, 5, 1, 6}
                },
                .verifier = IsDict({
                    {"<error>", {}, IsNull()},
                    {"y", {}, IsNumber("2")}
                })
            });

            return true;
        }();
    }
}
