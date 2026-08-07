#pragma once

#include "frontend/parser/helpers/parser_test_base.h"
#include <string>
#include <string_view>
#include <vector>

namespace valuascript::compiler::test
{
    struct TestingFrameworkSample
    {
        InjectableType start_type;
        std::string snippet;
        std::string test_name;
    };

    inline std::vector<TestingFrameworkSample> GetTestingFrameworkSamples()
    {
        return {
            {.start_type = InjectableType::Expression, .snippet = "1", .test_name = "Expression"},
            {.start_type = InjectableType::TypeAnnotation, .snippet = "int", .test_name = "TypeAnnotation"},
            {.start_type = InjectableType::Modifier, .snippet = "@meta", .test_name = "Modifier"},
            {.start_type = InjectableType::StrongStatement, .snippet = "let x = 1", .test_name = "StrongStatement"},
            {.start_type = InjectableType::WeakStatement, .snippet = "return 1", .test_name = "WeakStatement"}
        };
    }

    inline std::vector<InjectableType> GetIntermediateInjectableTypes()
    {
        return get_intermediate_injectable_types();
    }

    class TestingFrameworkTestBase : public ParserTestBase
    {
    protected:
        static bool has_context_segment(const ProcessingItem& item, std::string_view ctx_name)
        {
            return item.has_context(ctx_name);
        }

        static size_t CountTransitions(const ProcessingItem& item)
        {
            return item.transition_count();
        }

        static double get_chi_squared_critical_val_p001(size_t df)
        {
            static const double critical_table[] = {
                0.0,    // df = 0
                6.635,  // df = 1 (k = 2)
                9.210,  // df = 2 (k = 3)
                11.345, // df = 3 (k = 4)
                13.277, // df = 4 (k = 5)
                15.086, // df = 5 (k = 6)
                16.812, // df = 6 (k = 7)
                18.475, // df = 7 (k = 8)
                20.090, // df = 8 (k = 9)
                21.666  // df = 9 (k = 10)
            };
            if (df < std::size(critical_table))
                return critical_table[df];
            return 21.666 + 1.5 * (static_cast<double>(df) - 9.0);
        }
    };
}
