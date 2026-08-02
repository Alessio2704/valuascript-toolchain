#pragma once

#include "../errors_synchronization/parser_errors_synchronization_base.h"
#include "language_constructs_provider.h"

namespace valuascript::compiler::test
{
    namespace
    {
        struct WrapperEnv
        {
            std::string name;
            std::string pre_code;
            std::string post_code;
            bool supports_modifiers;
            std::function<void(const Program&)> verify;
            bool expects_missing_value_error = false;
        };

        std::pair<size_t, size_t> get_end_position(const std::string& text)
        {
            size_t line = 1;
            size_t col = 1;
            for (char c : text)
            {
                if (c == '\n')
                {
                    line++;
                    col = 1;
                }
                else
                {
                    col++;
                }
            }
            return {line, col};
        }

        size_t get_first_token_length(const std::string& source)
        {
            if (source.starts_with("func")) return 4;
            if (source.starts_with("struct")) return 6;
            if (source.starts_with("enum")) return 4;
            if (source.starts_with("typealias")) return 9;
            if (source.starts_with("import")) return 6;
            if (source.starts_with("let")) return 3;
            if (source.starts_with("#")) return 1;
            if (source.starts_with("@")) return 1;

            size_t space_idx = source.find_first_of(" \t\r\n");
            return space_idx == std::string::npos ? source.length() : space_idx;
        }

        std::vector<WrapperEnv> get_environments()
        {
            return {
                {
                    .name = "in_function",
                    .pre_code = "func wrapper() -> void {\n    let valid_stmt_1 = 1\n    ",
                    .post_code = "    let valid_stmt_2 = 2\n}\n",
                    .supports_modifiers = true,
                    .verify = [](const Program& p)
                    {
                        auto s = find_statement(p, [](const Statement* st)
                        {
                            auto a = dynamic_cast<const Assignment*>(st);
                            return a && !a->targets.empty() && a->targets[0].name == "valid_stmt_2";
                        });
                        EXPECT_NE(s, nullptr) << "Recovery statement 'valid_stmt_2' not found in function body.";
                    }
                },
                {
                    .name = "in_struct",
                    .pre_code = "struct WrapperStruct { valid_field_1: int, ",
                    .post_code = ", valid_field_2: int }\n",
                    .supports_modifiers = true,
                    .verify = [](const Program& p)
                    {
                        auto it = std::find_if(p.struct_definitions.begin(), p.struct_definitions.end(),
                                               [](auto& s) { return s->name == "WrapperStruct"; });
                        ASSERT_NE(it, p.struct_definitions.end()) << "WrapperStruct not found.";
                        EXPECT_GE((*it)->fields.size(), 2) << "Recovered field not found.";
                        if ((*it)->fields.size() >= 2)
                        {
                            EXPECT_EQ((*it)->fields.front().name, "valid_field_1");
                            EXPECT_EQ((*it)->fields.back().name, "valid_field_2");
                        }
                    }
                },
                {
                    .name = "in_enum",
                    .pre_code = "enum WrapperEnum: int { ValidCase1, ",
                    .post_code = ", ValidCase2 }\n",
                    .supports_modifiers = true,
                    .verify = [](const Program& p)
                    {
                        auto it = std::find_if(p.enum_definitions.begin(), p.enum_definitions.end(),
                                               [](auto& e) { return e->name == "WrapperEnum"; });
                        ASSERT_NE(it, p.enum_definitions.end()) << "WrapperEnum not found.";
                        EXPECT_GE((*it)->cases.size(), 2) << "Recovered case not found.";
                        if ((*it)->cases.size() >= 2)
                        {
                            EXPECT_EQ((*it)->cases.front().name, "ValidCase1");
                            EXPECT_EQ((*it)->cases.back().name, "ValidCase2");
                        }
                    }
                },
                {
                    .name = "in_grouping",
                    .pre_code = "func wrapper() -> void {\n    let a = ( 1, ",
                    .post_code = ", 2 )\n}\n",
                    .supports_modifiers = false,
                    .verify = [](const Program& p)
                    {
                        auto s = find_statement(p, [](const Statement* st)
                        {
                            auto a = dynamic_cast<const Assignment*>(st);
                            if (a && !a->targets.empty() && a->targets[0].name == "a")
                            {
                                auto tuple = dynamic_cast<const TupleLiteral*>(a->value.get());
                                if (tuple && tuple->elements.size() >= 2) return true;
                            }
                            return false;
                        });
                        EXPECT_NE(s, nullptr) << "Recovered tuple assignment 'let a = (1, 2)' not found.";
                    }
                },
                {
                    .name = "in_tensor_literal",
                    .pre_code = "func wrapper() -> void {\n    let a = [ 1, ",
                    .post_code = ", 2 ]\n}\n",
                    .supports_modifiers = false,
                    .verify = [](const Program& p)
                    {
                        auto s = find_statement(p, [](const Statement* st)
                        {
                            auto a = dynamic_cast<const Assignment*>(st);
                            if (a && !a->targets.empty() && a->targets[0].name == "a")
                            {
                                auto tensor = dynamic_cast<const TensorLiteral*>(a->value.get());
                                if (tensor && tensor->elements.size() >= 2) return true;
                            }
                            return false;
                        });
                        EXPECT_NE(s, nullptr) << "Recovered tensor assignment 'let a = [1, 2]' not found.";
                    }
                },
                {
                    .name = "in_switch_condition",
                    .pre_code = "func wrapper() -> void {\n    let a = switch ( ",
                    .post_code = " ) { default -> 1 }\n}\n",
                    .supports_modifiers = false,
                    .verify = [](const Program& p)
                    {
                        auto s = find_statement(p, [](const Statement* st)
                        {
                            auto a = dynamic_cast<const Assignment*>(st);
                            if (a && !a->targets.empty() && a->targets[0].name == "a")
                            {
                                auto sw = dynamic_cast<const SwitchExpression*>(a->value.get());
                                if (sw && sw->default_case != nullptr) return true;
                            }
                            return false;
                        });
                        EXPECT_NE(s, nullptr) << "Recovered switch expression 'switch() { default -> 1 }' not found.";
                    }
                },
                {
                    .name = "in_switch_body",
                    .pre_code = "func wrapper() -> void {\n    let a = switch (1) { ",
                    .post_code = " default -> 2 }\n}\n",
                    .supports_modifiers = false,
                    .verify = [](const Program& p)
                    {
                        auto s = find_statement(p, [](const Statement* st)
                        {
                            auto a = dynamic_cast<const Assignment*>(st);
                            if (a && !a->targets.empty() && a->targets[0].name == "a")
                            {
                                auto sw = dynamic_cast<const SwitchExpression*>(a->value.get());
                                if (sw && sw->default_case != nullptr) return true;
                            }
                            return false;
                        });
                        EXPECT_NE(s, nullptr) << "Recovered switch body default case not found.";
                    }
                },
                {
                    .name = "in_assignment_rhs",
                    .pre_code = "func wrapper() -> void {\n    let a = ",
                    .post_code = "\n    let valid_stmt_2 = 2\n}\n",
                    .supports_modifiers = false,
                    .verify = [](const Program& p)
                    {
                        auto s = find_statement(p, [](const Statement* st)
                        {
                            auto a = dynamic_cast<const Assignment*>(st);
                            return a && !a->targets.empty() && a->targets[0].name == "valid_stmt_2";
                        });
                        EXPECT_NE(s, nullptr) << "Recovered next statement 'valid_stmt_2' not found.";
                    },
                    .expects_missing_value_error = true
                }
            };
        }
    }
}
