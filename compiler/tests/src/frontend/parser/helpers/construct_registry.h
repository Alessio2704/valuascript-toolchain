#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    template <typename Verifier>
    struct RegistryEntry
    {
        std::string test_name;
        std::string code;
        Verifier verifier;
    };

    template <typename Verifier>
    struct ConstructCase
    {
        std::string name;
        std::string code;
        Verifier verifier;
    };

    class ConstructRegistry
    {
    public:
        template <typename Verifier>
        static void add(const ConstructCase<Verifier>& spec)
        {
            add(spec.name, spec.code, spec.verifier);
        }

        static std::vector<RegistryEntry<ImportVerifier>>& imports();
        static std::vector<RegistryEntry<ImportVerifier>>& modified_imports();
        static std::vector<RegistryEntry<DirectiveVerifier>>& directives();
        static std::vector<RegistryEntry<FuncVerifier>>& functions();
        static std::vector<RegistryEntry<FuncVerifier>>& modified_functions();
        static std::vector<RegistryEntry<ExtVerifier>>& extensions();
        static std::vector<RegistryEntry<ExtVerifier>>& modified_extensions();
        static std::vector<RegistryEntry<StructVerifier>>& structs();
        static std::vector<RegistryEntry<StructVerifier>>& modified_structs();
        static std::vector<RegistryEntry<EnumVerifier>>& enums();
        static std::vector<RegistryEntry<EnumVerifier>>& modified_enums();
        static std::vector<RegistryEntry<AliasVerifier>>& aliases();
        static std::vector<RegistryEntry<AliasVerifier>>& modified_aliases();
        static std::vector<RegistryEntry<AssignmentVerifier>>& assignments();
        static std::vector<RegistryEntry<AssignmentVerifier>>& modified_assignments();
        static std::vector<RegistryEntry<ReassignmentVerifier>>& reassignments();
        static std::vector<RegistryEntry<ReturnVerifier>>& returns();
        static std::vector<RegistryEntry<ReturnVerifier>>& modified_returns();
        static std::vector<RegistryEntry<ExprStmtVerifier>>& expr_stmts();
        static std::vector<RegistryEntry<ExprVerifier>>& expressions();
        static std::vector<RegistryEntry<ModifierVerifier>>& modifiers();
        static std::vector<RegistryEntry<TypeVerifier>>& type_annotations();

        static void add(const std::string& n, const std::string& c, const ImportVerifier& v) { imports().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const DirectiveVerifier& v) { directives().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const FuncVerifier& v) { functions().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const ExtVerifier& v) { extensions().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const StructVerifier& v) { structs().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const EnumVerifier& v) { enums().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const AliasVerifier& v) { aliases().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const AssignmentVerifier& v) { assignments().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const ReassignmentVerifier& v) { reassignments().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const ReturnVerifier& v) { returns().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const ExprStmtVerifier& v) { expr_stmts().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const ExprVerifier& v) { expressions().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const ModifierVerifier& v) { modifiers().emplace_back(n, c, v); }
        static void add(const std::string& n, const std::string& c, const TypeVerifier& v) { type_annotations().emplace_back(n, c, v); }
    };
}
