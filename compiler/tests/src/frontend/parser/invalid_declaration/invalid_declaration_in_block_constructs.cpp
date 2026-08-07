#include "invalid_declaration_in_block_constructs.h"
#include "frontend/parser/helpers/construct_registry.h"
#include "frontend/parser/helpers/error_registry.h"
#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/helpers/deterministic_sampler.h"

namespace valuascript::compiler::test
{
    namespace
    {
        template <typename T>
        void add_stratified_cases_for_context(std::vector<InvalidDeclarationConstructCase>& list,
                                              std::string_view context_name,
                                              const std::string& category_name,
                                              const std::vector<RegistryEntry<T>>& clean_registry,
                                              const std::vector<ErrorRegistryEntry<T>>& broken_registry,
                                              InjectableType type)
        {
            if (!clean_registry.empty())
            {
                const auto& entry = DeterministicSampler::sample_element(clean_registry, context_name, category_name, "clean");
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
            if (!broken_registry.empty())
            {
                const auto& entry = DeterministicSampler::sample_element(broken_registry, context_name, category_name, "broken");
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

    std::vector<InvalidDeclarationConstructCase> InvalidDeclarationConstructRegistry::cases_for_context(const Context& ctx)
    {
        std::vector<InvalidDeclarationConstructCase> list;

        add_stratified_cases_for_context(list, ctx.name, "FuncDef", ConstructRegistry::functions(), ErrorRegistry::functions(), InjectableType::Function);
        add_stratified_cases_for_context(list, ctx.name, "StructDef", ConstructRegistry::structs(), ErrorRegistry::structs(), InjectableType::Struct);
        add_stratified_cases_for_context(list, ctx.name, "EnumDef", ConstructRegistry::enums(), ErrorRegistry::enums(), InjectableType::Enum);
        add_stratified_cases_for_context(list, ctx.name, "ExtensionDef", ConstructRegistry::extensions(), ErrorRegistry::extensions(), InjectableType::Extension);
        add_stratified_cases_for_context(list, ctx.name, "TypeAliasDef", ConstructRegistry::aliases(), ErrorRegistry::aliases(), InjectableType::TypeAlias);
        add_stratified_cases_for_context(list, ctx.name, "ImportStmt", ConstructRegistry::imports(), ErrorRegistry::imports(), InjectableType::Import);
        add_stratified_cases_for_context(list, ctx.name, "DirectiveStmt", ConstructRegistry::directives(), ErrorRegistry::directives(), InjectableType::Directive);
        add_stratified_cases_for_context(list, ctx.name, "ReturnStmt", ConstructRegistry::returns(), ErrorRegistry::returns(), InjectableType::WeakStatement);

        return list;
    }

    const std::vector<InvalidDeclarationConstructCase>& InvalidDeclarationConstructRegistry::cases()
    {
        static const std::vector<InvalidDeclarationConstructCase> fallback = cases_for_context(Context{ .name = "default" });
        return fallback;
    }

    std::vector<InvalidDeclarationInBlockTestCase> GenerateInvalidDeclarationInBlockTestCases()
    {
        std::vector<InvalidDeclarationInBlockTestCase> test_cases;

        std::vector<Context> container_contexts;
        const auto& block_ctxs = ContextRegistry::get_block_contexts();
        container_contexts.insert(container_contexts.end(), block_ctxs.begin(), block_ctxs.end());

        const auto& top_level_ctxs = ContextRegistry::get_top_level_contexts();
        container_contexts.insert(container_contexts.end(), top_level_ctxs.begin(), top_level_ctxs.end());

        for (const auto& ctx : container_contexts)
        {
            auto constructs = InvalidDeclarationConstructRegistry::cases_for_context(ctx);
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
