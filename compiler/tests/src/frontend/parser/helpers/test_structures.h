#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "node_matchers.h"
#include "error_registry.h"

namespace valuascript::compiler::test
{
    inline bool is_terminal_type(InjectableType type)
    {
        switch (type)
        {
        case InjectableType::TopLevel:
        case InjectableType::Import:
        case InjectableType::Directive:
        case InjectableType::Function:
        case InjectableType::Extension:
        case InjectableType::Struct:
        case InjectableType::Enum:
        case InjectableType::TypeAlias:
            return true;
        default:
            return false;
        }
    }

    struct ContextOverrideAny
    {
        std::string_view context_name;
        std::optional<std::vector<ParserExpectedError>> errors = std::nullopt;
        std::optional<UniversalVerifier> verifier = std::nullopt;

        template <typename T>
        ContextOverrideAny(const ContextOverride<T>& typed)
            : context_name(typed.context_name),
              errors(typed.errors),
              verifier(typed.verifier.has_value() ? std::make_optional(typed.verifier->value) : std::nullopt)
        {
        }

        ContextOverrideAny(std::string_view name,
                           std::optional<std::vector<ParserExpectedError>> errs = std::nullopt,
                           std::optional<UniversalVerifier> ver = std::nullopt)
            : context_name(name), errors(std::move(errs)), verifier(std::move(ver))
        {
        }
    };

    template <typename T>
    inline std::vector<ContextOverrideAny> to_any_overrides(const std::vector<ContextOverride<T>>& typed_overrides)
    {
        std::vector<ContextOverrideAny> result;
        result.reserve(typed_overrides.size());
        for (const auto& item : typed_overrides)
        {
            result.emplace_back(item);
        }
        return result;
    }

    struct ProcessingItem
    {
        InjectableType type;
        std::string code;
        UniversalVerifier verifier;
        std::string path_name;
        std::string cumulative_prefix;
        int depth;
        int recursion_depth;
        std::vector<std::string_view> skip_contexts;
        bool is_skipped;
        std::vector<ContextOverrideAny> context_overrides = {};
        std::optional<std::vector<ParserExpectedError>> custom_errors = std::nullopt;
    };

    struct RecoveryScenario
    {
        std::string path_name;
        std::string full_code;
        ProgramSpec spec;
        std::vector<ParserExpectedError> shifted_errors;
        int depth;
    };
}
