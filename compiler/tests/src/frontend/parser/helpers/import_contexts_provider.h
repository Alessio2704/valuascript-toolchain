#pragma once
#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct ImportContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, ImportVerifier)> add_to_spec;
    };

    class ImportContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& code)
        {
            std::string res = templ;
            size_t pos = res.find("{import}");
            if (pos != std::string::npos) res.replace(pos, 8, code);
            return res;
        }

        static std::vector<ImportContext> get_all()
        {
            return {{"top_level", "{import}\n", [](ProgramSpec& s, ImportVerifier v) { s.imports.push_back(v); }}};
        }
    };
}
