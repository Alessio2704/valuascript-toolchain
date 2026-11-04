#pragma once

#include <gtest/gtest.h>
#include <source_location>
#include "ast/ast.h"

namespace valuascript::compiler::test
{
    template <AstElement T>
    T* ExpectNode(AstNode* node, std::source_location loc = std::source_location::current())
    {
        if (!node) [[unlikely]]
        {
            ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                << "Expected AST node of type [" << get_ast_node_name<T>()
                << "], but got [nullptr].";
            return nullptr;
        }
        T* casted = ast_cast<T>(node);
        if (!casted) [[unlikely]]
        {
            ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                << "Expected AST type [" << get_ast_node_name<T>()
                << "], but got [" << get_ast_node_name(*node) << "].";
            return nullptr;
        }
        return casted;
    }

    inline void ExpectNullNode(AstNode* node)
    {
        EXPECT_EQ(node, nullptr) << "Expected node to be null, but it was populated.";
    }

    struct NullVerifier
    {
        void operator()(AstNode* node) const { ExpectNullNode(node); }
        void operator()(TypeAnnotation* node) const { ExpectNullNode(node); }
    };

    inline NullVerifier IsNull() { return NullVerifier{}; }
    inline NullVerifier IsNullType() { return NullVerifier{}; }
}
