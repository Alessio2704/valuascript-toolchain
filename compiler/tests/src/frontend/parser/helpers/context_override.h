#pragma once

#include <optional>
#include <string_view>
#include <vector>
#include "core/valuascript_exception.h"
#include "sentinel_kind.h"
#include "universal_verifier.h"

namespace valuascript::compiler::test
{
    struct ParserExpectedError
    {
        ValuascriptErrorCode code;
        size_t line_start = 0;
        size_t column_start = 0;
        size_t line_end = 0;
        size_t column_end = 0;
        bool skip_span_check = false;
    };

    using PErr = ParserExpectedError;

    template <typename T = NullVerifier>
    struct ContextOverride
    {
        std::string_view context_name;
        std::optional<std::vector<ParserExpectedError>> errors = std::nullopt;
        std::optional<OneOf<T>> verifier = std::nullopt;
        std::vector<SentinelKind> excluded_sentinels = {};
        std::vector<SentinelKind> accepted_sentinels = {};
        bool skip_after_depth_0 = false;
        bool skip_transform = false;
    };

    struct ContextOverrideAny
    {
        std::string_view context_name;
        std::optional<std::vector<ParserExpectedError>> errors = std::nullopt;
        std::optional<UniversalVerifier> verifier = std::nullopt;
        std::vector<SentinelKind> excluded_sentinels = {};
        std::vector<SentinelKind> accepted_sentinels = {};
        bool skip_after_depth_0 = false;
        bool skip_transform = false;

        template <typename T>
        ContextOverrideAny(const ContextOverride<T>& typed)
            : context_name(typed.context_name),
              errors(typed.errors),
              verifier(typed.verifier.has_value() ? std::make_optional(typed.verifier->value) : std::nullopt),
              excluded_sentinels(typed.excluded_sentinels),
              accepted_sentinels(typed.accepted_sentinels),
              skip_after_depth_0(typed.skip_after_depth_0),
              skip_transform(typed.skip_transform)
        {
        }

        ContextOverrideAny(std::string_view name,
                           std::optional<std::vector<ParserExpectedError>> errs = std::nullopt,
                           std::optional<UniversalVerifier> ver = std::nullopt,
                           std::vector<SentinelKind> excluded = {},
                           std::vector<SentinelKind> accepted = {},
                           bool skip_after_d0 = false,
                           bool skip_xform = false)
            : context_name(name), errors(std::move(errs)), verifier(std::move(ver)),
              excluded_sentinels(std::move(excluded)), accepted_sentinels(std::move(accepted)),
              skip_after_depth_0(skip_after_d0), skip_transform(skip_xform)
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
}
