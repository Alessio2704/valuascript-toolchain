#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct ReturnStatementContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, StmtVerifier)> add_to_spec;
    };

    class ReturnStatementContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& ret_code)
        {
            std::string res = templ;
            size_t pos = 0;
            while ((pos = res.find("{return}", pos)) != std::string::npos)
            {
                res.replace(pos, 8, ret_code);
                pos += ret_code.length();
            }
            return res;
        }

        static std::vector<ReturnStatementContext> get_all()
        {
            return {
                {
                    "function_body",
                    "func ctx_wrapper() -> void {\n  {return}\n}\n", [](ProgramSpec& s, const StmtVerifier& v)
                    {
                        s.functions.emplace_back(IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {v}));
                    }
                }
            };
        }
    };
}
