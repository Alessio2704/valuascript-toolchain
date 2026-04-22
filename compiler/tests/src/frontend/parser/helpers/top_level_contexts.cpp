#include "context_registry.h"
#include "spec_adder.h"

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_top_level_contexts()
    {
        return {
            {
                "top_level_identity", NestingLevel::TopLevel,
                {
                    InjectableType::Import, InjectableType::Directive, InjectableType::Function,
                    InjectableType::Struct, InjectableType::Enum, InjectableType::TypeAlias,
                    InjectableType::Statement
                },
                "", "\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    std::visit([&](auto&& verifier) { SpecAdder::add(s, verifier); }, v);
                }
            }
        };
    }
}
