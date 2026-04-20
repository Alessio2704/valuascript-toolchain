#pragma once
#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct FunctionDefinitionContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, FuncVerifier)> add_to_spec;
    };

    class FunctionDefinitionContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& code)
        {
            std::string res = templ;
            size_t pos = res.find("{func}");
            if (pos != std::string::npos) res.replace(pos, 6, code);
            return res;
        }

        static std::vector<FunctionDefinitionContext> get_all()
        {
            return {
                {
                    "top_level", "{func}\n", [](ProgramSpec& s, const FuncVerifier& v)
                    {
                        s.functions.emplace_back(v);
                    }
                }
            };
        }
    };
}
