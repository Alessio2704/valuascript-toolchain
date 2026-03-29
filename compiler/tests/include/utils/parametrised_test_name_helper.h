#pragma once
#include <concepts>
#include <iosfwd>

namespace valuascript::compiler::test {
    template<typename T>
    concept HasTestIdentifier = requires(T t) { { t.test_name } -> std::convertible_to<std::string>; };

    template<HasTestIdentifier TestCase>
    std::ostream &operator<<(std::ostream &os, const TestCase &test_case) {
        return os << test_case.test_name;
    }
}
