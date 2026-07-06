#include "context_registry.h"
#include "recovery_sentinel.h"
#include "context_names.h"
#include <utility>

namespace valuascript::compiler::test
{
    namespace
    {
        StmtVerifier extract_stmt_v(const UniversalVerifier& v)
        {
            return std::visit(
                overloaded{
                    [](const StmtVerifier& ver) { return ver; },
                    [](const AssignmentVerifier& ver) { return StmtVerifier(ver); },
                    [](const ReassignmentVerifier& ver) { return StmtVerifier(ver); },
                    [](const ReturnVerifier& ver) { return StmtVerifier(ver); },
                    [](const ExprStmtVerifier& ver) { return StmtVerifier(ver); },
                    [](const auto&) { return StmtVerifier(); }
                }, v);
        }

        std::vector<StmtVerifier> build_block_body(const UniversalVerifier& v,
                                                   const std::vector<RecoveryBlock>& pre,
                                                   const std::vector<RecoveryBlock>& post)
        {
            std::vector<StmtVerifier> body;
            auto add_blocks = [&](const std::vector<RecoveryBlock>& blocks)
            {
                for (const auto& b : blocks)
                {
                    ProgramSpec temp;
                    b.add_to_spec(temp);
                    body.insert(body.end(), temp.execution_steps.begin(), temp.execution_steps.end());
                }
            };

            add_blocks(pre);
            body.push_back(extract_stmt_v(v));
            add_blocks(post);
            return body;
        }
    }

    const std::vector<Context>& ContextRegistry::get_block_contexts_impl()
    {
        static const std::vector<Context> contexts = {
            {
                .name = ContextNames::FunctionBodyWrapper,
                .input_types = {InjectableType::WeakStatement, InjectableType::StrongStatement},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_wrapper() -> void {\n  ",
                .suffix = "\n}\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    auto creator = [](const std::vector<StmtVerifier>& body) {
                        return UniversalVerifier(IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, body));
                    };
                    return creator({extract_stmt_v(v)});
                },
                .block_context = BlockContext::FunctionBody,
                .transform_verifier_block = [](const UniversalVerifier& v, const auto& pre, const auto& post)
                {
                    auto creator = [](const std::vector<StmtVerifier>& body) {
                        return UniversalVerifier(IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, body));
                    };
                    return creator(build_block_body(v, pre, post));
                }
            }
        };

        return contexts;
    }
}
