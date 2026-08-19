#pragma once

#include <vector>
#include <concepts>
#include <functional>
#include <optional>
#include "ast_walker.h"

namespace valuascript::compiler
{
    // -------------------------------------------------------------------------
    // 1. Generic Lambda AST Walker
    // -------------------------------------------------------------------------
    template <typename Func>
        requires std::invocable<Func, const AstNode&>
    void walk_ast(const Program& program, Func&& callback)
    {
        class LambdaWalker : public ConstAstWalker
        {
            Func callback_;
        public:
            explicit LambdaWalker(Func cb) : callback_(std::forward<Func>(cb)) {}

            TraversalAction enter_node(const AstNode& node) override
            {
                if constexpr (std::is_same_v<std::invoke_result_t<Func, const AstNode&>, TraversalAction>)
                {
                    return callback_(node);
                }
                else
                {
                    callback_(node);
                    return TraversalAction::Continue;
                }
            }
        };

        LambdaWalker walker(std::forward<Func>(callback));
        walker.walk(program);
    }

    // -------------------------------------------------------------------------
    // 2. Type-Filtered For-Each Walker
    // -------------------------------------------------------------------------
    template <typename TargetNode, typename Func>
        requires std::invocable<Func, const TargetNode&>
    void for_each_node(const Program& program, Func&& callback)
    {
        walk_ast(program, [&callback](const AstNode& node) {
            if (auto* typed = ast_cast<const TargetNode>(&node))
            {
                callback(*typed);
            }
        });
    }

    // -------------------------------------------------------------------------
    // 3. Find First Node Matching Predicate
    // -------------------------------------------------------------------------
    template <typename Predicate>
        requires std::predicate<Predicate, const AstNode&>
    [[nodiscard]] const AstNode* find_first_node(const Program& program, Predicate&& predicate)
    {
        class SearchWalker : public ConstAstWalker
        {
            Predicate predicate_;
            const AstNode* found_ = nullptr;
        public:
            explicit SearchWalker(Predicate pred) : predicate_(std::forward<Predicate>(pred)) {}

            TraversalAction enter_node(const AstNode& node) override
            {
                if (predicate_(node))
                {
                    found_ = &node;
                    return TraversalAction::Stop;
                }
                return TraversalAction::Continue;
            }

            [[nodiscard]] const AstNode* result() const noexcept { return found_; }
        };

        SearchWalker walker(std::forward<Predicate>(predicate));
        walker.walk(program);
        return walker.result();
    }

    template <typename TargetNode>
    [[nodiscard]] const TargetNode* find_first_node_of_type(const Program& program)
    {
        class TypedSearchWalker : public ConstAstWalker
        {
            const TargetNode* found_ = nullptr;
        public:
            TraversalAction enter_node(const AstNode& node) override
            {
                if (auto* typed = ast_cast<const TargetNode>(&node))
                {
                    found_ = typed;
                    return TraversalAction::Stop;
                }
                return TraversalAction::Continue;
            }

            [[nodiscard]] const TargetNode* result() const noexcept { return found_; }
        };

        TypedSearchWalker walker;
        walker.walk(program);
        return walker.result();
    }

    template <typename TargetNode, typename Predicate>
        requires std::predicate<Predicate, const TargetNode&>
    [[nodiscard]] const TargetNode* find_first_node_of_type(const Program& program, Predicate&& predicate)
    {
        class TypedSearchWalker : public ConstAstWalker
        {
            Predicate predicate_;
            const TargetNode* found_ = nullptr;
        public:
            explicit TypedSearchWalker(Predicate pred) : predicate_(std::forward<Predicate>(pred)) {}

            TraversalAction enter_node(const AstNode& node) override
            {
                if (auto* typed = ast_cast<const TargetNode>(&node))
                {
                    if (predicate_(*typed))
                    {
                        found_ = typed;
                        return TraversalAction::Stop;
                    }
                }
                return TraversalAction::Continue;
            }

            [[nodiscard]] const TargetNode* result() const noexcept { return found_; }
        };

        TypedSearchWalker walker(std::forward<Predicate>(predicate));
        walker.walk(program);
        return walker.result();
    }

    // -------------------------------------------------------------------------
    // 4. Find Node at Cursor Position (with Subtree Pruning for LSP)
    // -------------------------------------------------------------------------
    [[nodiscard]] inline const AstNode* find_node_at_position(const Program& program, size_t line, size_t column)
    {
        class PositionFinder : public ConstAstWalker
        {
            size_t line_;
            size_t col_;
            const AstNode* innermost_ = nullptr;

        public:
            PositionFinder(size_t l, size_t c) : line_(l), col_(c) {}

            TraversalAction enter_node(const AstNode& node) override
            {
                if (node.kind == AstKind::Program)
                {
                    return TraversalAction::Continue;
                }

                // If position is not within this node's span, PRUNE the subtree!
                if (!node.span.contains(line_, col_))
                {
                    return TraversalAction::SkipChildren;
                }

                // Update innermost match
                innermost_ = &node;
                return TraversalAction::Continue;
            }

            [[nodiscard]] const AstNode* result() const noexcept { return innermost_; }
        };

        PositionFinder finder(line, column);
        finder.walk(program);
        return finder.result();
    }

    [[nodiscard]] inline std::vector<const AstNode*> find_ancestor_path_at_position(const Program& program, size_t line, size_t column)
    {
        class PathFinder : public ConstAstWalker
        {
            size_t line_;
            size_t col_;
            std::vector<const AstNode*> deepest_path_;

        public:
            PathFinder(size_t l, size_t c) : line_(l), col_(c) {}

            TraversalAction enter_node(const AstNode& node) override
            {
                if (node.kind == AstKind::Program)
                {
                    deepest_path_ = {&node};
                    return TraversalAction::Continue;
                }

                if (!node.span.contains(line_, col_))
                {
                    return TraversalAction::SkipChildren;
                }

                deepest_path_.assign(ancestor_stack().begin(), ancestor_stack().end());
                return TraversalAction::Continue;
            }

            [[nodiscard]] std::vector<const AstNode*> result() && { return std::move(deepest_path_); }
            [[nodiscard]] const std::vector<const AstNode*>& result() const & { return deepest_path_; }
        };

        PathFinder finder(line, column);
        finder.walk(program);
        return std::move(finder).result();
    }

    // -------------------------------------------------------------------------
    // 5. Collect All Nodes Matching Predicate
    // -------------------------------------------------------------------------
    template <typename TargetNode>
    [[nodiscard]] std::vector<const TargetNode*> collect_nodes(const Program& program)
    {
        std::vector<const TargetNode*> results;
        walk_ast(program, [&results](const AstNode& node) {
            if (auto* typed = ast_cast<const TargetNode>(&node))
            {
                results.push_back(typed);
            }
        });
        return results;
    }

    template <typename TargetNode, typename Predicate>
        requires std::predicate<Predicate, const TargetNode&>
    [[nodiscard]] std::vector<const TargetNode*> collect_nodes(const Program& program, Predicate&& predicate)
    {
        std::vector<const TargetNode*> results;
        walk_ast(program, [&results, &predicate](const AstNode& node) {
            if (auto* typed = ast_cast<const TargetNode>(&node))
            {
                if (predicate(*typed))
                {
                    results.push_back(typed);
                }
            }
        });
        return results;
    }
}
