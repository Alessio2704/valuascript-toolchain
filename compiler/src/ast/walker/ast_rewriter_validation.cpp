#include "ast/walker/ast_rewriter.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename Rewriter, typename Tuple>
    struct ValidateRewriterCompleteness;

    template <typename Rewriter, typename... Types>
    struct ValidateRewriterCompleteness<Rewriter, std::tuple<Types...>>
    {
        static constexpr bool value = (
            (
                (std::derived_from<Types, Expression> && requires(Rewriter& r, std::unique_ptr<Types> n)
                {
                    { r.rewrite(std::move(n)) } -> std::same_as<ExprPtr>;
                }) ||
                (std::derived_from<Types, Statement> && requires(Rewriter& r, std::unique_ptr<Types> n)
                {
                    { r.rewrite(std::move(n)) } -> std::same_as<StmtPtr>;
                }) ||
                (std::derived_from<Types, TypeAnnotation> && requires(Rewriter& r, std::unique_ptr<Types> n)
                {
                    { r.rewrite(std::move(n)) } -> std::same_as<TypeAnnPtr>;
                }) ||
                (requires(Rewriter& r, std::unique_ptr<Types> n)
                {
                    { r.rewrite(std::move(n)) } -> std::same_as<std::unique_ptr<Types>>;
                }) ||
                (requires(Rewriter& r, Types n)
                {
                    { r.rewrite(std::move(n)) } -> std::same_as<Types>;
                })
            ) && ...
        );
    };

    static_assert(ValidateRewriterCompleteness<AstRewriter, AllAstNodeTypes>::value,
                  "AstRewriter is missing a rewrite() overload for one or more registered AST node types in AllAstNodeTypes")
    ;

    template <typename Rewriter, typename Tuple>
    struct ValidateCategoryRewriterCompleteness;

    template <typename Rewriter, typename... Categories>
    struct ValidateCategoryRewriterCompleteness<Rewriter, std::tuple<Categories...>>
    {
        static constexpr bool value = (
            (requires(Rewriter& r, std::unique_ptr<Categories> c)
            {
                { r.rewrite(std::move(c)) } -> std::same_as<std::unique_ptr<Categories>>;
            }) && ...
        );
    };

    static_assert(ValidateCategoryRewriterCompleteness<AstRewriter, AstCategoryTypes>::value,
                  "AstRewriter is missing a rewrite() overload for one or more registered categories in AstCategoryTypes");
}
