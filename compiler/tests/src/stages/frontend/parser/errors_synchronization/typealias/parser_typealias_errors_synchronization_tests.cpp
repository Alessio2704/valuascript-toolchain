#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_errors_synchronization_base.h"
#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler::test {
    namespace {
        void ExpectBaseType(const TypeAnnotation *type, const std::string &expected_name,
                            size_t expected_generics = 0) {
            ASSERT_NE(type, nullptr) << "Expected a TypeAnnotation, but got nullptr";
            EXPECT_EQ(type->name, expected_name);
            EXPECT_EQ(type->generic_args.size(), expected_generics);
        }

        void ExpectTupleType(const TypeAnnotation *type, size_t expected_elements) {
            ASSERT_NE(type, nullptr) << "Expected a TypeAnnotation, but got nullptr";
            EXPECT_EQ(type->name, "tuple");
            auto tuple_type = dynamic_cast<const TupleTypeAnnotation *>(type);
            ASSERT_NE(tuple_type, nullptr) << "TypeAnnotation is not a TupleTypeAnnotation";
            EXPECT_EQ(tuple_type->element_types.size(), expected_elements);
        }

        auto ExpectRecoveredAssignment(const std::string &expected_target_name = "a") {
            return [expected_target_name](const Program &ast) {
                ASSERT_FALSE(
                    ast.execution_steps.empty(
                    )) << "Expected at least one execution step indicating successful recovery";
                auto assign = dynamic_cast<Assignment *>(ast.execution_steps[0].get());
                ASSERT_NE(assign, nullptr) << "Expected first step to be an Assignment";
                ASSERT_FALSE(assign->targets.empty()) << "Expected at least one assignment target";
                EXPECT_EQ(assign->targets[0].first, expected_target_name);
            };
        }
    }

    class TypeAliasParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(TypeAliasParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAliasStressTests,
        TypeAliasParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "missing_alias_name",
            "typealias = int\n"
            "let a = 1\n",
            { {Err::ExpectedTypeAliasName, 1, 11} },
            [](const Program& ast) {
            EXPECT_TRUE(ast.type_aliases.empty());
            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_assignment_operator",
            "typealias MyType int\n"
            "let a = 1\n",
            { {Err::ExpectedAssignAfterTypeAliasName, 1, 18} },
            [](const Program& ast) {
            EXPECT_TRUE(ast.type_aliases.empty());
            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_target_type_annotation",
            "typealias MyType =\n"
            "let a = 1\n",
            { {Err::MissingTypeAnnotation, 1, 19} },
            [](const Program& ast) {
            EXPECT_TRUE(ast.type_aliases.empty());
            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_target_type_annotation",
            "typealias MyType = *\n"
            "let a = 1\n",
            { {Err::MissingTypeAnnotation, 1, 20} },
            [](const Program& ast) {
            EXPECT_TRUE(ast.type_aliases.empty());
            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "run_on_generic_into_next_statement",
            "typealias MyType = vector<int\n"
            "let a = 1\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 30} },
            [](const Program& ast) {
            ASSERT_EQ(ast.type_aliases.size(), 1);
            EXPECT_EQ(ast.type_aliases[0]->name, "MyType");
            ExpectBaseType(ast.type_aliases[0]->target_type.get(), "vector", 1);
            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "run_on_tuple_into_next_statement",
            "typealias MyType = (int, string\n"
            "let a = 1\n",
            { {Err::UnmatchedParenthesisInTuple, 1, 32} },
            [](const Program& ast) {
            ASSERT_EQ(ast.type_aliases.size(), 1);
            EXPECT_EQ(ast.type_aliases[0]->name, "MyType");
            ExpectTupleType(ast.type_aliases[0]->target_type.get(), 2);
            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "type_alias_reserved_keyword_as_name",
            "typealias func = int\n"
            "let a = 1\n",
            { {Err::ReservedKeywordAsIdentifier, 1, 11} },
            [](const Program& ast) {
            ASSERT_EQ(ast.type_aliases.size(), 1);
            EXPECT_EQ(ast.type_aliases[0]->name, "func");
            ExpectBaseType(ast.type_aliases[0]->target_type.get(), "int", 0);
            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "deeply_nested_generic_missing_commas_and_brackets",
            "typealias MyGraph = map<string vector<int>\n"
            "typealias Valid = bool\n",
            {
            {Err::ExpectedCommaSeparatorInGenericArgs, 1, 32},
            {Err::UnmatchedBracketAfterGenericArgs, 1, 43}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.type_aliases.size(), 2);

            EXPECT_EQ(ast.type_aliases[0]->name, "MyGraph");
            ExpectBaseType(ast.type_aliases[0]->target_type.get(), "map", 2);

            EXPECT_EQ(ast.type_aliases[1]->name, "Valid");
            ExpectBaseType(ast.type_aliases[1]->target_type.get(), "bool", 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_at_end_of_otherwise_valid_alias",
            "typealias User = string ^^\n"
            "let a = 1\n",
            { {Err::InvalidExpression, 1, 25} },
            [](const Program& ast) {
            ASSERT_EQ(ast.type_aliases.size(), 1);
            EXPECT_EQ(ast.type_aliases[0]->name, "User");
            ExpectBaseType(ast.type_aliases[0]->target_type.get(), "string", 0);

            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "type_alias_interleaved_with_struct_error_recovery",
            "struct Broken { a: vector<int }\n"
            "typealias Safe = string\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 31} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            EXPECT_EQ(ast.struct_definitions[0]->name, "Broken");

            ASSERT_EQ(ast.type_aliases.size(), 1);
            EXPECT_EQ(ast.type_aliases[0]->name, "Safe");
            ExpectBaseType(ast.type_aliases[0]->target_type.get(), "string", 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "type_alias_reserved_keyword_as_target_1",
            "typealias MyType = struct\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 20},
            },
            [](const Program& ast) {
            EXPECT_TRUE(ast.type_aliases.empty());
            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "type_alias_reserved_keyword_as_target_2",
            "typealias MyType = true\n"
            "let a = 1\n",
            { {Err::ReservedKeywordAsIdentifier, 1, 20} },
            [](const Program& ast) {
            EXPECT_TRUE(ast.type_aliases.empty());
            ExpectRecoveredAssignment("a")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_broken_type_aliases_in_sequence_1",
            "typealias A = vector<int\n"
            "typealias = string\n"
            "typealias C = map<int, >\n"
            "let d = 1\n",
            {
            {Err::UnmatchedBracketAfterGenericArgs, 1, 25},
            {Err::ExpectedTypeAliasName, 2, 11},
            {Err::TrailingCommaInGenericArgument, 3, 22}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.type_aliases.size(), 2);
            EXPECT_EQ(ast.type_aliases[0]->name, "A");
            ExpectBaseType(ast.type_aliases[0]->target_type.get(), "vector", 1);

            EXPECT_EQ(ast.type_aliases[1]->name, "C");
            ExpectBaseType(ast.type_aliases[1]->target_type.get(), "map", 1);

            ExpectRecoveredAssignment("d")(ast);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_broken_type_aliases_in_sequence_2",
            "typealias A = vector<int\n"
            "typealias B string\n"
            "typealias C = map<int, >\n"
            "let d = 1\n",
            {
            {Err::UnmatchedBracketAfterGenericArgs, 1, 25},
            {Err::ExpectedAssignAfterTypeAliasName, 2, 13},
            {Err::TrailingCommaInGenericArgument, 3, 22}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.type_aliases.size(), 2);

            EXPECT_EQ(ast.type_aliases[0]->name, "A");
            ExpectBaseType(ast.type_aliases[0]->target_type.get(), "vector", 1);

            EXPECT_EQ(ast.type_aliases[1]->name, "C");
            ExpectBaseType(ast.type_aliases[1]->target_type.get(), "map", 1);

            ExpectRecoveredAssignment("d")(ast);
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
