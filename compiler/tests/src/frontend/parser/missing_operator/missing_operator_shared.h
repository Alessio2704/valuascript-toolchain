#pragma once

#include <string>
#include <vector>
#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    enum class TemplateType { TwoLeaves, ThreeLeaves, FourLeaves, SpecialCases };
    enum class ExpansionPosition { Pos1, Pos2, Pos3 };

    struct MissingOperatorTemplateBase
    {
        std::string test_name;
        TemplateType type;
    };

    struct MissingOperatorExpansionDef
    {
        std::string test_name;
        TemplateType type;
        ExpansionPosition position;
    };

    inline std::vector<MissingOperatorExpansionDef> get_expansion_cases()
    {
        return {
            {.test_name = "ThreeLeaves_Pos1", .type = TemplateType::ThreeLeaves, .position = ExpansionPosition::Pos1},
            {.test_name = "ThreeLeaves_Pos2", .type = TemplateType::ThreeLeaves, .position = ExpansionPosition::Pos2},
            {.test_name = "FourLeaves_Pos1", .type = TemplateType::FourLeaves, .position = ExpansionPosition::Pos1},
            {.test_name = "FourLeaves_Pos2", .type = TemplateType::FourLeaves, .position = ExpansionPosition::Pos2},
            {.test_name = "FourLeaves_Pos3", .type = TemplateType::FourLeaves, .position = ExpansionPosition::Pos3},
        };
    }

    struct SpecialCaseDef
    {
        std::string test_name;
        std::string snippet;
        size_t start_col;
        size_t end_col;
        ExprVerifier verifier;
        std::vector<UniversalVerifier> multi;
    };

    struct AtomDef
    {
        std::string name;
        std::string code;
        ExprVerifier verifier;
        size_t first_token_len;
    };

    inline const std::vector<AtomDef>& get_atoms()
    {
        static std::vector<AtomDef> atoms = {
            {.name = "Number", .code = "1", .verifier = IsNumber("1"), .first_token_len = 1},
            {.name = "Identifier", .code = "a", .verifier = IsIdentifier("a"), .first_token_len = 1},
            {.name = "Percentage", .code = "11%", .verifier = IsPercentage("11%"), .first_token_len = 3},
            {.name = "Dot", .code = "a.b", .verifier = IsDot(IsIdentifier("a"), "b"), .first_token_len = 1},
            {.name = "Call", .code = "a()", .verifier = IsCall(IsIdentifier("a")), .first_token_len = 1},
            {.name = "Bracket", .code = "a[1]", .verifier = IsBracket(IsIdentifier("a"), IsNumber("1")), .first_token_len = 1},
            {.name = "Dict", .code = "{}", .verifier = IsDict(), .first_token_len = 1},
            {.name = "Switch", .code = "switch (x) {}", .verifier = IsSwitch(IsIdentifier("x"), std::vector<SwitchCaseSpec>{}), .first_token_len = 6}
        };
        return atoms;
    }

    struct TwoLeavesPairDef
    {
        AtomDef a;
        AtomDef b;
        std::string test_name;
    };

    inline std::vector<TwoLeavesPairDef> get_two_leaves_pairs()
    {
        std::vector<TwoLeavesPairDef> pairs;
        const auto& atoms = get_atoms();
        for (const auto& a : atoms)
        {
            for (const auto& b : atoms)
            {
                pairs.push_back({.a = a, .b = b, .test_name = a.name + "_" + b.name});
            }
        }
        return pairs;
    }

    inline const std::vector<SpecialCaseDef>& get_special_cases()
    {
        static std::vector<SpecialCaseDef> cases = {
            {
                .test_name = "BeforeGrouping",
                .snippet = "1 (2 + 3)",
                .start_col = 3,
                .end_col = 4,
                .verifier = IsCall(IsNumber("1")),
                .multi = {IsCall(IsNumber("1"))}
            },
            {
                .test_name = "InsideGrouping",
                .snippet = "1 + (2 3)",
                .start_col = 8,
                .end_col = 9,
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"),
                         IsGrouping(IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3")))),
                .multi = {
                    IsBinary(TokenType::Plus, IsNumber("1"),
                             IsGrouping(IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3"))))
                }
            },
            {
                .test_name = "CallAndGrouping",
                .snippet = "a + b (1 + 2)",
                .start_col = 7,
                .end_col = 8,
                .verifier = IsBinary(TokenType::Plus, IsIdentifier("a"), IsCall(IsIdentifier("b"))),
                .multi = {IsBinary(TokenType::Plus, IsIdentifier("a"), IsCall(IsIdentifier("b")))}
            },
            {
                .test_name = "BracketAndGrouping",
                .snippet = "a[1]  (b - c)",
                .start_col = 7,
                .end_col = 8,
                .verifier = IsCall(IsBracket(IsIdentifier("a"), IsNumber("1"))),
                .multi = {IsCall(IsBracket(IsIdentifier("a"), IsNumber("1")))}
            },
            {
                .test_name = "If_MissingOperator_InElse",
                .snippet = "if a then 1 else 0 1",
                .start_col = 20,
                .end_col = 21,
                .verifier = IsConditional(IsIdentifier("a"), IsNumber("1"), IsBinary(TokenType::Error, IsNumber("0"), IsNumber("1"))),
                .multi = {IsConditional(IsIdentifier("a"), IsNumber("1"), IsBinary(TokenType::Error, IsNumber("0"), IsNumber("1")))}
            },
            {
                .test_name = "If_MissingOperator_InThen",
                .snippet = "if a then 1 2 else 0",
                .start_col = 13,
                .end_col = 14,
                .verifier = IsConditional(IsIdentifier("a"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2")), IsNumber("0")),
                .multi = {
                    IsConditional(IsIdentifier("a"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2")),
                                  IsNumber("0"))
                }
            },
            {
                .test_name = "If_MissingOperator_Before",
                .snippet = "1 if a then 1 else 0",
                .start_col = 3,
                .end_col = 5,
                .verifier = IsBinary(TokenType::Error, IsNumber("1"), IsConditional(IsIdentifier("a"), IsNumber("1"), IsNumber("0"))),
                .multi = {IsNumber("1"), IsConditional(IsIdentifier("a"), IsNumber("1"), IsNumber("0"))}
            },
        };
        return cases;
    }
}
