#include <gtest/gtest.h>
#include "token/source_span.h"

using namespace valuascript::shared;

TEST(SourceSpanTest, MatchesLinesAndColumns)
{
    SourceSpan s1{.line_start = 1, .column_start = 5, .line_end = 1, .column_end = 10};
    SourceSpan s2{.line_start = 1, .column_start = 5, .line_end = 1, .column_end = 10};
    SourceSpan s3{.line_start = 2, .column_start = 5, .line_end = 2, .column_end = 10};

    EXPECT_TRUE(s1.matches_lines_columns(s2));
    EXPECT_FALSE(s1.matches_lines_columns(s3));
}

TEST(SourceSpanTest, MatchesOffsets)
{
    SourceSpan s1{.start_offset = 100, .length = 20};
    SourceSpan s2{.start_offset = 100, .length = 20};
    SourceSpan s3{.start_offset = 100, .length = 25};

    EXPECT_TRUE(s1.matches_offsets(s2));
    EXPECT_FALSE(s1.matches_offsets(s3));
}

TEST(SourceSpanTest, MatchesFilePath)
{
    SourceSpan s1;
    s1 = "main.vs";
    SourceSpan s2;
    s2 = "main.vs";
    SourceSpan s3;
    s3 = "lib.vs";
    SourceSpan s4;

    EXPECT_TRUE(s1.matches_file_path(s2));
    EXPECT_FALSE(s1.matches_file_path(s3));
    EXPECT_FALSE(s1.matches_file_path(s4));
}

TEST(SourceSpanTest, FuzzyMatchesPredicate)
{
    SourceSpan actual{
        .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 8,
        .start_offset = 10, .length = 6
    };
    actual = "test.vs";

    SourceSpan pattern1{
        .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 8
    };
    EXPECT_TRUE(actual.matches(pattern1));

    SourceSpan pattern2{
        .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 8,
        .start_offset = 10, .length = 6
    };
    EXPECT_TRUE(actual.matches(pattern2));

    SourceSpan pattern3{
        .line_start = 2, .column_start = 2, .line_end = 2, .column_end = 8
    };
    EXPECT_FALSE(actual.matches(pattern3));

    SourceSpan pattern4{
        .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 8,
        .start_offset = 15, .length = 6
    };
    EXPECT_FALSE(actual.matches(pattern4));

    SourceSpan pattern5{
        .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 8
    };
    pattern5 = "test.vs";
    EXPECT_TRUE(actual.matches(pattern5));

    SourceSpan pattern6{
        .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 8
    };
    pattern6 = "other.vs";
    EXPECT_FALSE(actual.matches(pattern6));
}
