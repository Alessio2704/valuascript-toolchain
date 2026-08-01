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
        FunctionBody,
        ExtensionBody
    };

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
