#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct ReassignmentContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, StmtVerifier)> add_to_spec;
    };

    class ReassignmentContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& reassign_code)
        {
            std::string res = templ;
            size_t pos = 0;
            while ((pos = res.find("{reassignment}", pos)) != std::string::npos)
            {
                res.replace(pos, 14, reassign_code);
                pos += reassign_code.length();
            }
            return res;
        }

        static std::vector<ReassignmentContext> get_all()
        {
            return {
                {
                    "top_level",
                    "{reassignment}\n", [](ProgramSpec& s, StmtVerifier v)
                    {
                        s.execution_steps.push_back(v);
                    }
                },
                {
                    "function_body",
                    "func ctx_wrapper() -> void {\n  {reassignment}\n}\n", [](ProgramSpec& s, StmtVerifier v)
                    {
                        s.functions.push_back(IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {v}));
                    }
                }
            };
        }
    };
}
