#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct AssignmentContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, StmtVerifier)> add_to_spec;
    };

    class AssignmentContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& assign_code)
        {
            std::string res = templ;
            size_t pos = 0;
            while ((pos = res.find("{assignment}", pos)) != std::string::npos)
            {
                res.replace(pos, 12, assign_code);
                pos += assign_code.length();
            }
            return res;
        }

        static std::vector<AssignmentContext> get_all()
        {
            return {
                {
                    "top_level",
                    "{assignment}\n", [](ProgramSpec& s, const StmtVerifier& v)
                    {
                        s.execution_steps.emplace_back(v);
                    }
                },
                {
                    "function_body",
                    "func ctx_wrapper() -> void {\n  {assignment}\n}\n", [](ProgramSpec& s, const StmtVerifier& v)
                    {
                        s.functions.emplace_back(IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {v}));
                    }
                }
            };
        }
    };
}
