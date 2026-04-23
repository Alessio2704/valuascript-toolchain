#include "context_registry.h"
#include "spec_adder.h"
#include <cassert>
#include <type_traits>

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_block_contexts_impl()
    {
        return {
            {
                "function_body_wrapper",
                {InjectableType::WeakStatement, InjectableType::StrongStatement},
                InjectableType::TopLevel,
                "func ctx_wrapper() -> void {\n  ", "\n}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    auto stmt_v = std::visit([](auto&& ver) -> StmtVerifier
                    {
                        if constexpr (std::is_same_v<std::decay_t<decltype(ver)>, StmtVerifier>) return ver;
                        else if constexpr (std::is_same_v<std::decay_t<decltype(ver)>, ReturnVerifier>)
                            return
                                StmtVerifier(ver);
                        else if constexpr (std::is_same_v<std::decay_t<decltype(ver)>, AssignmentVerifier>)
                            return
                                StmtVerifier(ver);
                        else if constexpr (std::is_same_v<std::decay_t<decltype(ver)>, ReassignmentVerifier>)
                            return
                                StmtVerifier(ver);
                        else if constexpr (std::is_same_v<std::decay_t<decltype(ver)>, ExprStmtVerifier>)
                            return
                                StmtVerifier(ver);
                        else return StmtVerifier();
                    }, v);
                    return UniversalVerifier(IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {stmt_v}));
                }
            }
        };
    }
}
