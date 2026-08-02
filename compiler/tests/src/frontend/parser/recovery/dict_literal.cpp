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
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictMissingComma",
                .code = "{ x: 1 y: 2 }",
                .errors = {
                    PErr{.code = E::ExpectedCommaSeparatorInDictionaryLiteral, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictMissingExpressionValue",
                .code = "{ x: , y: 2 }",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictBrokenExpressionValues",
                .code = "{ x: *, y: *, z: 3 }",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNull()},
                    DictItemSpec{.key = "z", .value_v = IsNumber("3")}
                )
            });

            reg({
                .name = "GarbageBetweenPairs",
                .code = "{ x: 1, +, y: 2 }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictEmptyComma",
                .code = "{ , }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()}
                )
            });

            reg({
                .name = "DictEmptyGarbage",
                .code = "{ * }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()}
                )
            });

            reg({
                .name = "MissingBothKeyAndValue",
                .code = "{ :, :, z: 3}",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 5},
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "z", .value_v = IsNumber("3")}
                )
            });

            reg({
                .name = "MissingBothKeyAndValueVertical",
                .code = "{\n"
                ":,\n"
                ":,\n"
                "z: 3\n}",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 2, .column_start = 1, .line_end = 2, .column_end = 2},
                    PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 2, .line_end = 2, .column_end = 3},
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 2},
                    PErr{.code = E::InvalidExpression, .line_start = 3, .column_start = 2, .line_end = 3, .column_end = 3}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "z", .value_v = IsNumber("3")}
                )
            });

            reg({
                .name = "DoubleCommaBetweenPairs",
                .code = "{ x: 1,, y: 2 }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictKeyIsString",
                .code = "{ \"x\": 1 }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 6}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNumber("1")}
                )
            });

            reg({
                .name = "DictKeyIsNumber",
                .code = "{ 1: 1 }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNumber("1")}
                )
            });

            reg({
                .name = "DictMissingColon",
                .code = "{ x 1, y: 2 }",
                .errors = {
                    PErr{.code = E::ExpectedColonAfterDictionaryKey, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            return true;
        }();
    }
}
