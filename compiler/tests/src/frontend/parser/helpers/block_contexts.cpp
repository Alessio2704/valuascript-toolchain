#include "context_registry.h"
#include "recovery_sentinel.h"
#include <utility>

namespace valuascript::compiler::test
{
    namespace
    {
        StmtVerifier extract_stmt_v(const UniversalVerifier& v)
        {
            return std::visit(overloaded{
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

    namespace
    {
        using ASTCreator = std::function<UniversalVerifier(std::vector<StmtVerifier>)>;

        Context make_block_context(std::string name, std::string prefix, std::string suffix, ASTCreator creator)
        {
            Context ctx;
            ctx.name = std::move(name);
            ctx.prefix = std::move(prefix);
            ctx.suffix = std::move(suffix);
            ctx.input_types = {InjectableType::WeakStatement, InjectableType::StrongStatement};
            ctx.output_type = InjectableType::TopLevel;
            ctx.is_block_context = true;

            ctx.transform_verifier = [creator](const UniversalVerifier& v)
            {
                return creator({extract_stmt_v(v)});
            };

            ctx.transform_verifier_block = [creator](const UniversalVerifier& v, const auto& pre, const auto& post)
            {
                return creator(build_block_body(v, pre, post));
            };

            return ctx;
        }
    }

    const std::vector<Context>& ContextRegistry::get_block_contexts_impl()
    {
        static const std::vector<Context> contexts = {
            make_block_context("function_body_wrapper", "func ctx_wrapper() -> void {\n  ", "\n}\n",
                               [](auto body)
                               {
                                   return UniversalVerifier(
                                       IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, std::move(body)));
                               })
        };

        return contexts;
    }
}
