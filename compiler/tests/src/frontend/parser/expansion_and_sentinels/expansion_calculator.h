#pragma once

#include "frontend/parser/helpers/injectable_type.h"
#include <vector>
#include <string_view>

namespace valuascript::compiler::test
{
    class ExpansionCalculator
    {
    public:
        static size_t compute_expected_expansions(InjectableType start_type,
                                                  const std::vector<std::string_view>& skip_contexts = {});
    };
}
