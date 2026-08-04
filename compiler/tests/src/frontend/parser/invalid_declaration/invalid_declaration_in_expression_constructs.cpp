#include "invalid_declaration_in_expression_constructs.h"
#include "frontend/parser/helpers/construct_registry.h"
#include "frontend/parser/helpers/error_registry.h"
#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/helpers/deterministic_sampler.h"
#include <algorithm>

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
                std::vector<const RegistryEntry<T>*> valid_clean;
                for (const auto& entry : clean_registry)
                {
                    bool skipped = std::find(entry.skip_contexts.begin(), entry.skip_contexts.end(), context_name) != entry.skip_contexts.end();
                    if (!skipped)
                    {
                        valid_clean.push_back(&entry);
                    }
                }
                if (!valid_clean.empty())
                {
                    const auto* entry_ptr = DeterministicSampler::sample_element(valid_clean, context_name, category_name, "clean");
                    list.push_back({
                        .name = "Clean_" + category_name + "_" + entry_ptr->test_name,
                        .code = entry_ptr->code,
                        .verifier = UniversalVerifier(entry_ptr->verifier),
                        .type = type,
                        .is_broken = false,
                        .suppressed_errors = {},
                        .skip_contexts = entry_ptr->skip_contexts
                    });
                }
            }

            if (!broken_registry.empty())
            {
                std::vector<const ErrorRegistryEntry<T>*> valid_broken;
                for (const auto& entry : broken_registry)
                {
                    bool skipped = std::find(entry.skip_contexts.begin(), entry.skip_contexts.end(), context_name) != entry.skip_contexts.end();
                    if (skipped) continue;

                    if (!is_valid_declaration_keyword(type, entry.code)) continue;

                    std::string_view name_view = entry.test_name;
                    if (name_view.find("EnumMissingName") != std::string_view::npos ||
                        name_view.find("MissingTypeName") != std::string_view::npos ||
                        name_view.find("MissingStructName") != std::string_view::npos ||
                        name_view.find("NoNameFunc") != std::string_view::npos ||
                        name_view.find("MissingAliasName") != std::string_view::npos ||
                        name_view.find("MissingVariableName") != std::string_view::npos ||
                        name_view.find("MultiReassignmentNotSupported") != std::string_view::npos ||
                        name_view.find("MissingImportStringPath") != std::string_view::npos ||
                        name_view.find("InvalidStandaloneStatement") != std::string_view::npos ||
                        name_view.find("InvalidCharacter") != std::string_view::npos ||
                        name_view.find("InvalidLeftSide") != std::string_view::npos)
                    {
                        continue;
                    }

                    valid_broken.push_back(&entry);
                }

                if (!valid_broken.empty())
                {
                    const auto* entry_ptr = DeterministicSampler::sample_element(valid_broken, context_name, category_name, "broken");
                    std::vector<ValuascriptErrorCode> err_codes;
                    err_codes.reserve(entry_ptr->errors.size());
                    for (const auto& e : entry_ptr->errors)
                    {
                        err_codes.push_back(e.code);
                    }
                    list.push_back({
                        .name = "Broken_" + category_name + "_" + entry_ptr->test_name,
                        .code = entry_ptr->code,
                        .verifier = entry_ptr->verifier.value,
                        .type = type,
                        .is_broken = true,
                        .suppressed_errors = std::move(err_codes),
                        .skip_contexts = entry_ptr->skip_contexts
                    });
                }
            }
        }
    }

    std::vector<InvalidDeclarationConstructCase> InvalidDeclarationInExpressionConstructRegistry::cases_for_context(const Context& ctx)
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
        add_stratified_cases_for_context(list, ctx.name, "Assignment", ConstructRegistry::assignments(), ErrorRegistry::assignments(), InjectableType::StrongStatement);
        add_stratified_cases_for_context(list, ctx.name, "Reassignment", ConstructRegistry::reassignments(), ErrorRegistry::reassignments(), InjectableType::WeakStatement);

        return list;
    }

    const std::vector<InvalidDeclarationConstructCase>& InvalidDeclarationInExpressionConstructRegistry::cases()
    {
        static const std::vector<InvalidDeclarationConstructCase> fallback = cases_for_context(Context{ .name = "default" });
        return fallback;
    }

    bool should_test_construct_in_context_expr(const Context& ctx, const InvalidDeclarationConstructCase& construct)
    {
        bool is_skipped_for_ctx = (std::find(construct.skip_contexts.begin(), construct.skip_contexts.end(),
                                             ctx.name) != construct.skip_contexts.end());
        if (is_skipped_for_ctx)
        {
            return false;
        }

        if (construct.is_broken && !is_valid_declaration_keyword(construct.type, construct.code))
        {
            return false;
        }

        if (construct.is_broken && has_unclosed_brace(construct.code))
        {
            return false;
        }

        if (construct.is_broken)
        {
            if (construct.name.find("EnumMissingName") != std::string::npos ||
                construct.name.find("MissingTypeName") != std::string::npos ||
                construct.name.find("MissingAliasName") != std::string::npos ||
                construct.name.find("MissingTargetTypeAnnotation") != std::string::npos ||
                construct.name.find("MissingVariableName") != std::string::npos ||
                construct.name.find("MultiReassignmentNotSupported") != std::string::npos ||
                construct.name.find("MissingImportStringPath") != std::string::npos)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<InvalidDeclarationInExpressionTestCase> GenerateInvalidDeclarationInExpressionTestCases()
    {
        std::vector<InvalidDeclarationInExpressionTestCase> test_cases;

        for (const auto& ctx : get_expression_test_contexts())
        {
            auto constructs = InvalidDeclarationInExpressionConstructRegistry::cases_for_context(ctx);
            for (const auto& construct : constructs)
            {
                if (should_test_construct_in_context_expr(ctx, construct))
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
