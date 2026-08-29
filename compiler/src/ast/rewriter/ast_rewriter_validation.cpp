#include "ast_rewriter.h"
#include "ast/metadata/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename Rewriter, typename T>
    concept RewriterSupportsNode = requires(Rewriter& r, std::unique_ptr<T> ptr_node, T val_node)
    {
        { r.rewrite(std::move(ptr_node)) };
    } || requires(Rewriter& r, T val_node)
    {
        { r.rewrite(std::move(val_node)) } -> std::same_as<T>;
    };

    template <typename Rewriter, typename T>
    requires RewriterSupportsNode<Rewriter, T>
    consteval bool verify_rewriter_node()
    {
        return true;
    }

    template <typename Rewriter, typename Tuple>
    struct AstRewriterNodeValidator;

    template <typename Rewriter, typename... Types>
    struct AstRewriterNodeValidator<Rewriter, std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_rewriter_node<Rewriter, Types>() && ...);
        }
    };

    static_assert(AstRewriterNodeValidator<AstRewriter, AllAstNodeTypes>::validate(),
                  "AstRewriter is missing a rewrite() overload for one or more registered AST node types in AllAstNodeTypes");

    template <typename Rewriter, typename Category>
    concept RewriterSupportsCategory = requires(Rewriter& r, std::unique_ptr<Category> cat)
    {
        { r.rewrite(std::move(cat)) } -> std::same_as<std::unique_ptr<Category>>;
    };

    template <typename Rewriter, typename Category>
    requires RewriterSupportsCategory<Rewriter, Category>
    consteval bool verify_rewriter_category()
    {
        return true;
    }

    template <typename Rewriter, typename Tuple>
    struct AstRewriterCategoryValidator;

    template <typename Rewriter, typename... Categories>
    struct AstRewriterCategoryValidator<Rewriter, std::tuple<Categories...>>
    {
        static consteval bool validate()
        {
            return (verify_rewriter_category<Rewriter, Categories>() && ...);
        }
    };

    static_assert(AstRewriterCategoryValidator<AstRewriter, AstCategoryTypes>::validate(),
                  "AstRewriter is missing a rewrite() overload for one or more registered categories in AstCategoryTypes");
}
