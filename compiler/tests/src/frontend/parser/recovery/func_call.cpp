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
                .name = "MissingCommaRecoversBothArgs",
                .code = "f(a: 1 b: 2)",
                .errors = {
                    PErr{.code = E::MissingCommaSeparatorForArgumentsInFunctionCall, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="a", .value_v=IsNumber("1")},
                        {.label="b", .value_v=IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "TrailingComma",
                .code = "f(a: 1,)",
                .errors = {
                    PErr{.code = E::TrailingCommaInFunctionCall, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="a", .value_v=IsNumber("1")}
                    }
                )
            });

            reg({
                .name = "MissingArgName",
                .code = "f(: 1, b: 2)",
                .errors = {
                    PErr{.code = E::MissingArgumentNameInFunctionCall, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="<error>", .value_v=IsNumber("1")},
                        {.label="b", .value_v=IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "MissingColon1",
                .code = "f(a 1, b: 2)",
                .errors = {
                    PErr{.code = E::MissingColonAfterArgument, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="<error>", .value_v=IsNull()},
                        {.label="b", .value_v=IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "MissingColon2",
                .code = "f(a, b, c)",
                .errors = {
                    PErr{.code = E::MissingColonAfterArgument, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 5},
                    PErr{.code = E::MissingColonAfterArgument, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8},
                    PErr{.code = E::MissingColonAfterArgument, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="a", .value_v=IsNull()},
                        {.label="b", .value_v=IsNull()},
                        {.label="c", .value_v=IsNull()}
                    }
                )
            });

            reg({
                .name = "MissingColon3",
                .code = "f(a: 1, b, c)",
                .errors = {
                    PErr{.code = E::MissingColonAfterArgument, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11},
                    PErr{.code = E::MissingColonAfterArgument, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="a", .value_v=IsNumber("1")},
                        {.label="b", .value_v=IsNull()},
                        {.label="c", .value_v=IsNull()}
                    }
                )
            });

            reg({
                .name = "MissingArgValue",
                .code = "f(a: , b: 2)",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="a", .value_v=IsNull()},
                        {.label="b", .value_v=IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "GarbageTokensRecoversGracefully",
                .code = "f(a: 1, +-*/, b: 2)",
                .errors = {
                    PErr{.code = E::MissingArgumentNameInFunctionCall, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="a", .value_v=IsNumber("1")},
                        {.label="<error>", .value_v=IsNull()},
                        {.label="b", .value_v=IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "MissingCommaAndColonBreaksList",
                .code = "f(a: 1 b 2)",
                .errors = {
                    PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                    PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {
                            .label="a", .value_v=IsBinary(
                                TokenType::Error, IsNumber("1"),
                                IsBinary(TokenType::Error, IsIdentifier("b"), IsNumber("2"))
                            )
                        }
                    }
                )
            });

            reg({
                .name = "MissingValueAtEnd",
                .code = "f(a: 1, b: )",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="a", .value_v=IsNumber("1")},
                        {.label="b", .value_v=IsNull()}
                    }
                )
            });

            reg({
                .name = "MultipleConsecutiveCommas",
                .code = "f(a: 1,, b: 2)",
                .errors = {
                    PErr{.code = E::MissingArgumentNameInFunctionCall, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="a", .value_v=IsNumber("1")},
                        {.label="<error>", .value_v=IsNull()},
                        {.label="b", .value_v=IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "InvalidTokenAsArgName",
                .code = "f(a: 1, *: 2, b: 3)",
                .errors = {
                    PErr{.code = E::MissingArgumentNameInFunctionCall, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {.label="a", .value_v=IsNumber("1")},
                        {.label="<error>", .value_v=IsNumber("2")},
                        {.label="b", .value_v=IsNumber("3")}
                    }
                )
            });

            return true;
        }();
    }
}
