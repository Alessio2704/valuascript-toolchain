#pragma once

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct RecoveryBlock;

    enum class InjectableType
    {
        Import, Directive, Function, Struct, Enum, TypeAlias,
        Expression, Modifier, TypeAnnotation,
        WeakStatement, StrongStatement,
        TopLevel
    };

    using UniversalVerifier = std::variant<
        NullVerifier,
        ImportVerifier,
        DirectiveVerifier,
        FuncVerifier,
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

    struct Context
    {
        std::string name;
        std::vector<InjectableType> input_types;
        InjectableType output_type;
        std::string prefix;
        std::string suffix;
        std::function<UniversalVerifier(const UniversalVerifier&)> transform_verifier;

        bool is_block_context = false;
        std::function<UniversalVerifier(const UniversalVerifier&,
                                        const std::vector<RecoveryBlock>&,
                                        const std::vector<RecoveryBlock>&)> transform_verifier_block = nullptr;
    };
}
