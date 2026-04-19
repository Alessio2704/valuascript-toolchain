#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class BasicLiteralExprSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(BasicLiteralExprSuccessPathTest, IntegerLiteral)
    {
        ExpectValidExpression("42", IsNumber("42"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, IntegerWithSeparators)
    {
        ExpectValidExpression("1_000_000", IsNumber("1_000_000"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, DecimalLiteral)
    {
        ExpectValidExpression("3.14159", IsNumber("3.14159"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, DecimalWithSeparators)
    {
        ExpectValidExpression("1_234.56_78", IsNumber("1_234.56_78"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, ZeroLiteral)
    {
        ExpectValidExpression("0", IsNumber("0"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, DecimalStartingWithZero)
    {
        ExpectValidExpression("0.001", IsNumber("0.001"));
    }


    TEST_F(BasicLiteralExprSuccessPathTest, BooleanTrue)
    {
        ExpectValidExpression("true", IsBoolean(true));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, BooleanFalse)
    {
        ExpectValidExpression("false", IsBoolean(false));
    }


    TEST_F(BasicLiteralExprSuccessPathTest, SimpleString)
    {
        ExpectValidExpression("\"hello\"", IsString("\"hello\""));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, StringWithSpaces)
    {
        ExpectValidExpression("\"hello world\"", IsString("\"hello world\""));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, EmptyString)
    {
        ExpectValidExpression("\"\"", IsString("\"\""));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, StringWithSpecialCharacters)
    {
        ExpectValidExpression("\"@#$%^&*()_+{}|:<>?\"", IsString("\"@#$%^&*()_+{}|:<>?\""));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, IntegerPercentage)
    {
        ExpectValidExpression("1%", IsPercentage("1%"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, DecimalPercentage)
    {
        ExpectValidExpression("2.5%", IsPercentage("2.5%"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, PercentageWithSeparators)
    {
        ExpectValidExpression("1_000.5%", IsPercentage("1_000.5%"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, PercentageWithComplexSeparators)
    {
        ExpectValidExpression("1_000.5_000_1%", IsPercentage("1_000.5_000_1%"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, StandardIdentifier)
    {
        ExpectValidExpression("my_variable_name", IsIdentifier("my_variable_name"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, CamelCaseIdentifier)
    {
        ExpectValidExpression("myVariableName", IsIdentifier("myVariableName"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, PascalCaseIdentifier)
    {
        ExpectValidExpression("MyVariableName", IsIdentifier("MyVariableName"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, IdentifierWithNumbers)
    {
        ExpectValidExpression("var42_item7", IsIdentifier("var42_item7"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, SingleUnderscoreIdentifier)
    {
        ExpectValidExpression("_", IsIdentifier("_"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, DoubleUnderscoreIdentifier)
    {
        ExpectValidExpression("__internal_id__", IsIdentifier("__internal_id__"));
    }

    TEST_F(BasicLiteralExprSuccessPathTest, SelfExpression)
    {
        ExpectValidExpression("self", IsSelf());
    }
}
