#include "invalid_declaration_in_block_constructs.h"
#include "frontend/parser/helpers/construct_registry.h"
#include "frontend/parser/helpers/error_registry.h"
#include "frontend/parser/helpers/context_registry.h"
#include <algorithm>

namespace valuascript::compiler::test
{
    namespace
    {
        template <typename T>
        void add_clean_registry_cases(std::vector<InvalidDeclarationConstructCase>& list,
                                       const std::string& category_name,
                                       const std::vector<RegistryEntry<T>>& registry_entries,
                                       InjectableType type)
        {
            for (const auto& entry : registry_entries)
            {
                list.push_back({
                    .name = "Clean_" + category_name + "_" + entry.test_name,
                    .code = entry.code,
                    .verifier = UniversalVerifier(entry.verifier),
                    .type = type,
                    .is_broken = false,
                    .suppressed_errors = {},
                    .skip_contexts = entry.skip_contexts
                });
            }
        }

        template <typename T>
        void add_broken_registry_cases(std::vector<InvalidDeclarationConstructCase>& list,
                                       const std::string& category_name,
                                       const std::vector<ErrorRegistryEntry<T>>& registry_entries,
                                       InjectableType type)
        {
            for (const auto& entry : registry_entries)
            {
                std::vector<ValuascriptErrorCode> err_codes;
                err_codes.reserve(entry.errors.size());
                for (const auto& e : entry.errors)
                {
                    err_codes.push_back(e.code);
                }

                list.push_back({
                    .name = "Broken_" + category_name + "_" + entry.test_name,
                    .code = entry.code,
                    .verifier = entry.verifier.value,
                    .type = type,
                    .is_broken = true,
                    .suppressed_errors = std::move(err_codes),
                    .skip_contexts = entry.skip_contexts
                });
            }
        }
    }

    const std::vector<InvalidDeclarationConstructCase>& InvalidDeclarationConstructRegistry::cases()
    {
        static const std::vector<InvalidDeclarationConstructCase> registry = []()
        {
            std::vector<InvalidDeclarationConstructCase> list;

            add_clean_registry_cases(list, "FuncDef", ConstructRegistry::functions(), InjectableType::Function);
            add_clean_registry_cases(list, "StructDef", ConstructRegistry::structs(), InjectableType::Struct);
            add_clean_registry_cases(list, "EnumDef", ConstructRegistry::enums(), InjectableType::Enum);
            add_clean_registry_cases(list, "ExtensionDef", ConstructRegistry::extensions(), InjectableType::Extension);
            add_clean_registry_cases(list, "TypeAliasDef", ConstructRegistry::aliases(), InjectableType::TypeAlias);
            add_clean_registry_cases(list, "ImportStmt", ConstructRegistry::imports(), InjectableType::Import);
            add_clean_registry_cases(list, "DirectiveStmt", ConstructRegistry::directives(), InjectableType::Directive);
            add_clean_registry_cases(list, "ReturnStmt", ConstructRegistry::returns(), InjectableType::WeakStatement);

            add_broken_registry_cases(list, "FuncDef", ErrorRegistry::functions(), InjectableType::Function);
            add_broken_registry_cases(list, "StructDef", ErrorRegistry::structs(), InjectableType::Struct);
            add_broken_registry_cases(list, "EnumDef", ErrorRegistry::enums(), InjectableType::Enum);
            add_broken_registry_cases(list, "ExtensionDef", ErrorRegistry::extensions(), InjectableType::Extension);
            add_broken_registry_cases(list, "TypeAliasDef", ErrorRegistry::aliases(), InjectableType::TypeAlias);
            add_broken_registry_cases(list, "ImportStmt", ErrorRegistry::imports(), InjectableType::Import);
            add_broken_registry_cases(list, "DirectiveStmt", ErrorRegistry::directives(), InjectableType::Directive);
            add_broken_registry_cases(list, "ReturnStmt", ErrorRegistry::returns(), InjectableType::WeakStatement);

            return list;
        }();
        return registry;
    }

    std::vector<InvalidDeclarationInBlockTestCase> GenerateInvalidDeclarationInBlockTestCases()
    {
        std::vector<InvalidDeclarationInBlockTestCase> test_cases;

        std::vector<Context> container_contexts;
        const auto& block_ctxs = ContextRegistry::get_block_contexts();
        container_contexts.insert(container_contexts.end(), block_ctxs.begin(), block_ctxs.end());

        const auto& top_level_ctxs = ContextRegistry::get_top_level_contexts();
        container_contexts.insert(container_contexts.end(), top_level_ctxs.begin(), top_level_ctxs.end());

        const auto& constructs = InvalidDeclarationConstructRegistry::cases();

        for (const auto& ctx : container_contexts)
        {
            for (const auto& construct : constructs)
            {
                if (should_test_construct_in_context(ctx, construct))
                {
                    test_cases.push_back({
                        .test_name = std::string(ctx.name) + "_" + construct.name,
                        .context = ctx,
                        .construct_case = construct
                    });
                }
            }
        }

        return test_cases;
    }
}
