#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace valuascript::compiler
{
    enum class AstKind : uint8_t
    {
        Unknown,
        NumberLiteral,
        PercentageLiteral,
        StringLiteral,
        BooleanLiteral,
        IdentifierAccess,
        SelfExpression,
        BinaryExpression,
        UnaryExpression,
        GroupingExpression,
        ConditionalExpression,
        FunctionCall,
        DictLiteral,
        TensorLiteral,
        TupleLiteral,
        BracketAccess,
        DotAccess,
        SwitchExpression,
        Assignment,
        Reassignment,
        ExpressionStatement,
        ReturnStatement,
        EnumDefinition,
        Directive,
        ImportStatement,
        FunctionDefinition,
        StructDefinition,
        TypeAliasDefinition,
        ExtensionDefinition,
        Program,
        TypeAnnotation,
        TupleTypeAnnotation,
        FunctionParameter,
        StructField,
        EnumCase,
        SwitchCase,
        AssignmentTarget,
        Modifier,
        CallArgument,
        DictItem,
        Comment,
        _Count
    };

    inline constexpr size_t AST_KIND_COUNT = static_cast<size_t>(AstKind::_Count) - 1;

    constexpr std::string_view to_string(AstKind kind) noexcept
    {
        switch (kind)
        {
            case AstKind::Unknown: return "Unknown";
            case AstKind::_Count: return "Unknown";
            case AstKind::NumberLiteral: return "NumberLiteral";
            case AstKind::PercentageLiteral: return "PercentageLiteral";
            case AstKind::StringLiteral: return "StringLiteral";
            case AstKind::BooleanLiteral: return "BooleanLiteral";
            case AstKind::IdentifierAccess: return "IdentifierAccess";
            case AstKind::SelfExpression: return "SelfExpression";
            case AstKind::BinaryExpression: return "BinaryExpression";
            case AstKind::UnaryExpression: return "UnaryExpression";
            case AstKind::GroupingExpression: return "GroupingExpression";
            case AstKind::ConditionalExpression: return "ConditionalExpression";
            case AstKind::FunctionCall: return "FunctionCall";
            case AstKind::DictLiteral: return "DictLiteral";
            case AstKind::TensorLiteral: return "TensorLiteral";
            case AstKind::TupleLiteral: return "TupleLiteral";
            case AstKind::BracketAccess: return "BracketAccess";
            case AstKind::DotAccess: return "DotAccess";
            case AstKind::SwitchExpression: return "SwitchExpression";
            case AstKind::Assignment: return "Assignment";
            case AstKind::Reassignment: return "Reassignment";
            case AstKind::ExpressionStatement: return "ExpressionStatement";
            case AstKind::ReturnStatement: return "ReturnStatement";
            case AstKind::EnumDefinition: return "EnumDefinition";
            case AstKind::Directive: return "Directive";
            case AstKind::ImportStatement: return "ImportStatement";
            case AstKind::FunctionDefinition: return "FunctionDefinition";
            case AstKind::StructDefinition: return "StructDefinition";
            case AstKind::TypeAliasDefinition: return "TypeAliasDefinition";
            case AstKind::ExtensionDefinition: return "ExtensionDefinition";
            case AstKind::Program: return "Program";
            case AstKind::TypeAnnotation: return "TypeAnnotation";
            case AstKind::TupleTypeAnnotation: return "TupleTypeAnnotation";
            case AstKind::FunctionParameter: return "FunctionParameter";
            case AstKind::StructField: return "StructField";
            case AstKind::EnumCase: return "EnumCase";
            case AstKind::SwitchCase: return "SwitchCase";
            case AstKind::AssignmentTarget: return "AssignmentTarget";
            case AstKind::Modifier: return "Modifier";
            case AstKind::CallArgument: return "CallArgument";
            case AstKind::DictItem: return "DictItem";
            case AstKind::Comment: return "Comment";
        }
        return "Unknown";
    }
}
