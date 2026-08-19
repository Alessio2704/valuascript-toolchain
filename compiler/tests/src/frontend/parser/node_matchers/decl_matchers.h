#pragma once

#include "core.h"
#include "specs.h"
#include "expect_functions.h"

namespace valuascript::compiler::test
{
    struct FunctionDefMatcher
    {
        using node_type = FunctionDefinition;
        StringStorage name;
        std::vector<ModifierSpec> modifiers;
        std::vector<ParamSpec> params;
        std::vector<TypeVerifier> returns;
        std::vector<StmtVerifier> body;
        std::optional<StringStorage> docstring;

        void operator()(FunctionDefinition* f) const
        {
            ExpectFunctionDef(f, name.get(), modifiers, params, returns, body, docstring);
        }
    };

    inline FluentNodeMatcher<FunctionDefMatcher> IsFunctionDef(StringStorage name,
                                                              std::vector<ModifierSpec> modifiers = {},
                                                              std::vector<ParamSpec> params = {},
                                                              std::vector<TypeVerifier> returns = {},
                                                              std::vector<StmtVerifier> body = {},
                                                              std::optional<StringStorage> docstring = std::nullopt)
    {
        return FluentNodeMatcher<FunctionDefMatcher>{FunctionDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .params = std::move(params),
            .returns = std::move(returns), .body = std::move(body), .docstring = std::move(docstring)
        }};
    }

    struct StructDefMatcher
    {
        using node_type = StructDefinition;
        StringStorage name;
        std::vector<ModifierSpec> modifiers;
        std::vector<FieldSpec> fields;

        void operator()(StructDefinition* s) const
        {
            ExpectStructDef(s, name.get(), modifiers, fields);
        }
    };

    template <typename T>
    StructVerifier IsStructDef(StringStorage, std::initializer_list<T>) = delete;
    template <typename T>
    StructVerifier IsStructDef(StringStorage, std::vector<ModifierSpec>, std::initializer_list<T>) = delete;

    inline FluentNodeMatcher<StructDefMatcher> IsStructDef(StringStorage name, std::vector<ModifierSpec> modifiers = {},
                                                          std::vector<FieldSpec> fields = {})
    {
        return FluentNodeMatcher<StructDefMatcher>{StructDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .fields = std::move(fields)
        }};
    }

    template <typename... FieldSpecs>
        requires (sizeof...(FieldSpecs) > 0 && !(sizeof...(FieldSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<FieldSpecs...>>>, std::vector<FieldSpec>>))
    inline auto IsStructDef(StringStorage name, FieldSpecs&&... fields)
    {
        std::vector<FieldSpec> field_list = {std::forward<FieldSpecs>(fields)...};
        return FluentNodeMatcher<StructDefMatcher>{StructDefMatcher{
            .name = std::move(name), .modifiers = {}, .fields = std::move(field_list)
        }};
    }

    template <typename... FieldSpecs>
        requires (sizeof...(FieldSpecs) > 0 && !(sizeof...(FieldSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<FieldSpecs...>>>, std::vector<FieldSpec>>))
    inline auto IsStructDef(StringStorage name, std::vector<ModifierSpec> modifiers, FieldSpecs&&... fields)
    {
        std::vector<FieldSpec> field_list = {std::forward<FieldSpecs>(fields)...};
        return FluentNodeMatcher<StructDefMatcher>{StructDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .fields = std::move(field_list)
        }};
    }

    struct EnumDefMatcher
    {
        using node_type = EnumDefinition;
        StringStorage name;
        std::vector<ModifierSpec> modifiers;
        TypeVerifier type;
        std::vector<EnumCaseSpec> cases;

        void operator()(EnumDefinition* e) const
        {
            ExpectEnumDef(e, name.get(), modifiers, type, cases);
        }
    };

    template <typename T>
    EnumVerifier IsEnumDef(StringStorage, std::initializer_list<T>) = delete;
    template <typename T, typename U = TypeVerifier>
    EnumVerifier IsEnumDef(StringStorage, std::vector<ModifierSpec>, U&&, std::initializer_list<T>) = delete;

    template <TypeNodeMatcher T = TypeVerifier>
    inline FluentNodeMatcher<EnumDefMatcher> IsEnumDef(StringStorage name, std::vector<ModifierSpec> modifiers = {},
                                                      T&& type = nullptr,
                                                      std::vector<EnumCaseSpec> cases = {})
    {
        return FluentNodeMatcher<EnumDefMatcher>{EnumDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .type = std::forward<T>(type),
            .cases = std::move(cases)
        }};
    }

    template <typename... EnumCaseSpecs>
        requires (sizeof...(EnumCaseSpecs) > 0 && !(sizeof...(EnumCaseSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<EnumCaseSpecs...>>>, std::vector<EnumCaseSpec>>) &&
            !std::same_as<std::decay_t<std::tuple_element_t<0, std::tuple<EnumCaseSpecs...>>>, std::vector<ModifierSpec>>)
    inline auto IsEnumDef(StringStorage name, EnumCaseSpecs&&... cases)
    {
        std::vector<EnumCaseSpec> case_list = {std::forward<EnumCaseSpecs>(cases)...};
        return FluentNodeMatcher<EnumDefMatcher>{EnumDefMatcher{
            .name = std::move(name), .modifiers = {}, .type = nullptr, .cases = std::move(case_list)
        }};
    }

    template <TypeNodeMatcher T = TypeVerifier, typename... EnumCaseSpecs>
        requires (sizeof...(EnumCaseSpecs) > 0 && !(sizeof...(EnumCaseSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<EnumCaseSpecs...>>>, std::vector<EnumCaseSpec>>))
    inline auto IsEnumDef(StringStorage name, std::vector<ModifierSpec> modifiers, T&& type,
                          EnumCaseSpecs&&... cases)
    {
        std::vector<EnumCaseSpec> case_list = {std::forward<EnumCaseSpecs>(cases)...};
        return FluentNodeMatcher<EnumDefMatcher>{EnumDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .type = std::forward<T>(type),
            .cases = std::move(case_list)
        }};
    }

    struct TypeAliasMatcher
    {
        using node_type = TypeAliasDefinition;
        StringStorage name;
        std::vector<ModifierSpec> modifiers;
        TypeVerifier target;

        void operator()(TypeAliasDefinition* a) const
        {
            ExpectTypeAlias(a, name.get(), modifiers, target);
        }
    };

    inline FluentNodeMatcher<TypeAliasMatcher> IsTypeAlias(StringStorage name, std::vector<ModifierSpec> modifiers = {},
                                                          TypeVerifier target = nullptr)
    {
        return FluentNodeMatcher<TypeAliasMatcher>{TypeAliasMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .target = std::move(target)
        }};
    }

    struct ExtensionDefMatcher
    {
        using node_type = ExtensionDefinition;
        std::vector<ModifierSpec> modifiers;
        TypeVerifier target;
        ProgramSpec spec;

        void operator()(ExtensionDefinition* e) const
        {
            ExpectExtensionDef(e, modifiers, target, spec);
        }
    };

    inline FluentNodeMatcher<ExtensionDefMatcher> IsExtensionDef(std::vector<ModifierSpec> modifiers = {},
                                                                TypeVerifier target = nullptr,
                                                                ProgramSpec spec = {})
    {
        return FluentNodeMatcher<ExtensionDefMatcher>{ExtensionDefMatcher{
            .modifiers = std::move(modifiers), .target = std::move(target), .spec = std::move(spec)
        }};
    }

    struct ImportMatcher
    {
        using node_type = ImportStatement;
        StringStorage path;
        std::vector<ModifierSpec> modifiers;

        void operator()(ImportStatement* i) const
        {
            ExpectImport(i, modifiers, path.get());
        }
    };

    inline FluentNodeMatcher<ImportMatcher> IsImport(StringStorage path, std::vector<ModifierSpec> modifiers = {})
    {
        return FluentNodeMatcher<ImportMatcher>{ImportMatcher{.path = std::move(path), .modifiers = std::move(modifiers)}};
    }

    template <typename V = AnyMatcher>
    struct DirectiveMatcher
    {
        using node_type = Directive;
        StringStorage name;
        MatcherStorage<V> value;

        void operator()(Directive* d) const
        {
            if (auto dir = ExpectNode<Directive>(d))
            {
                EXPECT_EQ(dir->name, name.get()) << "Directive name mismatch.";
                value(dir->value.get());
            }
        }
    };

    template <typename V = AnyMatcher>
    inline auto IsDirective(StringStorage name, V&& value = {})
    {
        return FluentNodeMatcher<DirectiveMatcher<std::decay_t<V>>>{
            DirectiveMatcher<std::decay_t<V>>{std::move(name), std::forward<V>(value)}
        };
    }
}
