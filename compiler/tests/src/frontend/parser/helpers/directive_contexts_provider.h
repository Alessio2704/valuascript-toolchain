#pragma once
#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct DirectiveContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, DirectiveVerifier)> add_to_spec;
    };

    class DirectiveContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& code)
        {
            std::string res = templ;
            size_t pos = res.find("{directive}");
            if (pos != std::string::npos) res.replace(pos, 11, code);
            return res;
        }

        static std::vector<DirectiveContext> get_all()
        {
            return {
                {"top_level", "{directive}\n", [](ProgramSpec& s, DirectiveVerifier v) { s.directives.push_back(v); }}
            };
        }
    };
}
