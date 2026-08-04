#pragma once
#include <concepts>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <gtest/gtest.h>

namespace valuascript::compiler::test
{
    template <typename T>
    concept HasMemberTestName = requires(const T& t) {
        { t.test_name } -> std::convertible_to<std::string_view>;
    };

    template <typename T>
    concept HasMemberName = requires(const T& t) {
        { t.name } -> std::convertible_to<std::string_view>;
    };

    template <typename T>
    concept IsStringLike = std::convertible_to<T, std::string_view>;

    template <typename T, typename = void>
    struct TestNameExtractor
    {
        static std::string get(const T& val)
        {
            if constexpr (HasMemberTestName<T>)
            {
                return std::string(val.test_name);
            }
            else if constexpr (HasMemberName<T>)
            {
                return std::string(val.name);
            }
            else if constexpr (IsStringLike<T>)
            {
                return std::string(val);
            }
            else
            {
                static_assert(HasMemberTestName<T> || HasMemberName<T> || IsStringLike<T>,
                              "TestNameExtractor<T> primary template requires T to have .test_name, .name, or be string-like. "
                              "Otherwise, specialize TestNameExtractor<T> for your custom parameter struct.");
            }
        }
    };

    template <typename T>
    concept HasExtractableTestName = !IsStringLike<T> && (HasMemberTestName<T> || HasMemberName<T>);

    template <typename T>
    std::string get_test_name(const T& val)
    {
        return TestNameExtractor<T>::get(val);
    }

    template <HasExtractableTestName T>
    std::ostream& operator<<(std::ostream& os, const T& val)
    {
        return os << TestNameExtractor<T>::get(val);
    }

    template <HasExtractableTestName T>
    void PrintTo(const T& val, std::ostream* os)
    {
        *os << TestNameExtractor<T>::get(val);
    }

    struct TestNameGenerator
    {
        template <typename ParamType>
        std::string operator()(const testing::TestParamInfo<ParamType>& info) const
        {
            return get_test_name(info.param);
        }
    };
}
