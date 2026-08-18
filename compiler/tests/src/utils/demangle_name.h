#pragma once

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#include <cstdlib>
#endif

namespace valuascript::compiler::test
{
    inline std::string get_demangled_name(const char* mangled_name)
    {
#if defined(__GNUC__) || defined(__clang__)
        int status = 0;
        char* demangled = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
        std::string result = (status == 0 && demangled) ? demangled : mangled_name;
        std::free(demangled);

        std::string prefix = "valuascript::compiler::";
        if (result.find(prefix) == 0)
        {
            result = result.substr(prefix.length());
        }

        return result;
#else
        return mangled_name;
#endif
    }
}
