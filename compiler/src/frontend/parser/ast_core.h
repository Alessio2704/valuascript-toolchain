#pragma once

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "token/token.h"

using namespace valuascript::shared;

namespace valuascript::compiler
{
    class Expression;
    class Statement;
    class TypeAnnotation;
    class ImportStatement;
    class Directive;
    class FunctionDefinition;
    class StructDefinition;
    class EnumDefinition;
    class TypeAliasDefinition;
    class ExtensionDefinition;

    using ExprPtr = std::unique_ptr<Expression>;
    using StmtPtr = std::unique_ptr<Statement>;
    using TypeAnnPtr = std::unique_ptr<TypeAnnotation>;

    using ImportPtr = std::unique_ptr<ImportStatement>;
    using DirectivePtr = std::unique_ptr<Directive>;
    using FuncDefPtr = std::unique_ptr<FunctionDefinition>;
    using StructDefPtr = std::unique_ptr<StructDefinition>;
    using EnumDefPtr = std::unique_ptr<EnumDefinition>;
    using TypeAliasPtr = std::unique_ptr<TypeAliasDefinition>;
    using ExtensionDefPtr = std::unique_ptr<ExtensionDefinition>;

    struct SourceSpan
    {
        size_t line_start = 0;
        size_t column_start = 0;
        size_t line_end = 0;
        size_t column_end = 0;
        std::string file_path;
    };

    class AstNode
    {
    public:
        SourceSpan span;
        virtual ~AstNode() = default;
    };

    class Expression : public AstNode
    {
    public:
        [[nodiscard]] virtual bool is_complete() const { return true; }
    };

    class Statement : public AstNode
    {
    };

    struct Modifier
    {
        std::string name;
        std::vector<std::pair<std::string, ExprPtr>> arguments;
        SourceSpan span;
    };
}
