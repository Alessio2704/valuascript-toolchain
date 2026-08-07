#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "node_matchers.h"
#include "error_registry.h"

namespace valuascript::compiler::test
{
    struct ContextOverrideAny
    {
        std::string_view context_name;
        std::optional<std::vector<ParserExpectedError>> errors = std::nullopt;
        std::optional<UniversalVerifier> verifier = std::nullopt;
        std::vector<SentinelKind> excluded_sentinels = {};
        std::vector<SentinelKind> accepted_sentinels = {};

        template <typename T>
        ContextOverrideAny(const ContextOverride<T>& typed)
            : context_name(typed.context_name),
              errors(typed.errors),
              verifier(typed.verifier.has_value() ? std::make_optional(typed.verifier->value) : std::nullopt),
              excluded_sentinels(typed.excluded_sentinels),
              accepted_sentinels(typed.accepted_sentinels)
        {
        }

        ContextOverrideAny(std::string_view name,
                           std::optional<std::vector<ParserExpectedError>> errs = std::nullopt,
                           std::optional<UniversalVerifier> ver = std::nullopt,
                           std::vector<SentinelKind> excluded = {},
                           std::vector<SentinelKind> accepted = {})
            : context_name(name), errors(std::move(errs)), verifier(std::move(ver)),
              excluded_sentinels(std::move(excluded)), accepted_sentinels(std::move(accepted))
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

    struct ContextStep
    {
        std::string_view context_name;
        bool is_recursion = false;
    };

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
        std::vector<SentinelKind> excluded_sentinels = {};
        std::vector<SentinelKind> accepted_sentinels = {};
        std::vector<ContextStep> context_history = {};

        [[nodiscard]] bool has_context(std::string_view name) const
        {
            return std::any_of(context_history.begin(), context_history.end(), [&](const ContextStep& step)
            {
                return step.context_name == name;
            });
        }

        [[nodiscard]] size_t transition_count() const
        {
            return context_history.size();
        }
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
