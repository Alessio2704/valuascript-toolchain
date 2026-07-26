#pragma once
#include <gtest/gtest.h>
#include "frontend/parser/parser_stage.h"
#include "frontend/lexer/lexer_stage.h"
#include "frontend/parser/ast.h"
#include "utils/parametrised_test_name_helper.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    enum class TargetNodeType
    {
        Assignment,
        Function,
        Struct,
        Enum,
        DictKey,
        EnumCase,
        FunctionParameter,
        StructField
    };

    class AstBaseTest : public testing::Test
    {
    protected:
        std::shared_ptr<Program> parse_code(const std::string& code, const bool fail_fast = true)
        {
            auto context = std::make_shared<CompilerContext>();
            context->settings.fail_fast = fail_fast;
            LexerStage lexer;
            auto lexer_result = lexer.run(*context, {
                                              {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                              {CompilerStageArtifactCode::SourceCode, code}
                                          });

            ParserStage parser;
            auto parser_result = parser.run(*context, {
                                                {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                                lexer_result
                                            });

            return extract_artifact_data<std::shared_ptr<Program>>({parser_result}, CompilerStageArtifactCode::Ast);
        }

        std::shared_ptr<Program> parse_expression_as_assignment(const std::string& expression)
        {
            std::string code = "let result = " + expression;

            auto context = std::make_shared<CompilerContext>();
            LexerStage lexer;
            auto lexer_result = lexer.run(*context, {
                                              {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                              {CompilerStageArtifactCode::SourceCode, code}
                                          });

            ParserStage parser;
            auto parser_result = parser.run(*context, {
                                                {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                                lexer_result
                                            });

            return extract_artifact_data<std::shared_ptr<Program>>({parser_result}, CompilerStageArtifactCode::Ast);
        }

        std::shared_ptr<Program> parse_expression_as_type_annotation(const std::string& expression)
        {
            std::string code = "let result: " + expression + " = 1";

            auto context = std::make_shared<CompilerContext>();
            LexerStage lexer;
            auto lexer_result = lexer.run(*context, {
                                              {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                              {CompilerStageArtifactCode::SourceCode, code}
                                          });

            ParserStage parser;
            auto parser_result = parser.run(*context, {
                                                {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                                lexer_result
                                            });

            return extract_artifact_data<std::shared_ptr<Program>>({parser_result}, CompilerStageArtifactCode::Ast);
        }

        Assignment* get_first_assignment(const std::shared_ptr<Program>& ast)
        {
            if (ast->execution_steps.empty()) return nullptr;
            return dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        }

        Expression* get_assigned_value(const std::shared_ptr<Program>& ast)
        {
            if (ast->execution_steps.empty()) return nullptr;
            auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
            if (!assign) return nullptr;
            return assign->value.get();
        }

        FunctionDefinition* get_first_func(const std::shared_ptr<Program>& ast)
        {
            if (ast->function_definitions.empty()) return nullptr;
            return ast->function_definitions[0].get();
        }

        ImportStatement* get_import(const std::shared_ptr<Program>& ast)
        {
            if (ast->import_statements.empty()) return nullptr;
            return ast->import_statements[0].get();
        }

        StructDefinition* get_struct_definition(const std::shared_ptr<Program>& ast)
        {
            if (ast->struct_definitions.empty()) return nullptr;
            auto struct_def = dynamic_cast<StructDefinition*>(ast->struct_definitions[0].get());
            if (!struct_def) return nullptr;
            return struct_def;
        }

        TensorLiteral* get_assigned_vector(const std::shared_ptr<Program>& ast)
        {
            if (ast->execution_steps.empty()) return nullptr;
            auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
            if (!assign) return nullptr;
            return dynamic_cast<TensorLiteral*>(assign->value.get());
        }

        Directive* get_directive(const std::shared_ptr<Program>& ast)
        {
            if (ast->directives.empty()) return nullptr;
            auto directive = dynamic_cast<Directive*>(ast->directives[0].get());
            if (!directive) return nullptr;
            return directive;
        }
    };
}
