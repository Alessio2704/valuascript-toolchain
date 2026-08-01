#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <optional>

#include "context_infrastructure.h"
#include "node_matchers.h"
#include "core/valuascript_exception.h"

namespace valuascript::compiler::test
{
    struct ParserExpectedError
    {
        ValuascriptErrorCode code;
        size_t line_start;
        size_t column_start;
        size_t line_end;
        size_t column_end;
        bool skip_span_check;

        ParserExpectedError(ValuascriptErrorCode c,
                            size_t ls = 0,
                            size_t cs = 0,
                            size_t le = 0,
                            size_t ce = 0,
                            bool skip_check = false)
            : code(c),
              line_start(ls),
              column_start(cs),
              line_end(le),
              column_end(ce),
              skip_span_check(skip_check)
        {
        }
    };

    template <typename T = NullVerifier>
    struct ContextOverride
    {
        std::string_view context_name;
        std::optional<std::vector<ParserExpectedError>> errors = std::nullopt;
        std::optional<OneOf<T>> verifier = std::nullopt;
        std::vector<SentinelKind> excluded_sentinels = {};
        std::vector<SentinelKind> accepted_sentinels = {};
    };

    template <typename T>
    struct ErrorRegistryEntry
    {
        std::string test_name;
        std::string code;
        std::vector<ParserExpectedError> errors;
        OneOf<T> verifier;
        std::vector<std::string_view> skip_contexts;
        std::vector<ContextOverride<T>> context_overrides = {};
        std::vector<SentinelKind> excluded_sentinels = {};
        std::vector<SentinelKind> accepted_sentinels = {};
    };

    template <typename T>
    struct RecoveryCase
    {
        std::string name;
        std::string code;
        std::vector<ParserExpectedError> errors;
        OneOf<T> verifier;
        std::vector<std::string_view> skip_contexts = {};
        std::vector<ContextOverride<T>> context_overrides = {};
        std::vector<SentinelKind> excluded_sentinels = {};
        std::vector<SentinelKind> accepted_sentinels = {};
    };

    using ImportCase = RecoveryCase<ImportVerifier>;
    using DirectiveCase = RecoveryCase<DirectiveVerifier>;
    using FuncCase = RecoveryCase<FuncVerifier>;
    using ExtCase = RecoveryCase<ExtVerifier>;
    using StructCase = RecoveryCase<StructVerifier>;
    using EnumCase = RecoveryCase<EnumVerifier>;
    using AliasCase = RecoveryCase<AliasVerifier>;
    using AssignmentCase = RecoveryCase<AssignmentVerifier>;
    using ReassignmentCase = RecoveryCase<ReassignmentVerifier>;
    using ReturnCase = RecoveryCase<ReturnVerifier>;
    using ExprStmtCase = RecoveryCase<ExprStmtVerifier>;
    using ExprCase = RecoveryCase<ExprVerifier>;
    using ModifierCase = RecoveryCase<ModifierVerifier>;
    using TypeCase = RecoveryCase<TypeVerifier>;

    class ErrorRegistry
    {
    public:
        template <typename T>
        static void add(const RecoveryCase<T>& spec)
        {
            add(spec.name, spec.code, spec.errors, spec.verifier,
                spec.skip_contexts, spec.context_overrides,
                spec.excluded_sentinels, spec.accepted_sentinels);
        }
        static std::vector<ErrorRegistryEntry<ImportVerifier>>& imports();
        static std::vector<ErrorRegistryEntry<DirectiveVerifier>>& directives();
        static std::vector<ErrorRegistryEntry<FuncVerifier>>& functions();
        static std::vector<ErrorRegistryEntry<ExtVerifier>>& extensions();
        static std::vector<ErrorRegistryEntry<StructVerifier>>& structs();
        static std::vector<ErrorRegistryEntry<EnumVerifier>>& enums();
        static std::vector<ErrorRegistryEntry<AliasVerifier>>& aliases();
        static std::vector<ErrorRegistryEntry<AssignmentVerifier>>& assignments();
        static std::vector<ErrorRegistryEntry<ReassignmentVerifier>>& reassignments();
        static std::vector<ErrorRegistryEntry<ReturnVerifier>>& returns();
        static std::vector<ErrorRegistryEntry<ExprStmtVerifier>>& expr_stmts();
        static std::vector<ErrorRegistryEntry<ExprVerifier>>& expressions();
        static std::vector<ErrorRegistryEntry<ModifierVerifier>>& modifiers();
        static std::vector<ErrorRegistryEntry<TypeVerifier>>& type_annotations();

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ImportVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ImportVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            imports().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<DirectiveVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<DirectiveVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            directives().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<FuncVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<FuncVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            functions().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ExtVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ExtVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            extensions().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<StructVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<StructVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            structs().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<EnumVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<EnumVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            enums().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<AliasVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<AliasVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            aliases().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<AssignmentVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<AssignmentVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            assignments().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ReassignmentVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ReassignmentVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            reassignments().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ReturnVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ReturnVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            returns().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ExprStmtVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ExprStmtVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            expr_stmts().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ExprVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ExprVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            expressions().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ModifierVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ModifierVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            modifiers().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<TypeVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<TypeVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            type_annotations().push_back({n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels});
        }
    };

    template <typename T>
    inline void reg(const RecoveryCase<T>& spec)
    {
        ErrorRegistry::add(spec);
    }
}
