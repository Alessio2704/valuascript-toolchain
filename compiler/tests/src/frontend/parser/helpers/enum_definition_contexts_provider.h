#pragma once
#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct EnumDefinitionContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, EnumVerifier)> add_to_spec;
    };

    class EnumDefinitionContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& code)
        {
            std::string res = templ;
            size_t pos = res.find("{enum}");
            if (pos != std::string::npos) res.replace(pos, 6, code);
            return res;
        }

        static std::vector<EnumDefinitionContext> get_all()
        {
            return {{"top_level", "{enum}\n", [](ProgramSpec& s, EnumVerifier v) { s.enums.push_back(v); }}};
        }
    };
}
