#include "frontend/parser/helpers/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesStructDefinitionAndProgramSegregation) {
        auto ast = parse_code("struct Assumption { cagr: Decimal, yrs: Integer, name: String }");

        // 1. Verify Segregation
        ASSERT_EQ(ast->struct_definitions.size(), 1) << "Program should have exactly 1 struct definition";

        // 2. Verify Struct Geometry
        auto struct_def = ast->struct_definitions[0].get();
        EXPECT_EQ(struct_def->name, "Assumption");
        ASSERT_EQ(struct_def->fields.size(), 3);

        // Field 0: cagr: Decimal
        EXPECT_EQ(struct_def->fields[0].name, "cagr");
        EXPECT_EQ(struct_def->fields[0].type->name, "Decimal");

        // Field 1: yrs: Integer
        EXPECT_EQ(struct_def->fields[1].name, "yrs");
        EXPECT_EQ(struct_def->fields[1].type->name, "Integer");

        // Field 2: name: String
        EXPECT_EQ(struct_def->fields[2].name, "name");
        EXPECT_EQ(struct_def->fields[2].type->name, "String");
    }

    TEST_F(AstBaseTest, ValidatesStructWithDeeplyNestedComplexTypes) {
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
        EXPECT_EQ(struct_def->fields[0].name, "rates");
        auto field0_type = struct_def->fields[0].type.get();
        EXPECT_EQ(field0_type->name, "Vector");
        ASSERT_EQ(field0_type->generic_args.size(), 1);
        EXPECT_EQ(field0_type->generic_args[0]->name, "scalar");

        // ==========================================
        // FIELD 1: bounds: (integer, integer)
        // ==========================================
        EXPECT_EQ(struct_def->fields[1].name, "bounds");
        auto field1_tuple = dynamic_cast<TupleTypeAnnotation *>(struct_def->fields[1].type.get());
        ASSERT_NE(field1_tuple, nullptr) << "Field 'bounds' must be a TupleTypeAnnotation";
        ASSERT_EQ(field1_tuple->element_types.size(), 2);
        EXPECT_EQ(field1_tuple->element_types[0]->name, "integer");
        EXPECT_EQ(field1_tuple->element_types[1]->name, "integer");

        // ==========================================
        // FIELD 2: nested_generic: Result<Vector<scalar>, Error>
        // ==========================================
        EXPECT_EQ(struct_def->fields[2].name, "nested_generic");
        auto field2_type = struct_def->fields[2].type.get();
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
        EXPECT_EQ(struct_def->fields[3].name, "matrix_tuple");
        auto field3_tuple = dynamic_cast<TupleTypeAnnotation *>(struct_def->fields[3].type.get());
        ASSERT_NE(field3_tuple, nullptr) << "Field 'matrix_tuple' must be a TupleTypeAnnotation";
        ASSERT_EQ(field3_tuple->element_types.size(), 2);

        // Element 0: Matrix<scalar>
        auto elem0 = field3_tuple->element_types[0].get();
        EXPECT_EQ(elem0->name, "Matrix");
        ASSERT_EQ(elem0->generic_args.size(), 1);
        EXPECT_EQ(elem0->generic_args[0]->name, "scalar");

        // Element 1: Nested Tuple (integer, integer)
        auto elem1_nested_tuple = dynamic_cast<TupleTypeAnnotation *>(field3_tuple->element_types[1].get());
        ASSERT_NE(elem1_nested_tuple, nullptr) << "Second element of matrix_tuple must be a nested TupleTypeAnnotation";
        ASSERT_EQ(elem1_nested_tuple->element_types.size(), 2);
        EXPECT_EQ(elem1_nested_tuple->element_types[0]->name, "integer");
        EXPECT_EQ(elem1_nested_tuple->element_types[1]->name, "integer");
    }
}
