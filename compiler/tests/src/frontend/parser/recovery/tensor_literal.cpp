#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ExprVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("TensorDoubleComma", "[1,, 2]",
                {
                    {ValuascriptErrorCode::InvalidExpression, 1, 4, 1, 5}
                },
                IsTensor({
                    IsNumber("1"), IsNumber("2")
                })
            );

            // reg("MissingClosingBracket", "[1, 2",
            //     {
            //         {ValuascriptErrorCode::UnmatchedBracketAfterTensorElements, 1, 5, 1, 6}
            //     },
            //     IsTensor({
            //         IsNumber("1"), IsNumber("2")
            //     })
            // );

            // reg("InvalidExpressionAsFirst", "[*, 1]",
            //     {
            //         {ValuascriptErrorCode::InvalidExpression, 1, 2, 1, 3}
            //     },
            //     IsTensor({
            //         IsNull(), IsNumber("1")
            //     })
            // );
            //
            // reg("InvalidExpressionAsSecond", "[1, *]",
            //     {
            //         {ValuascriptErrorCode::InvalidExpression, 1, 4, 1, 5}
            //     },
            //     IsTensor({
            //         IsNumber("1"), IsNull()
            //     })
            // );

            return true;
        }();
    }
}
