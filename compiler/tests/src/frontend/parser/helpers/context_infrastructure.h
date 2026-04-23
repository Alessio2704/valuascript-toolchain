#pragma once

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    enum class NestingLevel { TopLevel, BlockLevel };

    enum class InjectableType
    {
        Import, Directive, Function, Struct, Enum, TypeAlias,
        Statement, Return, Expression, Modifier, TypeAnnotation
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
        NestingLevel level;
        std::vector<InjectableType> allowed_atoms;
        std::string prefix;
        std::string suffix;
        std::function<void(ProgramSpec&, UniversalVerifier)> wrap_in_spec;
    };
}
