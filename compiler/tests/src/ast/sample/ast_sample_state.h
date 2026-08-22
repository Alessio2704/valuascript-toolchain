#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <cstddef>
#include <utility>

#include "token/source_span.h"
#include "ast/core/ast_core.h"

namespace valuascript::compiler::test
{
    class AstSampleState
    {
    public:
        static size_t& get_id_ref() noexcept
        {
            static size_t id = 100;
            return id;
        }

        static size_t next_id() noexcept
        {
            return ++get_id_ref();
        }

        static void reset(size_t start = 100) noexcept
        {
            get_id_ref() = start;
        }

        static SourceSpan make_span(int depth = 0)
        {
            size_t id = next_id();
            return SourceSpan{
                .line_start = (id % 1000) + 1,
                .column_start = (static_cast<size_t>(depth) * 4) + 1,
                .line_end = (id % 1000) + 1 + (depth > 0 ? 0 : 1),
                .column_end = (static_cast<size_t>(depth) * 4) + 13,
                .file_path = std::make_shared<const std::string>("sample_source.vs"),
                .start_offset = id * 20,
                .length = 12
            };
        }

        static NodeName make_name(std::string_view prefix = "sample_id", int depth = 0)
        {
            size_t id = next_id();
            std::string str = std::string(prefix) + "_" + std::to_string(id);
            return NodeName(std::move(str), make_span(depth));
        }
    };

    inline void reset_sample_generator_state(size_t start = 100) noexcept
    {
        AstSampleState::reset(start);
    }

    inline SourceSpan sample_span(int depth = 0)
    {
        return AstSampleState::make_span(depth);
    }

    inline NodeName sample_name(std::string_view prefix = "node", int depth = 0)
    {
        return AstSampleState::make_name(prefix, depth);
    }
}
