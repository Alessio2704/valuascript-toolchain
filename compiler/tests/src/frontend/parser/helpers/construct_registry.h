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

    class ConstructRegistry
    {
    public:
        static std::vector<RegistryEntry<ImportVerifier>>& imports();
        static std::vector<RegistryEntry<DirectiveVerifier>>& directives();
        static std::vector<RegistryEntry<FuncVerifier>>& functions();
        static std::vector<RegistryEntry<StructVerifier>>& structs();
        static std::vector<RegistryEntry<EnumVerifier>>& enums();
        static std::vector<RegistryEntry<AliasVerifier>>& aliases();
        static std::vector<RegistryEntry<AssignmentVerifier>>& assignments();
        static std::vector<RegistryEntry<ReassignmentVerifier>>& reassignments();
        static std::vector<RegistryEntry<ReturnVerifier>>& returns();
        static std::vector<RegistryEntry<ExprStmtVerifier>>& expr_stmts();
        static std::vector<RegistryEntry<ExprVerifier>>& expressions();
        static std::vector<RegistryEntry<ModifierVerifier>>& modifiers();
        static std::vector<RegistryEntry<TypeVerifier>>& type_annotations();

        static void add(std::string n, std::string c, ImportVerifier v) { imports().push_back({n, c, v}); }
        static void add(std::string n, std::string c, DirectiveVerifier v) { directives().push_back({n, c, v}); }
        static void add(std::string n, std::string c, FuncVerifier v) { functions().push_back({n, c, v}); }
        static void add(std::string n, std::string c, StructVerifier v) { structs().push_back({n, c, v}); }
        static void add(std::string n, std::string c, EnumVerifier v) { enums().push_back({n, c, v}); }
        static void add(std::string n, std::string c, AliasVerifier v) { aliases().push_back({n, c, v}); }
        static void add(std::string n, std::string c, AssignmentVerifier v) { assignments().push_back({n, c, v}); }
        static void add(std::string n, std::string c, ReassignmentVerifier v) { reassignments().push_back({n, c, v}); }
        static void add(std::string n, std::string c, ReturnVerifier v) { returns().push_back({n, c, v}); }
        static void add(std::string n, std::string c, ExprStmtVerifier v) { expr_stmts().push_back({n, c, v}); }
        static void add(std::string n, std::string c, ExprVerifier v) { expressions().push_back({n, c, v}); }
        static void add(std::string n, std::string c, ModifierVerifier v) { modifiers().push_back({n, c, v}); }
        static void add(std::string n, std::string c, TypeVerifier v) { type_annotations().push_back({n, c, v}); }
    };
}
