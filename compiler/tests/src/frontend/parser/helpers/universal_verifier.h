#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "injectable_type.h"
#include "node_matchers.h"

namespace valuascript::compiler::test
{
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
}
