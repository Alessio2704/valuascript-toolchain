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

    inline FuncVerifier IsFunctionDef(StringStorage name,
                                      std::vector<ModifierSpec> modifiers = {},
                                      std::vector<ParamSpec> params = {},
                                      std::vector<TypeVerifier> returns = {},
                                      std::vector<StmtVerifier> body = {},
                                      std::optional<StringStorage> docstring = std::nullopt)
    {
        return FuncVerifier(FunctionDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .params = std::move(params),
            .returns = std::move(returns), .body = std::move(body), .docstring = std::move(docstring)
        });
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

    inline StructVerifier IsStructDef(StringStorage name, std::vector<ModifierSpec> modifiers = {},
                                      std::vector<FieldSpec> fields = {})
    {
        return StructVerifier(StructDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .fields = std::move(fields)
        });
    }

    template <typename... FieldSpecs>
        requires (sizeof...(FieldSpecs) > 0 && !(sizeof...(FieldSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<FieldSpecs...>>>, std::vector<FieldSpec>>))
    inline StructVerifier IsStructDef(StringStorage name, FieldSpecs&&... fields)
    {
        std::vector<FieldSpec> field_list = {std::forward<FieldSpecs>(fields)...};
        return StructVerifier(StructDefMatcher{
            .name = std::move(name), .modifiers = {}, .fields = std::move(field_list)
        });
    }

    template <typename... FieldSpecs>
        requires (sizeof...(FieldSpecs) > 0 && !(sizeof...(FieldSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<FieldSpecs...>>>, std::vector<FieldSpec>>))
    inline StructVerifier IsStructDef(StringStorage name, std::vector<ModifierSpec> modifiers, FieldSpecs&&... fields)
    {
        std::vector<FieldSpec> field_list = {std::forward<FieldSpecs>(fields)...};
        return StructVerifier(StructDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .fields = std::move(field_list)
        });
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
    template <typename T>
    EnumVerifier IsEnumDef(StringStorage, std::vector<ModifierSpec>, TypeVerifier, std::initializer_list<T>) = delete;

    inline EnumVerifier IsEnumDef(StringStorage name, std::vector<ModifierSpec> modifiers = {},
                                  TypeVerifier type = nullptr,
                                  std::vector<EnumCaseSpec> cases = {})
    {
        return EnumVerifier(EnumDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .type = std::move(type),
            .cases = std::move(cases)
        });
    }

    template <typename... EnumCaseSpecs>
        requires (sizeof...(EnumCaseSpecs) > 0 && !(sizeof...(EnumCaseSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<EnumCaseSpecs...>>>, std::vector<EnumCaseSpec>>))
    inline EnumVerifier IsEnumDef(StringStorage name, EnumCaseSpecs&&... cases)
    {
        std::vector<EnumCaseSpec> case_list = {std::forward<EnumCaseSpecs>(cases)...};
        return EnumVerifier(EnumDefMatcher{
            .name = std::move(name), .modifiers = {}, .type = nullptr, .cases = std::move(case_list)
        });
    }

    template <typename... EnumCaseSpecs>
        requires (sizeof...(EnumCaseSpecs) > 0 && !(sizeof...(EnumCaseSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<EnumCaseSpecs...>>>, std::vector<EnumCaseSpec>>))
    inline EnumVerifier IsEnumDef(StringStorage name, std::vector<ModifierSpec> modifiers, TypeVerifier type,
                                  EnumCaseSpecs&&... cases)
    {
        std::vector<EnumCaseSpec> case_list = {std::forward<EnumCaseSpecs>(cases)...};
        return EnumVerifier(EnumDefMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .type = std::move(type),
            .cases = std::move(case_list)
        });
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

    inline AliasVerifier IsTypeAlias(StringStorage name, std::vector<ModifierSpec> modifiers = {},
                                     TypeVerifier target = nullptr)
    {
        return AliasVerifier(TypeAliasMatcher{
            .name = std::move(name), .modifiers = std::move(modifiers), .target = std::move(target)
        });
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

    inline ExtVerifier IsExtensionDef(std::vector<ModifierSpec> modifiers = {},
                                      TypeVerifier target = nullptr,
                                      ProgramSpec spec = {})
    {
        return ExtVerifier(ExtensionDefMatcher{
            .modifiers = std::move(modifiers), .target = std::move(target), .spec = std::move(spec)
        });
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

    inline ImportVerifier IsImport(StringStorage path, std::vector<ModifierSpec> modifiers = {})
    {
        return ImportVerifier(ImportMatcher{.path = std::move(path), .modifiers = std::move(modifiers)});
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
    inline DirectiveVerifier IsDirective(StringStorage name, V&& value = {})
    {
        return DirectiveVerifier(DirectiveMatcher<std::decay_t<V>>{std::move(name), std::forward<V>(value)});
    }
}
