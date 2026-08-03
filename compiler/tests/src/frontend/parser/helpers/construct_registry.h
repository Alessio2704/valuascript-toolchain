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
        std::vector<std::string_view> skip_contexts = {};
    };

    template <typename Verifier>
    struct ConstructCase
    {
        std::string name;
        std::string code;
        Verifier verifier;
        std::vector<std::string_view> skip_contexts = {};
    };

    class ConstructRegistry
    {
    public:
        template <typename Verifier>
        static void add(const ConstructCase<Verifier>& spec)
        {
            add(spec.name, spec.code, spec.verifier, spec.skip_contexts);
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

        static void add(const std::string& n, const std::string& c, const ImportVerifier& v, const std::vector<std::string_view>& s = {}) { imports().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const DirectiveVerifier& v, const std::vector<std::string_view>& s = {}) { directives().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const FuncVerifier& v, const std::vector<std::string_view>& s = {}) { functions().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const ExtVerifier& v, const std::vector<std::string_view>& s = {}) { extensions().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const StructVerifier& v, const std::vector<std::string_view>& s = {}) { structs().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const EnumVerifier& v, const std::vector<std::string_view>& s = {}) { enums().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const AliasVerifier& v, const std::vector<std::string_view>& s = {}) { aliases().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const AssignmentVerifier& v, const std::vector<std::string_view>& s = {}) { assignments().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const ReassignmentVerifier& v, const std::vector<std::string_view>& s = {}) { reassignments().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const ReturnVerifier& v, const std::vector<std::string_view>& s = {}) { returns().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const ExprStmtVerifier& v, const std::vector<std::string_view>& s = {}) { expr_stmts().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const ExprVerifier& v, const std::vector<std::string_view>& s = {}) { expressions().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const ModifierVerifier& v, const std::vector<std::string_view>& s = {}) { modifiers().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
        static void add(const std::string& n, const std::string& c, const TypeVerifier& v, const std::vector<std::string_view>& s = {}) { type_annotations().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s}); }
    };
}
