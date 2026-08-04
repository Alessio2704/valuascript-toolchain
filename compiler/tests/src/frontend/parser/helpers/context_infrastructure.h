#pragma once

#include <string>
#include <vector>
#include <array>
#include <variant>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct RecoveryBlock;

    enum class BlockContext
    {
        None,
        TopLevel,
        FunctionBody,
        ExtensionBody
    };

    inline constexpr bool is_nested_block_context(BlockContext ctx)
    {
        return ctx != BlockContext::None && ctx != BlockContext::TopLevel;
    }

    enum class SentinelKind
    {
        Assignment,
        Reassignment,
        ExprStmt,
        Return,
        Import,
        Function,
        Enum,
        Alias,
        Directive,
        Struct
    };

    namespace SentinelKinds
    {
        inline constexpr std::array<SentinelKind, 10> All = {
            SentinelKind::Assignment,
            SentinelKind::Reassignment,
            SentinelKind::ExprStmt,
            SentinelKind::Return,
            SentinelKind::Import,
            SentinelKind::Function,
            SentinelKind::Enum,
            SentinelKind::Alias,
            SentinelKind::Directive,
            SentinelKind::Struct
        };

        inline std::vector<SentinelKind> all()
        {
            return {All.begin(), All.end()};
        }
    }

    inline std::string to_string(SentinelKind kind)
    {
        switch (kind)
        {
        case SentinelKind::Assignment: return "Assignment";
        case SentinelKind::Reassignment: return "Reassignment";
        case SentinelKind::ExprStmt: return "ExprStmt";
        case SentinelKind::Return: return "Return";
        case SentinelKind::Import: return "Import";
        case SentinelKind::Function: return "Function";
        case SentinelKind::Enum: return "Enum";
        case SentinelKind::Alias: return "Alias";
        case SentinelKind::Directive: return "Directive";
        case SentinelKind::Struct: return "Struct";
        }
        return "Unknown";
    }

    enum class InjectableType
    {
        Identifier,
        Import, Directive, Function, Extension, Struct, Enum, TypeAlias,
        Expression, Modifier, TypeAnnotation,
        WeakStatement, StrongStatement,
        TopLevel
    };

    struct MultiInjectVerifier;

    using UniversalVerifier = std::variant<
        NullVerifier,
        std::string,
        ImportVerifier,
        DirectiveVerifier,
        FuncVerifier,
        ExtVerifier,
        StructVerifier,
        EnumVerifier,
        AliasVerifier,
        StmtVerifier,
        ReturnVerifier,
        ExprStmtVerifier,
        ExprVerifier,
        ModifierVerifier,
        TypeVerifier,
        std::shared_ptr<MultiInjectVerifier>
    >;

    enum class VerifierCategory
    {
        Expression,
        Type,
        Identifier,
        Modifier,
        Statement,
        TopLevelContainer,
        Null,
        Unknown
    };

    template <typename T>
    struct verifier_traits
    {
        static constexpr VerifierCategory category = VerifierCategory::Unknown;
    };

    template <> struct verifier_traits<ExprVerifier> { static constexpr VerifierCategory category = VerifierCategory::Expression; };
    template <> struct verifier_traits<TypeVerifier> { static constexpr VerifierCategory category = VerifierCategory::Type; };
    template <> struct verifier_traits<std::string> { static constexpr VerifierCategory category = VerifierCategory::Identifier; };
    template <> struct verifier_traits<ModifierVerifier> { static constexpr VerifierCategory category = VerifierCategory::Modifier; };
    template <> struct verifier_traits<NullVerifier> { static constexpr VerifierCategory category = VerifierCategory::Null; };
    template <> struct verifier_traits<StmtVerifier> { static constexpr VerifierCategory category = VerifierCategory::Statement; };
    template <> struct verifier_traits<ExprStmtVerifier> { static constexpr VerifierCategory category = VerifierCategory::Statement; };
    template <> struct verifier_traits<ReturnVerifier> { static constexpr VerifierCategory category = VerifierCategory::Statement; };
    template <> struct verifier_traits<FuncVerifier> { static constexpr VerifierCategory category = VerifierCategory::TopLevelContainer; };
    template <> struct verifier_traits<StructVerifier> { static constexpr VerifierCategory category = VerifierCategory::TopLevelContainer; };
    template <> struct verifier_traits<EnumVerifier> { static constexpr VerifierCategory category = VerifierCategory::TopLevelContainer; };
    template <> struct verifier_traits<ImportVerifier> { static constexpr VerifierCategory category = VerifierCategory::TopLevelContainer; };
    template <> struct verifier_traits<DirectiveVerifier> { static constexpr VerifierCategory category = VerifierCategory::TopLevelContainer; };
    template <> struct verifier_traits<ExtVerifier> { static constexpr VerifierCategory category = VerifierCategory::TopLevelContainer; };
    template <> struct verifier_traits<AliasVerifier> { static constexpr VerifierCategory category = VerifierCategory::TopLevelContainer; };

    template <typename V>
    concept ExpressionVerifierConcept = (verifier_traits<std::decay_t<V>>::category == VerifierCategory::Expression);

    template <typename V>
    concept TypeVerifierConcept = (verifier_traits<std::decay_t<V>>::category == VerifierCategory::Type);

    template <typename V>
    concept IdentifierVerifierConcept = (verifier_traits<std::decay_t<V>>::category == VerifierCategory::Identifier);

    template <typename V>
    concept ModifierVerifierConcept = (verifier_traits<std::decay_t<V>>::category == VerifierCategory::Modifier);

    template <typename V>
    concept StatementVerifierConcept = (verifier_traits<std::decay_t<V>>::category == VerifierCategory::Statement);

    template <typename V>
    concept TopLevelContainerConcept = (verifier_traits<std::decay_t<V>>::category == VerifierCategory::TopLevelContainer);

    template <typename TargetVerifier>
    struct OneOf
    {
        UniversalVerifier value;

        OneOf(TargetVerifier v) : value(std::move(v))
        {
        }

        template <typename Dummy = NullVerifier>
            requires (!std::is_same_v<TargetVerifier, NullVerifier>)
        OneOf(NullVerifier v) : value(v)
        {
        }

        template <typename Dummy = UniversalVerifier>
            requires (!std::is_same_v<TargetVerifier, UniversalVerifier>)
        OneOf(UniversalVerifier v) : value(std::move(v))
        {
        }
    };

    struct MultiInjectVerifier
    {
        UniversalVerifier binding_required;
        std::vector<UniversalVerifier> multi_element;
    };

    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };

    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    inline bool is_injectable_payload_for_context(const UniversalVerifier& v, const std::vector<InjectableType>& input_types)
    {
        return std::visit([&](const auto& verifier) -> bool {
            using T = std::decay_t<decltype(verifier)>;
            if constexpr (ExpressionVerifierConcept<T>)
            {
                return std::find(input_types.begin(), input_types.end(), InjectableType::Expression) != input_types.end();
            }
            else if constexpr (TypeVerifierConcept<T>)
            {
                return std::find(input_types.begin(), input_types.end(), InjectableType::TypeAnnotation) != input_types.end();
            }
            else if constexpr (IdentifierVerifierConcept<T>)
            {
                return std::find(input_types.begin(), input_types.end(), InjectableType::Identifier) != input_types.end();
            }
            else if constexpr (ModifierVerifierConcept<T>)
            {
                return std::find(input_types.begin(), input_types.end(), InjectableType::Modifier) != input_types.end();
            }
            else if constexpr (StatementVerifierConcept<T>)
            {
                return std::find(input_types.begin(), input_types.end(), InjectableType::StrongStatement) != input_types.end() ||
                       std::find(input_types.begin(), input_types.end(), InjectableType::WeakStatement) != input_types.end();
            }
            else if constexpr (TopLevelContainerConcept<T>)
            {
                return std::find(input_types.begin(), input_types.end(), InjectableType::TopLevel) != input_types.end();
            }
            else if constexpr (std::is_same_v<T, NullVerifier>)
            {
                return true;
            }
            else
            {
                return false;
            }
        }, v);
    }

    struct Context
    {
        std::string_view name;
        std::vector<InjectableType> input_types;
        InjectableType output_type;
        std::string prefix;
        std::string suffix;
        std::function<UniversalVerifier(const UniversalVerifier&)> transform_verifier;

        BlockContext block_context = BlockContext::None;
        std::function<UniversalVerifier(const UniversalVerifier&,
                                        const std::vector<RecoveryBlock>&,
                                        const std::vector<RecoveryBlock>&)> transform_verifier_block = nullptr;

        bool operator_binding_required = true;
        std::function<UniversalVerifier(const std::vector<UniversalVerifier>&)> transform_multi_verifier = nullptr;
    };

    std::string_view get_injectable_type_keyword(InjectableType type);
    bool is_valid_declaration_keyword(InjectableType type, const std::string& code);
    bool has_unclosed_brace(const std::string& code);
}
