#pragma once

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct RecoveryBlock;

    enum class BlockContext
    {
        None,
        FunctionBody,
        ExtensionBody
    };

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
}
