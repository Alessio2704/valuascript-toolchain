#pragma once
#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct StructDefinitionContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, StructVerifier)> add_to_spec;
    };

    class StructDefinitionContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& code)
        {
            std::string res = templ;
            size_t pos = res.find("{struct}");
            if (pos != std::string::npos) res.replace(pos, 8, code);
            return res;
        }

        static std::vector<StructDefinitionContext> get_all()
        {
            return {{"top_level", "{struct}\n", [](ProgramSpec& s, const StructVerifier& v) { s.structs.push_back(v); }}};
        }
    };
}
