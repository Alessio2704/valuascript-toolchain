#include "ast/walker/ast_walker.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename Walker, typename T>
    concept WalkerSupportsNode = requires(Walker& w, MaybeConst<std::is_const_v<std::remove_pointer_t<typename Walker::NodePtr>>, T>& n)
    {
        { w.enter(n) } -> std::same_as<TraversalAction>;
        { w.leave(n) } -> std::same_as<void>;
        { w.walk_children(n) } -> std::same_as<void>;
    };

    template <typename Walker, typename T>
    requires WalkerSupportsNode<Walker, T>
    consteval bool verify_walker_node()
    {
        return true;
    }

    template <typename Walker, typename Tuple>
    struct AstWalkerNodeValidator;

    template <typename Walker, typename... Types>
    struct AstWalkerNodeValidator<Walker, std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_walker_node<Walker, Types>() && ...);
        }
    };

    static_assert(AstWalkerNodeValidator<ConstAstWalker, AllAstNodeTypes>::validate(),
                  "ConstAstWalker is missing enter / leave / walk_children hooks for one or more registered AST node types in AllAstNodeTypes");
    static_assert(AstWalkerNodeValidator<AstWalker, AllAstNodeTypes>::validate(),
                  "AstWalker is missing enter / leave / walk_children hooks for one or more registered AST node types in AllAstNodeTypes");

    template <typename Walker, typename Category>
    concept WalkerSupportsCategory = requires(Walker& w, MaybeConst<std::is_const_v<std::remove_pointer_t<typename Walker::NodePtr>>, Category>& c)
    {
        { w.enter(c) } -> std::same_as<TraversalAction>;
        { w.leave(c) } -> std::same_as<void>;
    };

    template <typename Walker, typename Category>
    requires WalkerSupportsCategory<Walker, Category>
    consteval bool verify_walker_category()
    {
        return true;
    }

    template <typename Walker, typename Tuple>
    struct AstWalkerCategoryValidator;

    template <typename Walker, typename... Categories>
    struct AstWalkerCategoryValidator<Walker, std::tuple<Categories...>>
    {
        static consteval bool validate()
        {
            return (verify_walker_category<Walker, Categories>() && ...);
        }
    };

    static_assert(AstWalkerCategoryValidator<ConstAstWalker, AstCategoryTypes>::validate(),
                  "ConstAstWalker is missing enter / leave category hooks for one or more registered category types in AstCategoryTypes");
    static_assert(AstWalkerCategoryValidator<AstWalker, AstCategoryTypes>::validate(),
                  "AstWalker is missing enter / leave category hooks for one or more registered category types in AstCategoryTypes");
}
