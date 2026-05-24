// #include "frontend/parser/helpers/parser_test_base.h"
//
// namespace valuascript::compiler::test
// {
//     using E = ParserErrorCode;
//
//     namespace
//     {
//         const bool _ = []()
//         {
//             auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ExprVerifier>& v)
//             {
//                 ErrorRegistry::add(n, c, errs, v);
//             };
//
//             reg("DictMissingKey", "{ : 1, y: 2 }",
//                 {
//                     {E::ExpectedDictionaryKey, 1, 3, 1, 4}
//                 },
//                 IsDict({
//                     {"<error>", {}, IsNumber("1")},
//                     {"y", {}, IsNumber("2")}
//                 })
//             );
//
//             return true;
//         }();
//     }
// }
