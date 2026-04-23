#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"
#include "core/valuascript_exception.h"

namespace valuascript::compiler::test
{
    struct ExpectedError
    {
        ValuascriptErrorCode code;
        size_t line_start;
        size_t column_start;
        size_t line_end;
        size_t column_end;

        ExpectedError(ValuascriptErrorCode code,
                      size_t line_start,
                      size_t column_start,
                      size_t line_end = 0,
                      size_t column_end = 0)
            : code(code),
              line_start(line_start),
              column_start(column_start),
              line_end(line_end),
              column_end(column_end)
        {
        }
    };

    template <typename Verifier>
    struct ErrorRegistryEntry
    {
        std::string test_name;
        std::string code;
        std::vector<ExpectedError> errors;
        Verifier verifier;
    };

    class ErrorRegistry
    {
    public:
        static std::vector<ErrorRegistryEntry<ImportVerifier>>& imports();
        static std::vector<ErrorRegistryEntry<DirectiveVerifier>>& directives();
        static std::vector<ErrorRegistryEntry<FuncVerifier>>& functions();
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

        static void add(const std::string&, const std::string&, const std::vector<ExpectedError>&,
                        const NullVerifier&)
        {
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const ImportVerifier& v)
        {
            imports().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const DirectiveVerifier& v)
        {
            directives().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const FuncVerifier& v)
        {
            functions().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const StructVerifier& v)
        {
            structs().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const EnumVerifier& v)
        {
            enums().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const AliasVerifier& v)
        {
            aliases().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const AssignmentVerifier& v)
        {
            assignments().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const ReassignmentVerifier& v)
        {
            reassignments().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const ReturnVerifier& v)
        {
            returns().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const ExprStmtVerifier& v)
        {
            expr_stmts().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const ExprVerifier& v)
        {
            expressions().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const ModifierVerifier& v)
        {
            modifiers().emplace_back(n, c, errs, v);
        }

        static void add(const std::string& n, const std::string& c, const std::vector<ExpectedError>& errs,
                        const TypeVerifier& v)
        {
            type_annotations().emplace_back(n, c, errs, v);
        }
    };
}
