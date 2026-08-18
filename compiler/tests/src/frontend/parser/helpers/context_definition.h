#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "block_context.h"
#include "injectable_type.h"
#include "universal_verifier.h"

namespace valuascript::compiler::test
{
    struct RecoveryBlock;

    struct Context
    {
        std::string_view name = {};
        std::vector<InjectableType> input_types = {};
        InjectableType output_type = InjectableType::Identifier;
        std::string prefix = {};
        std::string suffix = {};
        std::function<UniversalVerifier(const UniversalVerifier&)> transform_verifier = nullptr;

        BlockContext block_context = BlockContext::None;
        std::function<UniversalVerifier(const UniversalVerifier&,
                                        const std::vector<RecoveryBlock>&,
                                        const std::vector<RecoveryBlock>&)> transform_verifier_block = nullptr;

        bool operator_binding_required = true;
        std::function<UniversalVerifier(const std::vector<UniversalVerifier>&)> transform_multi_verifier = nullptr;
    };
}
