#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct ExpressionStatementContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, StmtVerifier)> add_to_spec;
    };

    class ExpressionStatementContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& stmt_code)
        {
            std::string res = templ;
            size_t pos = 0;
            while ((pos = res.find("{expr_stmt}", pos)) != std::string::npos)
            {
                res.replace(pos, 11, stmt_code);
                pos += stmt_code.length();
            }
            return res;
        }

        static std::vector<ExpressionStatementContext> get_all()
        {
            return {
                {
                    "top_level",
                    "{expr_stmt}\n", [](ProgramSpec& s, StmtVerifier v)
                    {
                        s.execution_steps.push_back(v);
                    }
                },
                {
                    "function_body",
                    "func ctx_wrapper() -> void {\n  {expr_stmt}\n}\n", [](ProgramSpec& s, StmtVerifier v)
                    {
                        s.functions.push_back(IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {v}));
                    }
                }
            };
        }
    };
}
