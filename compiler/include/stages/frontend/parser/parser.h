#pragma once

#include <memory>
#include <vector>
#include "stages/frontend/parser/ast.h"
#include "stages/frontend/parser/token_cursor.h"

namespace valuascript::compiler {
    struct ParseSyncException : std::exception {
        [[nodiscard]] const char *what() const noexcept override {
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

        enum class Precedence {
            None = 0,
            Or = 1,         // or
            And = 2,        // and
            Comparison = 3, // ==, !=, <, >, <=, >=
            Term = 4,       // +, -
            Factor = 5,     // *, /, %
            Power = 6       // ^
        };

        static Precedence get_operator_precedence(TokenType type) ;

        [[nodiscard]] static bool is_right_associative(TokenType type);

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

        std::unique_ptr<Expression> parse_expression(Precedence precedence = Precedence::Or);

        std::unique_ptr<Expression> parse_unary_expression();

        std::unique_ptr<Expression> parse_postfix_expression();

        std::unique_ptr<Expression> parse_primary_expression();

        std::unique_ptr<Expression> parse_tuple_or_grouping();

        std::unique_ptr<Expression> parse_tensor_literal();

        std::unique_ptr<Expression> parse_dict_literal();

        std::unique_ptr<Expression> parse_function_call(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_tensor_access(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_dot_access(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_conditional_expression();

        std::unique_ptr<Expression> parse_switch_expression();

        static bool is_valid_lvalue(const Expression *expr);

        static bool is_expression_start(TokenType type);

        static bool is_binary_operator(TokenType type);

        void verify_statement_end() const;

        std::vector<std::unique_ptr<Expression> > parse_expression_list(
            TokenType closing_token,
            std::optional<ErrorCode> trailing_comma_err = std::nullopt);

        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > parse_key_value_list(
            TokenType closing_token,
            ErrorCode key_err, const std::string &key_msg,
            ErrorCode colon_err, const std::string &colon_msg,
            ErrorCode missing_comma_err,
            std::optional<ErrorCode> trailing_comma_err = std::nullopt);

        void synchronize();

        template<typename T, typename ElementParser>
        std::vector<T> parse_comma_separated_list(
            TokenType closing_token,
            ErrorCode missing_comma_err,
            const std::string &missing_comma_msg,
            ElementParser parse_element) {
            std::vector<T> elements;

            while (!cursor_.check(closing_token) && !cursor_.is_at_end()) {
                elements.push_back(parse_element());

                if (cursor_.match({TokenType::Comma})) {
                } else if (!cursor_.check(closing_token)) {
                    if (cursor_.check(TokenType::Identifier)) {
                        cursor_.report_error(cursor_.peek(), missing_comma_err, missing_comma_msg);
                    } else {
                        break;
                    }
                }
            }
            return elements;
        }
    };
}
