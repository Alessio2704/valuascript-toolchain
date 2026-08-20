#include "ast/walker/ast_rewriter.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename Rewriter, typename T>
    struct CheckRewriterSingleType
    {
        static constexpr bool value = [] {
            if constexpr (requires(Rewriter& r, std::unique_ptr<T> n) { r.rewrite(std::move(n)); })
            {
                using RetType = decltype(std::declval<Rewriter&>().rewrite(std::declval<std::unique_ptr<T>>()));
                return std::is_same_v<RetType, ExprPtr> ||
                       std::is_same_v<RetType, StmtPtr> ||
                       std::is_same_v<RetType, TypeAnnPtr> ||
                       std::is_same_v<RetType, std::unique_ptr<T>>;
            }
            else if constexpr (requires(Rewriter& r, T n) { r.rewrite(std::move(n)); })
            {
                using RetType = decltype(std::declval<Rewriter&>().rewrite(std::declval<T>()));
                return std::is_same_v<RetType, T>;
            }
            else
            {
                return false;
            }
        }();
    };

    template <typename Rewriter, typename Tuple>
    struct ValidateRewriterCompleteness;

    template <typename Rewriter, typename... Types>
    struct ValidateRewriterCompleteness<Rewriter, std::tuple<Types...>>
    {
        static constexpr bool value = (CheckRewriterSingleType<Rewriter, Types>::value && ...);
    };

    static_assert(ValidateRewriterCompleteness<AstRewriter, AllAstNodeTypes>::value,
                  "AstRewriter is missing a rewrite() overload for one or more registered AST node types in AllAstNodeTypes");

    template <typename Rewriter, typename Category>
    struct CheckRewriterSingleCategory
    {
        static constexpr bool value = [] {
            if constexpr (requires(Rewriter& r, std::unique_ptr<Category> c) { r.rewrite(std::move(c)); })
            {
                return std::is_same_v<decltype(std::declval<Rewriter&>().rewrite(std::declval<std::unique_ptr<Category>>())), std::unique_ptr<Category>>;
            }
            return false;
        }();
    };

    template <typename Rewriter, typename Tuple>
    struct ValidateCategoryRewriterCompleteness;

    template <typename Rewriter, typename... Categories>
    struct ValidateCategoryRewriterCompleteness<Rewriter, std::tuple<Categories...>>
    {
        static constexpr bool value = (CheckRewriterSingleCategory<Rewriter, Categories>::value && ...);
    };

    static_assert(ValidateCategoryRewriterCompleteness<AstRewriter, AstCategoryTypes>::value,
                  "AstRewriter is missing a rewrite() overload for one or more registered categories in AstCategoryTypes");
}
