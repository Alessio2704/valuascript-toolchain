#pragma once

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include "pool_kind.h"
#include "injectable_type.h"
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
        std::vector<PoolKind> excluded_pools = {};
    };

    template <typename Verifier>
    struct ConstructCase
    {
        std::string name;
        std::string code;
        Verifier verifier;
        std::vector<std::string_view> skip_contexts = {};
        std::vector<PoolKind> excluded_pools = {};
    };

    class ConstructRegistry
    {
    private:
        template <typename T, typename Fn>
        static bool search_construct_category(InjectableType type, const std::vector<RegistryEntry<T>>& entries, std::string_view test_name, Fn&& fn)
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
            if (search_construct_category(InjectableType::Import, imports(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Import, modified_imports(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Directive, directives(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Function, functions(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Function, modified_functions(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Extension, extensions(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Extension, modified_extensions(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Struct, structs(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Struct, modified_structs(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Enum, enums(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Enum, modified_enums(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::TypeAlias, aliases(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::TypeAlias, modified_aliases(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::StrongStatement, assignments(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::StrongStatement, modified_assignments(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::StrongStatement, reassignments(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::WeakStatement, returns(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::WeakStatement, modified_returns(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::StrongStatement, expr_stmts(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Expression, expressions(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::Modifier, modifiers(), test_name, fn)) return true;
            if (search_construct_category(InjectableType::TypeAnnotation, type_annotations(), test_name, fn)) return true;
            return false;
        }

        template <typename Verifier>
        static void add(const ConstructCase<Verifier>& spec)
        {
            add(spec.name, spec.code, spec.verifier, spec.skip_contexts, spec.excluded_pools);
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

        static void add(const std::string& n, const std::string& c, const ImportVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { imports().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const DirectiveVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { directives().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const FuncVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { functions().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const ExtVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { extensions().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const StructVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { structs().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const EnumVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { enums().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const AliasVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { aliases().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const AssignmentVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { assignments().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const ReassignmentVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { reassignments().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const ReturnVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { returns().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const ExprStmtVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { expr_stmts().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const ExprVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { expressions().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const ModifierVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { modifiers().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
        static void add(const std::string& n, const std::string& c, const TypeVerifier& v, const std::vector<std::string_view>& s = {}, const std::vector<PoolKind>& ep = {}) { type_annotations().push_back({.test_name = n, .code = c, .verifier = v, .skip_contexts = s, .excluded_pools = ep}); }
    };
}
