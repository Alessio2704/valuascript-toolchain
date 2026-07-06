#include "context_registry.h"
#include "recovery_sentinel.h"
#include "context_names.h"
#include <utility>

namespace valuascript::compiler::test
{
    namespace
    {
        void add_to_spec(ProgramSpec& spec, const UniversalVerifier& v)
        {
            std::visit(overloaded{
                [&](const StmtVerifier& ver) { spec.execution_steps.push_back(ver); },
                [&](const AssignmentVerifier& ver) { spec.execution_steps.push_back(StmtVerifier(ver)); },
                [&](const ReassignmentVerifier& ver) { spec.execution_steps.push_back(StmtVerifier(ver)); },
                [&](const ReturnVerifier& ver) { spec.execution_steps.push_back(StmtVerifier(ver)); },
                [&](const ExprStmtVerifier& ver) { spec.execution_steps.push_back(StmtVerifier(ver)); },
                [&](const FuncVerifier& ver) { spec.functions.push_back(ver); },
                [&](const StructVerifier& ver) { spec.structs.push_back(ver); },
                [&](const EnumVerifier& ver) { spec.enums.push_back(ver); },
                [&](const AliasVerifier& ver) { spec.type_aliases.push_back(ver); },
                [&](const ExtVerifier& ver) { spec.extensions.push_back(ver); },
                [&](const auto&) {}
            }, v);
        }

        ProgramSpec build_extension_spec(const UniversalVerifier& v,
                                         const std::vector<RecoveryBlock>& pre,
                                         const std::vector<RecoveryBlock>& post)
        {
            ProgramSpec spec;
            for (const auto& b : pre) b.add_to_spec(spec);
            add_to_spec(spec, v);
            for (const auto& b : post) b.add_to_spec(spec);
            return spec;
        }
    }

    const std::vector<Context>& ContextRegistry::get_extension_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                .name = ContextNames::ExtensionBodyWrapper,
                .input_types = {InjectableType::Function, InjectableType::Struct, InjectableType::Enum, InjectableType::StrongStatement},
                .output_type = InjectableType::TopLevel,
                .prefix = "extension ctx_target {\n  ",
                .suffix = "\n}\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    ProgramSpec spec;
                    add_to_spec(spec, v);
                    return UniversalVerifier(IsExtensionDef({}, IsType("ctx_target"), std::move(spec)));
                },
                .block_context = BlockContext::ExtensionBody,
                .transform_verifier_block = [](const UniversalVerifier& v, const auto& pre, const auto& post)
                {
                    return UniversalVerifier(IsExtensionDef({}, IsType("ctx_target"), build_extension_spec(v, pre, post)));
                }
            }
        };

        return contexts;
    }
}
