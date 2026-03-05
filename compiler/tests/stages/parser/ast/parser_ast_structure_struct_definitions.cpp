#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

class AstStructDefinitionTest : public testing::Test {
protected:
    std::shared_ptr<Program> parse_code(const std::string &code) {
        LexerStage lexer;
        auto lexer_result = lexer.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        });

        ParserStage parser;
        auto parser_result = parser.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            lexer_result
        });

        return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
    }

    StructDefinition *get_struct_definition(const std::shared_ptr<Program> &ast) {
        if (ast->struct_definitions.empty()) return nullptr;
        auto struct_def = dynamic_cast<StructDefinition *>(ast->struct_definitions[0].get());
        if (!struct_def) return nullptr;
        return struct_def;
    }
};

TEST_F(AstStructDefinitionTest, ValidatesStructDefinitionAndProgramSegregation) {

    auto ast = parse_code("struct Assumption { cagr: Decimal, yrs: Integer, name: String }");

    // 1. Verify Segregation
    ASSERT_EQ(ast->struct_definitions.size(), 1) << "Program should have exactly 1 struct definition";

    // 2. Verify Struct Geometry
    auto struct_def = ast->struct_definitions[0].get();
    EXPECT_EQ(struct_def->name, "Assumption");
    ASSERT_EQ(struct_def->fields.size(), 3);

    // Field 0: cagr: Decimal
    EXPECT_EQ(struct_def->fields[0].first, "cagr");
    EXPECT_EQ(struct_def->fields[0].second->name, "Decimal");

    // Field 1: yrs: Integer
    EXPECT_EQ(struct_def->fields[1].first, "yrs");
    EXPECT_EQ(struct_def->fields[1].second->name, "Integer");

    // Field 2: name: String
    EXPECT_EQ(struct_def->fields[2].first, "name");
    EXPECT_EQ(struct_def->fields[2].second->name, "String");
}

TEST_F(AstStructDefinitionTest, ValidatesStructWithDeeplyNestedComplexTypes) {
    // Proves that StructDefinition fields perfectly leverage the recursive type parser,
    // seamlessly handling generics, tuples, and nested combinations.

    std::string code =
        "struct ComplexModel {\n"
        "    rates: Vector<scalar>,\n"
        "    bounds: (integer, integer),\n"
        "    nested_generic: Result<Vector<scalar>, Error>,\n"
        "    matrix_tuple: (Matrix<scalar>, (integer, integer))\n"
        "}";

    auto ast = parse_code(code);

    ASSERT_EQ(ast->struct_definitions.size(), 1) << "Program must parse exactly 1 struct definition";
    auto struct_def = ast->struct_definitions[0].get();

    EXPECT_EQ(struct_def->name, "ComplexModel");
    ASSERT_EQ(struct_def->fields.size(), 4) << "Struct must have exactly 4 fields";

    // ==========================================
    // FIELD 0: rates: Vector<scalar>
    // ==========================================
    EXPECT_EQ(struct_def->fields[0].first, "rates");
    auto field0_type = struct_def->fields[0].second.get();
    EXPECT_EQ(field0_type->name, "Vector");
    ASSERT_EQ(field0_type->generic_args.size(), 1);
    EXPECT_EQ(field0_type->generic_args[0]->name, "scalar");

    // ==========================================
    // FIELD 1: bounds: (integer, integer)
    // ==========================================
    EXPECT_EQ(struct_def->fields[1].first, "bounds");
    auto field1_tuple = dynamic_cast<TupleTypeAnnotation*>(struct_def->fields[1].second.get());
    ASSERT_NE(field1_tuple, nullptr) << "Field 'bounds' must be a TupleTypeAnnotation";
    ASSERT_EQ(field1_tuple->element_types.size(), 2);
    EXPECT_EQ(field1_tuple->element_types[0]->name, "integer");
    EXPECT_EQ(field1_tuple->element_types[1]->name, "integer");

    // ==========================================
    // FIELD 2: nested_generic: Result<Vector<scalar>, Error>
    // ==========================================
    EXPECT_EQ(struct_def->fields[2].first, "nested_generic");
    auto field2_type = struct_def->fields[2].second.get();
    EXPECT_EQ(field2_type->name, "Result");
    ASSERT_EQ(field2_type->generic_args.size(), 2) << "Result must have 2 generic arguments";

    // Arg 0: Vector<scalar>
    auto arg0 = field2_type->generic_args[0].get();
    EXPECT_EQ(arg0->name, "Vector");
    ASSERT_EQ(arg0->generic_args.size(), 1);
    EXPECT_EQ(arg0->generic_args[0]->name, "scalar");

    // Arg 1: Error
    auto arg1 = field2_type->generic_args[1].get();
    EXPECT_EQ(arg1->name, "Error");
    EXPECT_TRUE(arg1->generic_args.empty());

    // ==========================================
    // FIELD 3: matrix_tuple: (Matrix<scalar>, (integer, integer))
    // ==========================================
    EXPECT_EQ(struct_def->fields[3].first, "matrix_tuple");
    auto field3_tuple = dynamic_cast<TupleTypeAnnotation*>(struct_def->fields[3].second.get());
    ASSERT_NE(field3_tuple, nullptr) << "Field 'matrix_tuple' must be a TupleTypeAnnotation";
    ASSERT_EQ(field3_tuple->element_types.size(), 2);

    // Element 0: Matrix<scalar>
    auto elem0 = field3_tuple->element_types[0].get();
    EXPECT_EQ(elem0->name, "Matrix");
    ASSERT_EQ(elem0->generic_args.size(), 1);
    EXPECT_EQ(elem0->generic_args[0]->name, "scalar");

    // Element 1: Nested Tuple (integer, integer)
    auto elem1_nested_tuple = dynamic_cast<TupleTypeAnnotation*>(field3_tuple->element_types[1].get());
    ASSERT_NE(elem1_nested_tuple, nullptr) << "Second element of matrix_tuple must be a nested TupleTypeAnnotation";
    ASSERT_EQ(elem1_nested_tuple->element_types.size(), 2);
    EXPECT_EQ(elem1_nested_tuple->element_types[0]->name, "integer");
    EXPECT_EQ(elem1_nested_tuple->element_types[1]->name, "integer");
}