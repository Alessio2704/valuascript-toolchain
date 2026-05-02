#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("IntegerLiteral", "42", IsNumber("42"));
            reg("IntegerWithSeparators", "1_000_000", IsNumber("1_000_000"));
            reg("DecimalLiteral", "3.14159", IsNumber("3.14159"));
            reg("DecimalWithSeparators", "1_234.56_78", IsNumber("1_234.56_78"));
            reg("ZeroLiteral", "0", IsNumber("0"));
            reg("DecimalStartingWithZero", "0.001", IsNumber("0.001"));
            reg("DecimalNotStartingWithZero", ".5", IsNumber(".5"));
            reg("PercentageDecimalNotStartingWithZero", ".5%", IsPercentage(".5%"));

            reg("BooleanTrue", "true", IsBoolean(true));
            reg("BooleanFalse", "false", IsBoolean(false));

            reg("SimpleString", "\"hello\"", IsString("\"hello\""));
            reg("StringWithSpaces", "\"hello world\"", IsString("\"hello world\""));
            reg("EmptyString", "\"\"", IsString("\"\""));
            reg("StringWithSpecialCharacters", "\"@#$%^&*()_+{}|:<>?\"", IsString("\"@#$%^&*()_+{}|:<>?\""));

            reg("IntegerPercentage", "1%", IsPercentage("1%"));
            reg("DecimalPercentage", "2.5%", IsPercentage("2.5%"));
            reg("PercentageWithSeparators", "1_000.5%", IsPercentage("1_000.5%"));
            reg("PercentageWithComplexSeparators", "1_000.5_000_1%", IsPercentage("1_000.5_000_1%"));

            reg("StandardIdentifier", "my_variable_name", IsIdentifier("my_variable_name"));
            reg("CamelCaseIdentifier", "myVariableName", IsIdentifier("myVariableName"));
            reg("PascalCaseIdentifier", "MyVariableName", IsIdentifier("MyVariableName"));
            reg("IdentifierWithNumbers", "var42_item7", IsIdentifier("var42_item7"));
            reg("SingleUnderscoreIdentifier", "_", IsIdentifier("_"));
            reg("DoubleUnderscoreIdentifier", "__internal_id__", IsIdentifier("__internal_id__"));

            reg("SelfExpression", "self", IsSelf());

            return true;
        }();
    }
}
