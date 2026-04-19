#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct TypeAliasContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, AliasVerifier)> add_to_spec;
    };

    class TypeAliasContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& code)
        {
            std::string res = templ;
            size_t pos = res.find("{typealias}");
            if (pos != std::string::npos)
            {
                res.replace(pos, 11, code);
            }
            return res;
        }

        static std::vector<TypeAliasContext> get_all()
        {
            return {
                {
                    "top_level",
                    "{typealias}\n", [](ProgramSpec& s, AliasVerifier v)
                    {
                        s.type_aliases.push_back(v);
                    }
                }
            };
        }
    };
}
