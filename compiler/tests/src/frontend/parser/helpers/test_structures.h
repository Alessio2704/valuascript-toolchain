#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "frontend/parser/node_matchers/node_matchers.h"
#include "context_override.h"

namespace valuascript::compiler::test
{
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
        bool has_nested_block_sentinels = false;
        std::optional<SentinelKind> inner_pre_kind = std::nullopt;
        bool is_inner_pre_modified = false;
        std::optional<SentinelKind> inner_post_kind = std::nullopt;
        bool is_inner_post_modified = false;

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
