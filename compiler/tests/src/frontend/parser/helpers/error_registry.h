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

    template <typename T>
    struct ContextOverride
    {
        std::string_view context_name;
        std::optional<std::vector<ParserExpectedError>> errors = std::nullopt;
        std::optional<OneOf<T>> verifier = std::nullopt;
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
    };

    class ErrorRegistry
    {
    public:
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
                        const std::vector<ContextOverride<ImportVerifier>>& context_overrides = {})
        {
            imports().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<DirectiveVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<DirectiveVerifier>>& context_overrides = {})
        {
            directives().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<FuncVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<FuncVerifier>>& context_overrides = {})
        {
            functions().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ExtVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ExtVerifier>>& context_overrides = {})
        {
            extensions().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<StructVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<StructVerifier>>& context_overrides = {})
        {
            structs().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<EnumVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<EnumVerifier>>& context_overrides = {})
        {
            enums().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<AliasVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<AliasVerifier>>& context_overrides = {})
        {
            aliases().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<AssignmentVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<AssignmentVerifier>>& context_overrides = {})
        {
            assignments().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ReassignmentVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ReassignmentVerifier>>& context_overrides = {})
        {
            reassignments().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ReturnVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ReturnVerifier>>& context_overrides = {})
        {
            returns().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ExprStmtVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ExprStmtVerifier>>& context_overrides = {})
        {
            expr_stmts().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ExprVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ExprVerifier>>& context_overrides = {})
        {
            expressions().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<ModifierVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<ModifierVerifier>>& context_overrides = {})
        {
            modifiers().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ParserExpectedError>& errs,
                        const OneOf<TypeVerifier>& v, const std::vector<std::string_view>& skip_contexts = {},
                        const std::vector<ContextOverride<TypeVerifier>>& context_overrides = {})
        {
            type_annotations().push_back({n, c, errs, v, skip_contexts, context_overrides});
        }
    };
}
