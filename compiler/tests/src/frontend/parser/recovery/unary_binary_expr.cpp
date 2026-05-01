#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& errs, const auto& v) { ErrorRegistry::add(n, c, errs, v); };

            // reg("BinaryMissingLeft", "1 + ",
            //     std::vector<ExpectedError>{
            //         {ValuascriptErrorCode::InvalidExpression, 1, 4, 1, 5}
            //     },
            //     IsBinary(TokenType::Plus,
            //              IsNumber("1"),
            //              IsNull()
            //     ));

            return true;
        }();
    }
}
