#include "frontend/parser/helpers/ast_base_test.h"
#include "frontend/parser/ast.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesSimpleTypeAlias) {
        auto ast = parse_code("typealias Identifier = string");

        ASSERT_EQ(ast->type_aliases.size(), 1);
        auto alias_def = ast->type_aliases[0].get();

        EXPECT_EQ(alias_def->name, "Identifier");

        auto type_ann = alias_def->target_type.get();
        ASSERT_NE(type_ann, nullptr);
        EXPECT_EQ(type_ann->name, "string");
        EXPECT_TRUE(type_ann->generic_args.empty()) << "Expected no generic arguments for a simple primitive.";
        EXPECT_TRUE(alias_def->modifiers.empty());
    }

    TEST_F(AstBaseTest, ValidatesGenericTypeAlias) {
        auto ast = parse_code("typealias StringList = vector<string>");

        ASSERT_EQ(ast->type_aliases.size(), 1);
        auto alias_def = ast->type_aliases[0].get();

        EXPECT_EQ(alias_def->name, "StringList");

        auto type_ann = alias_def->target_type.get();
        ASSERT_NE(type_ann, nullptr);
        EXPECT_EQ(type_ann->name, "vector");

        ASSERT_EQ(type_ann->generic_args.size(), 1);
        EXPECT_EQ(type_ann->generic_args[0]->name, "string");
    }

    TEST_F(AstBaseTest, ValidatesNestedGenericTypeAlias) {
        auto ast = parse_code("typealias Matrix = map<string, vector<decimal>>");

        ASSERT_EQ(ast->type_aliases.size(), 1);
        auto alias_def = ast->type_aliases[0].get();

        EXPECT_EQ(alias_def->name, "Matrix");

        auto type_ann = alias_def->target_type.get();
        ASSERT_NE(type_ann, nullptr);
        EXPECT_EQ(type_ann->name, "map");

        ASSERT_EQ(type_ann->generic_args.size(), 2);
        EXPECT_EQ(type_ann->generic_args[0]->name, "string");

        auto nested_generic = type_ann->generic_args[1].get();
        ASSERT_NE(nested_generic, nullptr);
        EXPECT_EQ(nested_generic->name, "vector");
        ASSERT_EQ(nested_generic->generic_args.size(), 1);
        EXPECT_EQ(nested_generic->generic_args[0]->name, "decimal");
    }

    TEST_F(AstBaseTest, ValidatesTupleTypeAlias) {
        auto ast = parse_code("typealias ConfigTuple = (string, integer, bool)");

        ASSERT_EQ(ast->type_aliases.size(), 1);
        auto alias_def = ast->type_aliases[0].get();

        EXPECT_EQ(alias_def->name, "ConfigTuple");

        auto type_ann = alias_def->target_type.get();
        ASSERT_NE(type_ann, nullptr);

        auto tuple_type = dynamic_cast<TupleTypeAnnotation *>(type_ann);
        ASSERT_NE(tuple_type, nullptr) << "Expected target_type to be parsed as a TupleTypeAnnotation.";

        EXPECT_EQ(tuple_type->name, "tuple") << "Tuple types strictly have the base name 'tuple'.";
        ASSERT_EQ(tuple_type->element_types.size(), 3);
        EXPECT_EQ(tuple_type->element_types[0]->name, "string");
        EXPECT_EQ(tuple_type->element_types[1]->name, "integer");
        EXPECT_EQ(tuple_type->element_types[2]->name, "bool");
    }

    TEST_F(AstBaseTest, ValidatesTypeAliasWithModifiers) {
        auto ast = parse_code("@export @meta(version: 2) typealias PublicState = string");

        ASSERT_EQ(ast->type_aliases.size(), 1);
        auto alias_def = ast->type_aliases[0].get();

        EXPECT_EQ(alias_def->name, "PublicState");
        EXPECT_EQ(alias_def->target_type->name, "string");

        ASSERT_EQ(alias_def->modifiers.size(), 2);

        // Verify `@export` modifier (no arguments)
        EXPECT_EQ(alias_def->modifiers[0].name, "export");
        EXPECT_TRUE(alias_def->modifiers[0].arguments.empty());

        // Verify `@meta(version: 2)` modifier
        EXPECT_EQ(alias_def->modifiers[1].name, "meta");
        ASSERT_EQ(alias_def->modifiers[1].arguments.size(), 1);

        EXPECT_EQ(alias_def->modifiers[1].arguments[0].first, "version");

        auto version_val = dynamic_cast<NumberLiteral *>(alias_def->modifiers[1].arguments[0].second.get());
        ASSERT_NE(version_val, nullptr) << "Expected modifier argument to be a NumberLiteral.";
        EXPECT_EQ(version_val->value, "2");
    }

    TEST_F(AstBaseTest, ValidatesMultipleTypeAliases) {
        auto ast = parse_code(R"(
            typealias ID = string
            typealias Callback = (string, string)
            typealias Registry = map<ID, Callback>
        )");

        ASSERT_EQ(ast->type_aliases.size(), 3);

        EXPECT_EQ(ast->type_aliases[0]->name, "ID");
        EXPECT_EQ(ast->type_aliases[0]->target_type->name, "string");

        EXPECT_EQ(ast->type_aliases[1]->name, "Callback");
        auto tuple_type = dynamic_cast<TupleTypeAnnotation *>(ast->type_aliases[1]->target_type.get());
        ASSERT_NE(tuple_type, nullptr);
        ASSERT_EQ(tuple_type->element_types.size(), 2);

        EXPECT_EQ(ast->type_aliases[2]->name, "Registry");
        auto map_type = ast->type_aliases[2]->target_type.get();
        EXPECT_EQ(map_type->name, "map");
        ASSERT_EQ(map_type->generic_args.size(), 2);
        EXPECT_EQ(map_type->generic_args[0]->name, "ID");
        EXPECT_EQ(map_type->generic_args[1]->name, "Callback");
    }

    TEST_F(AstBaseTest, ValidatesTypeAliasUsageInAssignment) {
        // Proves that type aliases act as independent declarations and 
        // that their names can seamlessly be used as type annotations elsewhere in the AST.

        auto ast = parse_code(R"(
            typealias Point2D = (decimal, decimal)
            let origin: Point2D = (0.0, 0.0)
        )");

        // 1. Verify the declaration
        ASSERT_EQ(ast->type_aliases.size(), 1);
        EXPECT_EQ(ast->type_aliases[0]->name, "Point2D");

        auto tuple_ann = dynamic_cast<TupleTypeAnnotation *>(ast->type_aliases[0]->target_type.get());
        ASSERT_NE(tuple_ann, nullptr);
        ASSERT_EQ(tuple_ann->element_types.size(), 2);

        // 2. Verify the assignment
        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        ASSERT_EQ(assignment->targets.size(), 1);
        EXPECT_EQ(assignment->targets[0].first, "origin");

        // Verify the type annotation on the variable `origin` matches the alias name
        auto type_ann = assignment->targets[0].second.get();
        ASSERT_NE(type_ann, nullptr) << "Expected a type annotation on the variable.";
        EXPECT_EQ(type_ann->name, "Point2D") << "Variable should be typed as the alias 'Point2D'.";

        // Verify the value assigned is a tuple
        auto tuple_val = dynamic_cast<TupleLiteral *>(assignment->value.get());
        ASSERT_NE(tuple_val, nullptr) << "Expected value to be a TupleLiteral.";
        ASSERT_EQ(tuple_val->elements.size(), 2);
    }

    TEST_F(AstBaseTest, ValidatesTypeAliasOfTypeAlias) {
        // Semantically a bit useless, but AST must represent it accurately.
        auto ast = parse_code(R"(
            typealias A = string
            typealias B = A
            typealias C = B
        )");

        ASSERT_EQ(ast->type_aliases.size(), 3);

        EXPECT_EQ(ast->type_aliases[0]->name, "A");
        EXPECT_EQ(ast->type_aliases[0]->target_type->name, "string");

        EXPECT_EQ(ast->type_aliases[1]->name, "B");
        EXPECT_EQ(ast->type_aliases[1]->target_type->name, "A");

        EXPECT_EQ(ast->type_aliases[2]->name, "C");
        EXPECT_EQ(ast->type_aliases[2]->target_type->name, "B");
    }

    TEST_F(AstBaseTest, ValidatesTypeAliasWithGenericsContainingTuples) {
        // Proves we can parse deeply nested composition: a map where the value is a vector of tuples.
        auto ast = parse_code("typealias Graph = map<string, vector<(integer, integer)>>");

        ASSERT_EQ(ast->type_aliases.size(), 1);
        auto alias_def = ast->type_aliases[0].get();
        EXPECT_EQ(alias_def->name, "Graph");

        auto map_type = alias_def->target_type.get();
        ASSERT_NE(map_type, nullptr);
        EXPECT_EQ(map_type->name, "map");
        ASSERT_EQ(map_type->generic_args.size(), 2);

        // First arg: string
        EXPECT_EQ(map_type->generic_args[0]->name, "string");

        // Second arg: vector
        auto vector_type = map_type->generic_args[1].get();
        EXPECT_EQ(vector_type->name, "vector");
        ASSERT_EQ(vector_type->generic_args.size(), 1);

        // Vector's arg: tuple
        auto tuple_type = dynamic_cast<TupleTypeAnnotation *>(vector_type->generic_args[0].get());
        ASSERT_NE(tuple_type, nullptr) << "Expected vector's generic argument to be a TupleTypeAnnotation.";
        EXPECT_EQ(tuple_type->name, "tuple");
        ASSERT_EQ(tuple_type->element_types.size(), 2);
        EXPECT_EQ(tuple_type->element_types[0]->name, "integer");
        EXPECT_EQ(tuple_type->element_types[1]->name, "integer");
    }

    TEST_F(AstBaseTest, ValidatesTypeAliasWithTuplesContainingGenerics) {
        // Proves we can parse a tuple where elements are complex generic types.
        auto ast = parse_code("typealias ExecutionContext = (map<string, any>, vector<string>)");

        ASSERT_EQ(ast->type_aliases.size(), 1);
        auto tuple_type = dynamic_cast<TupleTypeAnnotation *>(ast->type_aliases[0]->target_type.get());

        ASSERT_NE(tuple_type, nullptr);
        ASSERT_EQ(tuple_type->element_types.size(), 2);

        // Element 0: map<string, any>
        auto map_type = tuple_type->element_types[0].get();
        EXPECT_EQ(map_type->name, "map");
        ASSERT_EQ(map_type->generic_args.size(), 2);
        EXPECT_EQ(map_type->generic_args[0]->name, "string");
        EXPECT_EQ(map_type->generic_args[1]->name, "any");

        // Element 1: vector<string>
        auto vector_type = tuple_type->element_types[1].get();
        EXPECT_EQ(vector_type->name, "vector");
        ASSERT_EQ(vector_type->generic_args.size(), 1);
        EXPECT_EQ(vector_type->generic_args[0]->name, "string");
    }

    TEST_F(AstBaseTest, ValidatesTypeAliasWithDeeplyNestedGenerics) {
        // Proves the parser doesn't trip on repetitive nested bracket closures
        auto ast = parse_code("typealias RussianDoll = Box<Box<Box<scalar>>>");

        ASSERT_EQ(ast->type_aliases.size(), 1);

        auto box1 = ast->type_aliases[0]->target_type.get();
        EXPECT_EQ(box1->name, "Box");
        ASSERT_EQ(box1->generic_args.size(), 1);

        auto box2 = box1->generic_args[0].get();
        EXPECT_EQ(box2->name, "Box");
        ASSERT_EQ(box2->generic_args.size(), 1);

        auto box3 = box2->generic_args[0].get();
        EXPECT_EQ(box3->name, "Box");
        ASSERT_EQ(box3->generic_args.size(), 1);

        auto scalar_type = box3->generic_args[0].get();
        EXPECT_EQ(scalar_type->name, "scalar");
        EXPECT_TRUE(scalar_type->generic_args.empty());
    }
}
