#include <gtest/gtest.h>
#include "../helpers/parser_errors_synchronization_base.h"

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

        auto ExpectAssignmentType(std::function<void(const TypeAnnotation *)> verifier) {
            return [verifier = std::move(verifier)](const Program &ast) {
                ASSERT_FALSE(ast.execution_steps.empty()) << "Expected at least one execution step";
                auto assign = dynamic_cast<Assignment *>(ast.execution_steps[0].get());
                ASSERT_NE(assign, nullptr) << "Expected first step to be an Assignment";
                ASSERT_FALSE(assign->targets.empty()) << "Expected at least one assignment target";
                verifier(assign->targets[0].second.get());
            };
        }
    }

    class TypeAnnotationParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(TypeAnnotationParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAnnotationStressTests,
        TypeAnnotationParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "generic_missing_closing_bracket",
            "let a: vector<int = 1\n"
            "let b = 2\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 19} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "vector", 1);
                ExpectBaseType(type->generic_args[0].get(), "int", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "generic_missing_type_after_comma",
            "let a: map<string, > = 1\n",
            { {Err::TrailingCommaInGenericArgument, 1, 18} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 1);
                ExpectBaseType(type->generic_args[0].get(), "string", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "generic_garbage_argument_recovers",
            "let a: map<string, *, int> = 1\n",
            { {Err::MissingTypeAnnotation, 1, 20} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                ExpectBaseType(type->generic_args[0].get(), "string", 0);
                ExpectBaseType(type->generic_args[1].get(), "int", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_missing_closing_paren",
            "let a: (int, string = 1\n",
            { {Err::UnmatchedParenthesisInTuple, 1, 21} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectTupleType(type, 2);
                auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
                ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);
                ExpectBaseType(tuple_type->element_types[1].get(), "string", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_missing_element",
            "let a: (int, , string) = 1\n",
            { {Err::MissingTypeAnnotation, 1, 14} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectTupleType(type, 2);
                auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
                ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);
                ExpectBaseType(tuple_type->element_types[1].get(), "string", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_single_element_disallowed",
            "let a: (int,) = 1\n",
            { {Err::SingleElementTuplesNotAllowed, 1, 12} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectTupleType(type, 1);
                auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
                ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "generic_deeply_nested_empty",
            "let a: map<string, vector<>> = 1\n",
            { {Err::EmptyGenericTypeAnnotation, 1, 27} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                ExpectBaseType(type->generic_args[0].get(), "string", 0);
                ExpectBaseType(type->generic_args[1].get(), "vector", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "generic_unclosed_nested_error",
            "let a: map<string, vector<int> = 1\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 32} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                ExpectBaseType(type->generic_args[0].get(), "string", 0);

                auto inner_generic = type->generic_args[1].get();
                ExpectBaseType(inner_generic, "vector", 1);
                ExpectBaseType(inner_generic->generic_args[0].get(), "int", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_nested_in_generic",
            "let a: map<string, (int, )> = 1\n",
            { {Err::SingleElementTuplesNotAllowed, 1, 24} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                ExpectBaseType(type->generic_args[0].get(), "string", 0);

                auto inner_tuple = type->generic_args[1].get();
                ExpectTupleType(inner_tuple, 1);
                ExpectBaseType(dynamic_cast<const TupleTypeAnnotation*>(inner_tuple)->element_types[0].get(), "int", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_inside_tuple_type_broken",
            "let a: (int, (string, *, float)) = 1\n",
            { {Err::MissingTypeAnnotation, 1, 23} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectTupleType(type, 2);
                auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
                ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);

                auto inner_tuple = tuple_type->element_types[1].get();
                ExpectTupleType(inner_tuple, 2);
                auto inner_tuple_cast = dynamic_cast<const TupleTypeAnnotation*>(inner_tuple);
                ExpectBaseType(inner_tuple_cast->element_types[0].get(), "string", 0);
                ExpectBaseType(inner_tuple_cast->element_types[1].get(), "float", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "generic_missing_comma_between_args",
            "let a: map<string int> = 1\n",
            { {Err::ExpectedCommaSeparatorInGenericArgs, 1, 19} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                ExpectBaseType(type->generic_args[0].get(), "string", 0);
                ExpectBaseType(type->generic_args[1].get(), "int", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "generic_multiple_missing_commas",
            "let a: tuple<int string float> = 1\n",
            {
            {Err::ExpectedCommaSeparatorInGenericArgs, 1, 18},
            {Err::ExpectedCommaSeparatorInGenericArgs, 1, 25}
            },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "tuple", 3);
                ExpectBaseType(type->generic_args[0].get(), "int", 0);
                ExpectBaseType(type->generic_args[1].get(), "string", 0);
                ExpectBaseType(type->generic_args[2].get(), "float", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_missing_comma_between_args",
            "let a: (int string) = 1\n",
            { {Err::ExpectedCommaSeparatorInTupleType, 1, 13} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectTupleType(type, 2);
                auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
                ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);
                ExpectBaseType(tuple_type->element_types[1].get(), "string", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "generic_reserved_keyword_as_type",
            "let a: map<int, let> = 1\n",
            { {Err::ReservedKeywordAsIdentifier, 1, 17} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                ExpectBaseType(type->generic_args[0].get(), "int", 0);
                ExpectBaseType(type->generic_args[1].get(), "let", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "generic_multiple_garbage_elements",
            "let a: map<*, int, *, float> = 1\n",
            {
            {Err::MissingTypeAnnotation, 1, 12},
            {Err::MissingTypeAnnotation, 1, 20}
            },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                ExpectBaseType(type->generic_args[0].get(), "int", 0);
                ExpectBaseType(type->generic_args[1].get(), "float", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "generic_empty_slots_with_commas",
            "let a: map<int, , string> = 1\n",
            { {Err::MissingTypeAnnotation, 1, 17} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                ExpectBaseType(type->generic_args[0].get(), "int", 0);
                ExpectBaseType(type->generic_args[1].get(), "string", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_multiple_garbage_elements",
            "let a: (int, *, float, ^) = 1\n",
            {
            {Err::MissingTypeAnnotation, 1, 14},
            {Err::MissingTypeAnnotation, 1, 24}
            },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectTupleType(type, 2);
                auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
                ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);
                ExpectBaseType(tuple_type->element_types[1].get(), "float", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_trailing_comma",
            "let a: (int, string, ) = 1\n",
            { {Err::SingleElementTuplesNotAllowed, 1, 20} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectTupleType(type, 2);
                auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
                ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);
                ExpectBaseType(tuple_type->element_types[1].get(), "string", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "deeply_nested_mixed_garbage",
            "let a: map<string, (int, *, vector<float, >)> = 1\n",
            {
            {Err::MissingTypeAnnotation, 1, 26},
            {Err::TrailingCommaInGenericArgument, 1, 41}
            },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                ExpectBaseType(type->generic_args[0].get(), "string", 0);

                auto inner_tuple = type->generic_args[1].get();
                ExpectTupleType(inner_tuple, 2);
                auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(inner_tuple);
                ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);

                auto inner_vec = tuple_type->element_types[1].get();
                ExpectBaseType(inner_vec, "vector", 1);
                ExpectBaseType(inner_vec->generic_args[0].get(), "float", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "eof_inside_generic",
            "let a: vector<int, ",
            {
            {Err::UnmatchedBracketAfterGenericArgs, 1, 19},
            {Err::IncompleteAssignment, 1, 19}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            ASSERT_EQ(assign->targets.size(), 1);

            auto type = assign->targets[0].second.get();
            ExpectBaseType(type, "vector", 1);
            ExpectBaseType(type->generic_args[0].get(), "int", 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "eof_inside_tuple_type",
            "let a: (int, ",
            {
            {Err::UnmatchedParenthesisInTuple, 1, 13},
            {Err::IncompleteAssignment, 1, 13}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            ASSERT_EQ(assign->targets.size(), 1);

            auto type = assign->targets[0].second.get();
            ExpectTupleType(type, 1);
            auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
            ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "generic_run_on_into_next_statement",
            "let a: vector<int\n"
            "let b = 2\n",
            {
            {Err::UnmatchedBracketAfterGenericArgs, 1, 18},
            {Err::IncompleteAssignment, 1, 18}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_a, nullptr);
            ExpectBaseType(assign_a->targets[0].second.get(), "vector", 1);

            auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_run_on_into_next_statement",
            "let a: (int\n"
            "let b = 2\n",
            {
            {Err::UnmatchedParenthesisInTuple, 1, 12},
            {Err::IncompleteAssignment, 1, 12}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_a, nullptr);
            ExpectTupleType(assign_a->targets[0].second.get(), 1);

            auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "generic_closed_with_wrong_bracket",
            "let a: vector<int} = 1\n",
            {
            {Err::UnmatchedBracketAfterGenericArgs, 1, 18}
            },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "vector", 1);
                ExpectBaseType(type->generic_args[0].get(), "int", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_type_closed_with_wrong_bracket",
            "let a: (int, string] = 1\n",
            {
            {Err::UnmatchedParenthesisInTuple, 1, 20}
            },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectTupleType(type, 2);
                auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
                ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);
                ExpectBaseType(tuple_type->element_types[1].get(), "string", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "missing_base_type_before_generic_discards_assignment",
            "let a: <int> = 1\n"
            "let b = 2\n",
            {
            {Err::MissingTypeAnnotation, 1, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "a");
            EXPECT_EQ(assign->targets[0].second.get(), nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_broken_type_recovers_next_field",
            "struct S {\n"
            "  a: vector<int,\n"
            "  b: string\n"
            "}\n"
            "let a = 1\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 2, 16} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 2);

            EXPECT_EQ(s->fields[0].name, "a");
            ExpectBaseType(s->fields[0].type.get(), "vector", 1);

            EXPECT_EQ(s->fields[1].name, "b");
            ExpectBaseType(s->fields[1].type.get(), "string", 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "enum_underlying_type_broken_recovers_body",
            "enum E: vector<int {\n"
            "  A = 1\n"
            "}\n"
            "let a = 1\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 20} },
            [](const Program& ast) {
            ASSERT_EQ(ast.enum_definitions.size(), 1);
            auto e = ast.enum_definitions[0].get();
            ExpectBaseType(e->underlying_type.get(), "vector", 1);
            ASSERT_EQ(e->cases.size(), 1);
            EXPECT_EQ(e->cases[0].name, "A");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_return_types_one_broken_recovers_others",
            "func f() -> int, vector<float, string {\n"
            "  return 1\n"
            "}\n"
            "let a = 1\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 39} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto f = ast.function_definitions[0].get();
            ASSERT_EQ(f->return_types.size(), 2);
            ExpectBaseType(f->return_types[0].get(), "int", 0);
            ExpectBaseType(f->return_types[1].get(), "vector", 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_return_types_trailing_comma_error",
            "func f() -> int, string, {\n"
            "  return 1\n"
            "}\n"
            "let a = 1\n",
            { {Err::TrailingComma, 1, 24} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->return_types.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "generic_nested_in_tuple_missing_comma_recovery",
            "let a: (int, vector<string int>) = 1\n",
            { {Err::ExpectedCommaSeparatorInGenericArgs, 1, 28} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                auto tuple = dynamic_cast<const TupleTypeAnnotation*>(type);
                ASSERT_NE(tuple, nullptr);
                auto vec = tuple->element_types[1].get();
                ExpectBaseType(vec, "vector", 2);
                ExpectBaseType(vec->generic_args[0].get(), "string", 0);
                ExpectBaseType(vec->generic_args[1].get(), "int", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "type_annotation_with_broken_tuple_as_generic_arg",
            "let a: map<string, (int, *)> = 1\n",
            { {Err::MissingTypeAnnotation, 1, 26} },
            ExpectAssignmentType([](const TypeAnnotation* type) {
                ExpectBaseType(type, "map", 2);
                auto inner_tuple = dynamic_cast<const TupleTypeAnnotation*>(type->generic_args[1].get());
                ASSERT_NE(inner_tuple, nullptr);
                EXPECT_EQ(inner_tuple->element_types.size(), 1);
                EXPECT_EQ(inner_tuple->element_types[0]->name, "int");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_broken_generic_type_recovers_next_param",
            "func test(a: vector<int,\n"
            "          b: string) -> void {\n"
            "}\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 24} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto f = ast.function_definitions[0].get();
            ASSERT_EQ(f->parameters.size(), 2);

            EXPECT_EQ(f->parameters[0].name, "a");
            ExpectBaseType(f->parameters[0].type.get(), "vector", 1);

            EXPECT_EQ(f->parameters[1].name, "b");
            ExpectBaseType(f->parameters[1].type.get(), "string", 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_broken_tuple_type_recovers_next_field",
            "struct S {\n"
            "  a: (int, string,\n"
            "  b: bool\n"
            "}\n",
            { {Err::UnmatchedParenthesisInTuple, 2, 18} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 2);

            EXPECT_EQ(s->fields[0].name, "a");

            auto t_ann = dynamic_cast<TupleTypeAnnotation*>(s->fields[0].type.get());
            ASSERT_NE(t_ann, nullptr);
            EXPECT_EQ(t_ann->element_types.size(), 2);

            EXPECT_EQ(s->fields[1].name, "b");
            ExpectBaseType(s->fields[1].type.get(), "bool", 0);
            }
            },ParserErrorsSynchronizationTestCase{
            "struct_field_deeply_broken_generic_recovers_next_field",
            "struct S {\n"
            "  a: dict<int, vector<string,\n"
            "  b: float\n"
            "}\n",
            {
            {Err::UnmatchedBracketAfterGenericArgs, 2, 29},
            {Err::UnmatchedBracketAfterGenericArgs, 2, 29}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 2);

            EXPECT_EQ(s->fields[0].name, "a");
            auto d_ann = dynamic_cast<TypeAnnotation*>(s->fields[0].type.get());
            ASSERT_NE(d_ann, nullptr);
            EXPECT_EQ(d_ann->name, "dict");
            ASSERT_EQ(d_ann->generic_args.size(), 2);

            EXPECT_EQ(s->fields[1].name, "b");
            ExpectBaseType(s->fields[1].type.get(), "float", 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_missing_bracket_and_comma_recovers",
            "func test(a: vector<int b: string) -> void {}\n",
            {
            {Err::UnmatchedBracketAfterGenericArgs, 1, 25},
            {Err::ExpectedCommaSeparatorInParameterList, 1, 25}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto f = ast.function_definitions[0].get();
            ASSERT_EQ(f->parameters.size(), 2);

            EXPECT_EQ(f->parameters[0].name, "a");
            EXPECT_EQ(f->parameters[1].name, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_return_type_broken_generic_recovers_body",
            "func test() -> vector<int {\n"
            "  let a = 1\n"
            "}\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 27} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto f = ast.function_definitions[0].get();
            EXPECT_EQ(f->name, "test");

            ASSERT_EQ(f->return_types.size(), 1);
            ExpectBaseType(f->return_types[0].get(), "vector", 1);

            ASSERT_EQ(f->body.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "enum_underlying_type_broken_generic_recovers_cases",
            "enum E: vector<int { A }",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 20} },
            [](const Program& ast) {
            ASSERT_EQ(ast.enum_definitions.size(), 1);
            auto e = ast.enum_definitions[0].get();
            EXPECT_EQ(e->name, "E");

            ASSERT_NE(e->underlying_type, nullptr);
            ExpectBaseType(e->underlying_type.get(), "vector", 1);


            ASSERT_EQ(e->cases.size(), 1);
            EXPECT_EQ(e->cases[0].name, "A");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "type_annotation_missing_comma_in_generics",
            "struct S {\n"
            "  a: dict<int string>\n"
            "}\n",
            { {Err::ExpectedCommaSeparatorInGenericArgs, 2, 15} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 1);

            auto t = s->fields[0].type.get();
            EXPECT_EQ(t->name, "dict");
            ASSERT_EQ(t->generic_args.size(), 2);

            ExpectBaseType(t->generic_args[0].get(), "int", 0);
            ExpectBaseType(t->generic_args[1].get(), "string", 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "type_annotation_invalid_token_recovers",
            "struct S {\n"
            "  a: vector<123>,\n"
            "  b: int\n"
            "}\n",
            {
            {Err::MissingTypeAnnotation, 2, 13},
            {Err::EmptyGenericTypeAnnotation, 2, 16},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 2);

            EXPECT_EQ(s->fields[0].name, "a");
            EXPECT_EQ(s->fields[1].name, "b");
            ExpectBaseType(s->fields[1].type.get(), "int", 0);
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
