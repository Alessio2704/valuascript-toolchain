#include "context_registry.h"
#include "context_names.h"

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_top_level_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                .name = ContextNames::TopLevelWrapper,
                .input_types = {
                    InjectableType::TopLevel, InjectableType::Function, InjectableType::Struct,
                    InjectableType::Enum, InjectableType::Extension, InjectableType::TypeAlias,
                    InjectableType::Import, InjectableType::Directive, InjectableType::StrongStatement
                },
                .output_type = InjectableType::TopLevel,
                .prefix = "",
                .suffix = "",
                .transform_verifier = [](const UniversalVerifier& v) { return v; },
                .block_context = BlockContext::TopLevel
            }
        };

        return contexts;
    }
}
