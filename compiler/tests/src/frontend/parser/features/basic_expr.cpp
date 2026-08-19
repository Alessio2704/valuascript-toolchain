#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ExprVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({.name = "IntegerLiteral", .code = "42", .verifier = IsNumber("42")});
            reg({.name = "IntegerWithSeparators", .code = "1_000_000", .verifier = IsNumber("1_000_000")});
            reg({.name = "DecimalLiteral", .code = "3.14159", .verifier = IsNumber("3.14159")});
            reg({.name = "DecimalWithSeparators", .code = "1_234.56_78", .verifier = IsNumber("1_234.56_78")});
            reg({.name = "ZeroLiteral", .code = "0", .verifier = IsNumber("0")});
            reg({.name = "DecimalStartingWithZero", .code = "0.001", .verifier = IsNumber("0.001")});
            reg({.name = "DecimalNotStartingWithZero", .code = ".5", .verifier = IsNumber(".5")});
            reg({.name = "PercentageDecimalNotStartingWithZero", .code = ".5%", .verifier = IsPercentage(".5%")});

            reg({.name = "BooleanTrue", .code = "true", .verifier = IsBoolean(true)});
            reg({.name = "BooleanFalse", .code = "false", .verifier = IsBoolean(false)});

            reg({.name = "SimpleString", .code = "\"hello\"", .verifier = IsString("\"hello\"")});
            reg({.name = "StringWithSpaces", .code = "\"hello world\"", .verifier = IsString("\"hello world\"")});
            reg({.name = "EmptyString", .code = "\"\"", .verifier = IsString("\"\"")});
            reg({.name = "StringWithSpecialCharacters", .code = "\"@#$%^&*()_+{}|:<>?\"", .verifier = IsString("\"@#$%^&*()_+{}|:<>?\"")});

            reg({.name = "IntegerPercentage", .code = "1%", .verifier = IsPercentage("1%")});
            reg({.name = "DecimalPercentage", .code = "2.5%", .verifier = IsPercentage("2.5%")});
            reg({.name = "PercentageWithSeparators", .code = "1_000.5%", .verifier = IsPercentage("1_000.5%")});
            reg({.name = "PercentageWithComplexSeparators", .code = "1_000.5_000_1%", .verifier = IsPercentage("1_000.5_000_1%")});

            reg({.name = "StandardIdentifier", .code = "my_variable_name", .verifier = IsIdentifier("my_variable_name")});
            reg({.name = "CamelCaseIdentifier", .code = "myVariableName", .verifier = IsIdentifier("myVariableName")});
            reg({.name = "PascalCaseIdentifier", .code = "MyVariableName", .verifier = IsIdentifier("MyVariableName")});
            reg({.name = "IdentifierWithNumbers", .code = "var42_item7", .verifier = IsIdentifier("var42_item7")});
            reg({.name = "SingleUnderscoreIdentifier", .code = "_", .verifier = IsIdentifier("_")});
            reg({.name = "DoubleUnderscoreIdentifier", .code = "__internal_id__", .verifier = IsIdentifier("__internal_id__")});

            reg({.name = "SelfExpression", .code = "self", .verifier = IsSelf()});

            return true;
        }();
    }
}
