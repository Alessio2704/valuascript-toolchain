#pragma once

#include <string>
#include <vector>
#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    enum class TemplateType { TwoLeaves, ThreeLeaves, FourLeaves, SpecialCases };

    struct MissingOperatorTemplateBase
    {
        std::string test_name;
        TemplateType type;
    };

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
            {"Number", "1", IsNumber("1"), 1},
            {"Identifier", "a", IsIdentifier("a"), 1},
            {"Percentage", "11%", IsPercentage("11%"), 3},
            {"Dot", "a.b", IsDot(IsIdentifier("a"), "b"), 1},
            {"Call", "a()", IsCall(IsIdentifier("a")), 1},
            {"Bracket", "a[1]", IsBracket(IsIdentifier("a"), IsNumber("1")), 1},
            {"Dict", "{}", IsDict({}), 1},
            {"Switch", "switch (x) {}", IsSwitch(IsIdentifier("x"), {}), 6}
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
                pairs.push_back({a, b, a.name + "_" + b.name});
            }
        }
        return pairs;
    }

    inline const std::vector<SpecialCaseDef>& get_special_cases()
    {
        static std::vector<SpecialCaseDef> cases = {
            {
                "BeforeGrouping",
                "1 (2 + 3)",
                3, 4,
                IsCall(IsNumber("1")),
                {IsCall(IsNumber("1"))}
            },
            {
                "InsideGrouping",
                "1 + (2 3)",
                8, 9,
                IsBinary(TokenType::Plus, IsNumber("1"),
                         IsGrouping(IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3")))),
                {
                    IsBinary(TokenType::Plus, IsNumber("1"),
                             IsGrouping(IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3"))))
                }
            },
            {
                "CallAndGrouping",
                "a + b (1 + 2)",
                7, 8,
                IsBinary(TokenType::Plus, IsIdentifier("a"), IsCall(IsIdentifier("b"))),
                {IsBinary(TokenType::Plus, IsIdentifier("a"), IsCall(IsIdentifier("b")))}
            },
            {
                "BracketAndGrouping",
                "a[1]  (b - c)",
                7, 8,
                IsCall(IsBracket(IsIdentifier("a"), IsNumber("1"))),
                {IsCall(IsBracket(IsIdentifier("a"), IsNumber("1")))}
            },
            {
                "If_MissingOperator_InElse",
                "if a then 1 else 0 1",
                20, 21,
                IsConditional(IsIdentifier("a"), IsNumber("1"), IsBinary(TokenType::Error, IsNumber("0"), IsNumber("1"))),
                {IsConditional(IsIdentifier("a"), IsNumber("1"), IsBinary(TokenType::Error, IsNumber("0"), IsNumber("1")))}
            },
            {
                "If_MissingOperator_InThen",
                "if a then 1 2 else 0",
                13, 14,
                IsConditional(IsIdentifier("a"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2")), IsNumber("0")),
                {
                    IsConditional(IsIdentifier("a"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2")),
                                  IsNumber("0"))
                }
            },
            {
                "If_MissingOperator_Before",
                "1 if a then 1 else 0",
                3, 5,
                IsBinary(TokenType::Error, IsNumber("1"), IsConditional(IsIdentifier("a"), IsNumber("1"), IsNumber("0"))),
                {IsNumber("1"), IsConditional(IsIdentifier("a"), IsNumber("1"), IsNumber("0"))}
            },
        };
        return cases;
    }
}
