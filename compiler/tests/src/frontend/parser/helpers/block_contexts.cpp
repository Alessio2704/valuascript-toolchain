#include "context_registry.h"
#include "recovery_sentinel.h"
#include "context_names.h"
#include <utility>
#include <optional>

namespace valuascript::compiler::test
{
    namespace
    {
        std::optional<StmtVerifier> extract_stmt_v(const UniversalVerifier& v)
        {
            if (std::holds_alternative<NullVerifier>(v))
            {
                return std::nullopt;
            }

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
            if (auto stmt = extract_stmt_v(v))
            {
                body.push_back(std::move(*stmt));
            }
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
                    if (auto stmt = extract_stmt_v(v))
                    {
                        return creator({std::move(*stmt)});
                    }
                    return creator({});
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
