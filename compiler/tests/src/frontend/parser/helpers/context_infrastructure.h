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
        TypeVerifier
    >;

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

        OneOf(NullVerifier v) : value(v)
        {
        }

        OneOf(UniversalVerifier v) : value(std::move(v))
        {
        }
    };

    struct Context
    {
        std::string name;
        std::vector<InjectableType> input_types;
        InjectableType output_type;
        std::string prefix;
        std::string suffix;
        std::function<UniversalVerifier(const UniversalVerifier&)> transform_verifier;

        BlockContext block_context = BlockContext::None;
        std::function<UniversalVerifier(const UniversalVerifier&,
                                        const std::vector<RecoveryBlock>&,
                                        const std::vector<RecoveryBlock>&)> transform_verifier_block = nullptr;
    };
}
