#pragma once

#include "utils/function/inline_function.h"
#include "ast/ast.h"

namespace valuascript::compiler::test
{
    template <typename NodeT, size_t BufferSize = 64>
    using InlineVerifier = valuascript::shared::InlineFunction<void(NodeT*), BufferSize>;

    using ExprVerifier = InlineVerifier<Expression>;
    using TypeVerifier = InlineVerifier<TypeAnnotation>;
    using ImportVerifier = InlineVerifier<ImportStatement>;
    using DirectiveVerifier = InlineVerifier<Directive>;
    using FuncVerifier = InlineVerifier<FunctionDefinition>;
    using ExtVerifier = InlineVerifier<ExtensionDefinition>;
    using StructVerifier = InlineVerifier<StructDefinition>;
    using EnumVerifier = InlineVerifier<EnumDefinition>;
    using AliasVerifier = InlineVerifier<TypeAliasDefinition>;
    using AssignmentVerifier = InlineVerifier<Assignment>;
    using ReassignmentVerifier = InlineVerifier<Reassignment>;
    using ReturnVerifier = InlineVerifier<ReturnStatement>;
    using ExprStmtVerifier = InlineVerifier<ExpressionStatement>;
}
