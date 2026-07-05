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

            reg("MissingCommaRecoversBothArgs", "f(a: 1 b: 2)",
                {
                    {E::MissingCommaSeparatorForArgumentsInFunctionCall, 1, 8, 1, 9}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"b", IsNumber("2")}
                    }
                )
            );

            reg("TrailingComma", "f(a: 1,)",
                {
                    {E::TrailingCommaInFunctionCall, 1, 7, 1, 8}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")}
                    }
                )
            );

            reg("MissingArgName", "f(: 1, b: 2)",
                {
                    {E::MissingArgumentNameInFunctionCall, 1, 3, 1, 4}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"<error>", IsNumber("1")},
                        {"b", IsNumber("2")}
                    }
                )
            );

            reg("MissingColon1", "f(a 1, b: 2)",
                {
                    {E::MissingColonAfterArgument, 1, 5, 1, 6}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"<error>", IsNull()},
                        {"b", IsNumber("2")}
                    }
                )
            );

            reg("MissingColon2", "f(a, b, c)",
                {
                    {E::MissingColonAfterArgument, 1, 4, 1, 5},
                    {E::MissingColonAfterArgument, 1, 7, 1, 8},
                    {E::MissingColonAfterArgument, 1, 10, 1, 11}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNull()},
                        {"b", IsNull()},
                        {"c", IsNull()}
                    }
                )
            );

            reg("MissingColon3", "f(a: 1, b, c)",
                {
                    {E::MissingColonAfterArgument, 1, 10, 1, 11},
                    {E::MissingColonAfterArgument, 1, 13, 1, 14}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"b", IsNull()},
                        {"c", IsNull()}
                    }
                )
            );

            reg("MissingArgValue", "f(a: , b: 2)",
                {
                    {E::InvalidExpression, 1, 6, 1, 7}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNull()},
                        {"b", IsNumber("2")}
                    }
                )
            );

            reg("GarbageTokensRecoversGracefully", "f(a: 1, +-*/, b: 2)",
                {
                    {E::MissingArgumentNameInFunctionCall, 1, 9, 1, 10}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"<error>", IsNull()},
                        {"b", IsNumber("2")}
                    }
                )
            );

            reg("MissingCommaAndColonBreaksList", "f(a: 1 b 2)",
                {
                    {E::MissingOperator, 1, 8, 1, 9},
                    {E::MissingOperator, 1, 10, 1, 11},
                },
                IsCall(
                    IsIdentifier("f"), {
                        {
                            "a", IsBinary(
                                TokenType::Error, IsNumber("1"),
                                IsBinary(TokenType::Error, IsIdentifier("b"), IsNumber("2"))
                            )
                        }
                    }
                )
            );

            reg("MissingValueAtEnd", "f(a: 1, b: )",
                {
                    {E::InvalidExpression, 1, 12, 1, 13}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"b", IsNull()}
                    }
                )
            );

            reg("MultipleConsecutiveCommas", "f(a: 1,, b: 2)",
                {
                    {E::MissingArgumentNameInFunctionCall, 1, 8, 1, 9}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"<error>", IsNull()},
                        {"b", IsNumber("2")}
                    }
                )
            );

            reg("InvalidTokenAsArgName", "f(a: 1, *: 2, b: 3)",
                {
                    {E::MissingArgumentNameInFunctionCall, 1, 9, 1, 10}
                },
                IsCall(
                    IsIdentifier("f"), {
                        {"a", IsNumber("1")},
                        {"<error>", IsNumber("2")},
                        {"b", IsNumber("3")}
                    }
                )
            );

            return true;
        }();
    }
}
