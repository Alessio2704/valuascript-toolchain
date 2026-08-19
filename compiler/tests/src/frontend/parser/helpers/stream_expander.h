#pragma once

#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <string_view>

#include "test_structures.h"
#include "sentinel_kind.h"
#include "context_override.h"
#include "universal_verifier.h"
#include "injectable_type.h"
#include "frontend/parser/expansion_and_sentinels/expansion_policy.h"

namespace valuascript::compiler::test
{
    using ExpansionCallback = std::function<void(ProcessingItem&&)>;

    class StreamExpander
    {
    public:
        static std::vector<ProcessingItem> apply_context_augmentations(
            InjectableType type,
            const std::string& snippet,
            const UniversalVerifier& verifier,
            const std::string& group_name,
            const std::vector<std::string_view>& skip_contexts = {},
            const std::vector<ContextOverrideAny>& context_overrides = {},
            const std::vector<SentinelKind>& excluded_sentinels = {},
            const std::vector<SentinelKind>& accepted_sentinels = {});

        static void expand_to_top_level_stream(
            std::vector<ProcessingItem> items,
            const ExpansionCallback& callback,
            bool inject_sentinels = false,
            std::optional<ExpansionPolicy> policy_override = std::nullopt);
    };
}
