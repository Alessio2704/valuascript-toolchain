#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <optional>
#include "injectable_type.h"
#include "sentinel_kind.h"
#include "universal_verifier.h"
#include "node_matchers.h"
#include "context_override.h"

namespace valuascript::compiler::test
{
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
    private:
        template <typename T, typename Fn>
        static bool search_category(InjectableType type, const std::vector<ErrorRegistryEntry<T>>& entries, std::string_view test_name, Fn&& fn)
        {
            for (const auto& entry : entries)
            {
                if (entry.test_name == test_name)
                {
                    fn(type, entry);
                    return true;
                }
            }
            return false;
        }

    public:
        template <typename Fn>
        static bool find(std::string_view test_name, Fn&& fn)
        {
            if (search_category(InjectableType::Import, imports(), test_name, fn)) return true;
            if (search_category(InjectableType::Directive, directives(), test_name, fn)) return true;
            if (search_category(InjectableType::Function, functions(), test_name, fn)) return true;
            if (search_category(InjectableType::Extension, extensions(), test_name, fn)) return true;
            if (search_category(InjectableType::Struct, structs(), test_name, fn)) return true;
            if (search_category(InjectableType::Enum, enums(), test_name, fn)) return true;
            if (search_category(InjectableType::TypeAlias, aliases(), test_name, fn)) return true;
            if (search_category(InjectableType::StrongStatement, assignments(), test_name, fn)) return true;
            if (search_category(InjectableType::StrongStatement, reassignments(), test_name, fn)) return true;
            if (search_category(InjectableType::WeakStatement, returns(), test_name, fn)) return true;
            if (search_category(InjectableType::StrongStatement, expr_stmts(), test_name, fn)) return true;
            if (search_category(InjectableType::Expression, expressions(), test_name, fn)) return true;
            if (search_category(InjectableType::Modifier, modifiers(), test_name, fn)) return true;
            if (search_category(InjectableType::TypeAnnotation, type_annotations(), test_name, fn)) return true;
            return false;
        }

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
            imports().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<DirectiveVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<DirectiveVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            directives().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<FuncVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<FuncVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            functions().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ExtVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ExtVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            extensions().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<StructVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<StructVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            structs().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<EnumVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<EnumVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            enums().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<AliasVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<AliasVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            aliases().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<AssignmentVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<AssignmentVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            assignments().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ReassignmentVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ReassignmentVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            reassignments().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ReturnVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ReturnVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            returns().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ExprStmtVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ExprStmtVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            expr_stmts().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ExprVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ExprVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            expressions().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ModifierVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ModifierVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            modifiers().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<TypeVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<TypeVerifier>>& context_overrides = {},
                        const std::vector<SentinelKind>& excluded_sentinels = {},
                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            type_annotations().push_back({.test_name = n, .code = c, .errors = errs, .verifier = v, .skip_contexts = skip_contexts, .context_overrides = context_overrides, .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels});
        }
    };

    template <typename T>
    inline void reg(const RecoveryCase<T>& spec)
    {
        ErrorRegistry::add(spec);
    }
}
