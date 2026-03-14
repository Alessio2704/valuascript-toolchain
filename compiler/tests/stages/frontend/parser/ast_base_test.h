#pragma once
#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

enum class TargetNodeType { Assignment, Function, Struct, Enum };

namespace valuascript::compiler::test {
    class AstBaseTest : public testing::Test {
    protected:
        std::shared_ptr<Program> parse_code(const std::string &code) {
            auto context = std::make_shared<CompilerContext>();
            LexerStage lexer;
            auto lexer_result = lexer.run(context, {
                                              {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                              {CompilerStageArtifactCode::SourceCode, code}
                                          });

            ParserStage parser;
            auto parser_result = parser.run(context, {
                                                {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                                lexer_result
                                            });

            return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
        }

        std::shared_ptr<Program> parse_expression_as_assignment(const std::string &expression) {
            std::string code = "let result = " + expression;

            auto context = std::make_shared<CompilerContext>();
            LexerStage lexer;
            auto lexer_result = lexer.run(context, {
                                              {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                              {CompilerStageArtifactCode::SourceCode, code}
                                          });

            ParserStage parser;
            auto parser_result = parser.run(context, {
                                                {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                                lexer_result
                                            });

            return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
        }

        std::shared_ptr<Program> parse_expression_as_type_annotation(const std::string &expression) {
            std::string code = "let result: " + expression + " = 1";

            auto context = std::make_shared<CompilerContext>();
            LexerStage lexer;
            auto lexer_result = lexer.run(context, {
                                              {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                              {CompilerStageArtifactCode::SourceCode, code}
                                          });

            ParserStage parser;
            auto parser_result = parser.run(context, {
                                                {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                                                lexer_result
                                            });

            return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
        }

        Assignment *get_first_assignment(const std::shared_ptr<Program> &ast) {
            if (ast->execution_steps.empty()) return nullptr;
            return dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        }

        Expression *get_assigned_value(const std::shared_ptr<Program> &ast) {
            if (ast->execution_steps.empty()) return nullptr;
            auto assign = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
            if (!assign) return nullptr;
            return assign->value.get();
        }

        FunctionDefinition *get_first_func(const std::shared_ptr<Program> &ast) {
            if (ast->function_definitions.empty()) return nullptr;
            return ast->function_definitions[0].get();
        }

        ImportStatement *get_import(const std::shared_ptr<Program> &ast) {
            if (ast->import_statements.empty()) return nullptr;
            return ast->import_statements[0].get();
        }

        StructDefinition *get_struct_definition(const std::shared_ptr<Program> &ast) {
            if (ast->struct_definitions.empty()) return nullptr;
            auto struct_def = dynamic_cast<StructDefinition *>(ast->struct_definitions[0].get());
            if (!struct_def) return nullptr;
            return struct_def;
        }

        TensorLiteral *get_assigned_vector(const std::shared_ptr<Program> &ast) {
            if (ast->execution_steps.empty()) return nullptr;
            auto assign = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
            if (!assign) return nullptr;
            return dynamic_cast<TensorLiteral *>(assign->value.get());
        }

        Directive *get_directive(const std::shared_ptr<Program> &ast) {
            if (ast->directives.empty()) return nullptr;
            auto directive = dynamic_cast<Directive *>(ast->directives[0].get());
            if (!directive) return nullptr;
            return directive;
        }

        const std::vector<Modifier> *get_modifiers(const std::shared_ptr<Program> &ast, TargetNodeType type) {
            switch (type) {
                case TargetNodeType::Assignment:
                    if (!ast->execution_steps.empty()) {
                        if (const auto assign = dynamic_cast<Assignment *>(ast->execution_steps[0].get())) {
                            return &assign->modifiers;
                        }
                    }
                    break;
                case TargetNodeType::Function:
                    if (!ast->function_definitions.empty()) return &ast->function_definitions[0]->modifiers;
                    break;
                case TargetNodeType::Struct:
                    if (!ast->struct_definitions.empty()) return &ast->struct_definitions[0]->modifiers;
                    break;
                case TargetNodeType::Enum:
                    if (!ast->enum_definitions.empty()) return &ast->enum_definitions[0]->modifiers;
                    break;
            }
            return {};
        }
    };
}
