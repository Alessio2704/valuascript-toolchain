#pragma once

#include <memory>
#include <vector>
#include "stages/frontend/parser/ast.h"
#include "stages/frontend/parser/token_cursor.h"

namespace valuascript::compiler {

    struct ParseSyncException : std::exception {
        [[nodiscard]] const char* what() const noexcept override {
            return "Parser panic mode triggered.";
        }
    };

    class Parser {
    private:
        TokenCursor cursor_;

    public:
        explicit Parser(TokenCursor cursor);

        std::unique_ptr<Program> parse_program();

    private:
        // Core & Declarations
        void parse_top_level_declaration(Program *program);

        std::vector<Modifier> parse_modifiers();

        std::unique_ptr<ImportStatement> parse_import_statement();

        std::unique_ptr<Directive> parse_directive();

        std::unique_ptr<StructDefinition> parse_struct_definition(std::vector<Modifier> modifiers);

        std::unique_ptr<EnumDefinition> parse_enum_definition(std::vector<Modifier> modifiers);

        std::unique_ptr<FunctionDefinition> parse_function_definition(std::vector<Modifier> modifiers);

        std::unique_ptr<TypeAnnotation> parse_type_annotation();

        // Statements
        std::unique_ptr<Statement> parse_statement();

        std::unique_ptr<Assignment> parse_assignment(std::vector<Modifier> modifiers);

        std::unique_ptr<Statement> parse_expression_statement();

        std::unique_ptr<ReturnStatement> parse_return_statement();

        // Expressions
        std::unique_ptr<Expression> parse_expression();

        std::unique_ptr<Expression> parse_or_expression();

        std::unique_ptr<Expression> parse_and_expression();

        std::unique_ptr<Expression> parse_comparison_expression();

        std::unique_ptr<Expression> parse_addition_expression();

        std::unique_ptr<Expression> parse_multiplication_expression();

        std::unique_ptr<Expression> parse_power_expression();

        std::unique_ptr<Expression> parse_unary_expression();

        std::unique_ptr<Expression> parse_postfix_expression();

        std::unique_ptr<Expression> parse_primary_expression();

        std::unique_ptr<Expression> parse_tuple_or_grouping();

        std::unique_ptr<Expression> parse_tensor_literal();

        std::unique_ptr<Expression> parse_dict_literal();

        std::unique_ptr<Expression> parse_function_call(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_tensor_access(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_switch_expression();

        static bool is_valid_lvalue(const Expression *expr);

        static bool is_expression_start(TokenType type);

        static bool is_binary_operator(TokenType type);

        void check_trailing_expression() const;

        void synchronize();
    };
}
