#include "context_registry.h"
#include "recovery_sentinel.h"
#include <type_traits>

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_block_contexts_impl()
    {
        Context ctx;
        ctx.name = "function_body_wrapper";
        ctx.input_types = {InjectableType::WeakStatement, InjectableType::StrongStatement};
        ctx.output_type = InjectableType::TopLevel;
        ctx.prefix = "func ctx_wrapper() -> void {\n  ";
        ctx.suffix = "\n}\n";
        ctx.is_block_context = true;

        auto extract_stmt_v = [](const UniversalVerifier& v) -> StmtVerifier
        {
            return std::visit([](auto&& ver) -> StmtVerifier
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
        };

        ctx.transform_verifier = [extract_stmt_v](const UniversalVerifier& v) -> UniversalVerifier
        {
            return UniversalVerifier(IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {extract_stmt_v(v)}));
        };

        ctx.transform_verifier_block = [extract_stmt_v](const UniversalVerifier& v,
                                                        const std::vector<RecoveryBlock>& pre,
                                                        const std::vector<RecoveryBlock>& post) -> UniversalVerifier
        {
            std::vector<StmtVerifier> body;

            for (const auto& b : pre)
            {
                ProgramSpec temp;
                b.add_to_spec(temp);
                body.insert(body.end(), temp.execution_steps.begin(), temp.execution_steps.end());
            }

            body.push_back(extract_stmt_v(v));

            for (const auto& b : post)
            {
                ProgramSpec temp;
                b.add_to_spec(temp);
                body.insert(body.end(), temp.execution_steps.begin(), temp.execution_steps.end());
            }

            return UniversalVerifier(IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, body));
        };

        return {ctx};
    }
}
