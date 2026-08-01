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
                    {E::MissingCommaSeparatorForArgumentsInFunctionCall, 1, 8, 1, 9}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"b", IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "TrailingComma",
                .code = "f(a: 1,)",
                .errors = {
                    {E::TrailingCommaInFunctionCall, 1, 7, 1, 8}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")}
                    }
                )
            });

            reg({
                .name = "MissingArgName",
                .code = "f(: 1, b: 2)",
                .errors = {
                    {E::MissingArgumentNameInFunctionCall, 1, 3, 1, 4}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"<error>", IsNumber("1")},
                        {"b", IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "MissingColon1",
                .code = "f(a 1, b: 2)",
                .errors = {
                    {E::MissingColonAfterArgument, 1, 5, 1, 6}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"<error>", IsNull()},
                        {"b", IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "MissingColon2",
                .code = "f(a, b, c)",
                .errors = {
                    {E::MissingColonAfterArgument, 1, 4, 1, 5},
                    {E::MissingColonAfterArgument, 1, 7, 1, 8},
                    {E::MissingColonAfterArgument, 1, 10, 1, 11}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNull()},
                        {"b", IsNull()},
                        {"c", IsNull()}
                    }
                )
            });

            reg({
                .name = "MissingColon3",
                .code = "f(a: 1, b, c)",
                .errors = {
                    {E::MissingColonAfterArgument, 1, 10, 1, 11},
                    {E::MissingColonAfterArgument, 1, 13, 1, 14}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"b", IsNull()},
                        {"c", IsNull()}
                    }
                )
            });

            reg({
                .name = "MissingArgValue",
                .code = "f(a: , b: 2)",
                .errors = {
                    {E::InvalidExpression, 1, 6, 1, 7}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNull()},
                        {"b", IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "GarbageTokensRecoversGracefully",
                .code = "f(a: 1, +-*/, b: 2)",
                .errors = {
                    {E::MissingArgumentNameInFunctionCall, 1, 9, 1, 10}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"<error>", IsNull()},
                        {"b", IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "MissingCommaAndColonBreaksList",
                .code = "f(a: 1 b 2)",
                .errors = {
                    {E::MissingOperator, 1, 8, 1, 9},
                    {E::MissingOperator, 1, 10, 1, 11}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {
                            "a", IsBinary(
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
                    {E::InvalidExpression, 1, 12, 1, 13}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"b", IsNull()}
                    }
                )
            });

            reg({
                .name = "MultipleConsecutiveCommas",
                .code = "f(a: 1,, b: 2)",
                .errors = {
                    {E::MissingArgumentNameInFunctionCall, 1, 8, 1, 9}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"<error>", IsNull()},
                        {"b", IsNumber("2")}
                    }
                )
            });

            reg({
                .name = "InvalidTokenAsArgName",
                .code = "f(a: 1, *: 2, b: 3)",
                .errors = {
                    {E::MissingArgumentNameInFunctionCall, 1, 9, 1, 10}
                },
                .verifier = IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"<error>", IsNumber("2")},
                        {"b", IsNumber("3")}
                    }
                )
            });

            return true;
        }();
    }
}
