#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "ast/factory/ast_factory_state.h"

namespace valuascript::compiler::test
{
    TEST(AstFactoryStateTest, IdIncrementAndReset)
    {
        AstFactoryState::reset(200);
        EXPECT_EQ(AstFactoryState::next_id(), 201);
        EXPECT_EQ(AstFactoryState::next_id(), 202);
        EXPECT_EQ(AstFactoryState::next_id(), 203);

        reset_factory_state(500);
        EXPECT_EQ(AstFactoryState::next_id(), 501);
        EXPECT_EQ(AstFactoryState::next_id(), 502);
    }

    TEST(AstFactoryStateTest, FactorySpanProperties)
    {
        reset_factory_state(100);

        auto span0 = factory_span(0);
        EXPECT_EQ(span0.line_start, 102);
        EXPECT_EQ(span0.line_end, 103);
        EXPECT_EQ(span0.column_start, 1);
        EXPECT_EQ(span0.column_end, 13);
        EXPECT_EQ(span0.start_offset, 2020);
        EXPECT_EQ(span0.length, 12);
        ASSERT_NE(span0.file_path, nullptr);
        EXPECT_EQ(*span0.file_path, "sample_source.vs");

        auto span2 = factory_span(2);
        EXPECT_EQ(span2.line_start, 103);
        EXPECT_EQ(span2.line_end, 103);
        EXPECT_EQ(span2.column_start, 9);
        EXPECT_EQ(span2.column_end, 21);
        EXPECT_EQ(span2.start_offset, 2040);
        EXPECT_EQ(span2.length, 12);
    }

    TEST(AstFactoryStateTest, FactoryNameProperties)
    {
        reset_factory_state(300);

        auto name0 = factory_name("custom", 1);
        EXPECT_EQ(name0.value, "custom_301");
        EXPECT_EQ(name0.span.line_start, 303);
        EXPECT_EQ(name0.span.column_start, 5);
        EXPECT_EQ(name0.span.column_end, 17);
    }

    TEST(AstFactoryStateTest, DeterministicReplayAcrossResets)
    {
        reset_factory_state(777);
        std::vector<SourceSpan> spans1;
        std::vector<NodeName> names1;

        for (int i = 0; i < 10; ++i)
        {
            spans1.push_back(factory_span(i % 4));
            names1.push_back(factory_name("node", i % 4));
        }

        reset_factory_state(777);
        std::vector<SourceSpan> spans2;
        std::vector<NodeName> names2;

        for (int i = 0; i < 10; ++i)
        {
            spans2.push_back(factory_span(i % 4));
            names2.push_back(factory_name("node", i % 4));
        }

        ASSERT_EQ(spans1.size(), spans2.size());
        for (size_t i = 0; i < spans1.size(); ++i)
        {
            EXPECT_EQ(spans1[i], spans2[i]);
        }

        ASSERT_EQ(names1.size(), names2.size());
        for (size_t i = 0; i < names1.size(); ++i)
        {
            EXPECT_EQ(names1[i].value, names2[i].value);
            EXPECT_EQ(names1[i].span, names2[i].span);
        }

        reset_factory_state(888);
        auto diff_span = factory_span(0);
        EXPECT_NE(spans1[0], diff_span);
    }
}
