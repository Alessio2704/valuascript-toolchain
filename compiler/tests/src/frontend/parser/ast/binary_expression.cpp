#include "frontend/parser/helpers/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, RejectsChainedComparisons) {
        try {
            parse_code("let a = 1 < 2 < 3");
            FAIL() << "Parser should have thrown an exception for chained comparisons.";
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_code(), ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations);
        }
    }

    TEST_F(AstBaseTest, RejectsChainedEquality) {
        try {
            parse_code("let a = x == y == z");
            FAIL() << "Parser should have thrown an exception for chained equality.";
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_code(), ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations);
        }
    }
}
