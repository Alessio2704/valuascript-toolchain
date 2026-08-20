#include "ast/walker/ast_walker.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename Walker, typename T>
    struct CheckWalkerSingleType
    {
        static constexpr bool value = requires(Walker& w, MaybeConst<std::is_const_v<std::remove_pointer_t<typename Walker::NodePtr>>, T>& n)
        {
            { w.enter(n) } -> std::same_as<TraversalAction>;
            { w.leave(n) } -> std::same_as<void>;
            { w.walk_children(n) } -> std::same_as<void>;
        };
    };

    template <typename Walker, typename Tuple>
    struct ValidateWalkerCompleteness;

    template <typename Walker, typename... Types>
    struct ValidateWalkerCompleteness<Walker, std::tuple<Types...>>
    {
        static constexpr bool value = (CheckWalkerSingleType<Walker, Types>::value && ...);
    };

    static_assert(ValidateWalkerCompleteness<ConstAstWalker, AllAstNodeTypes>::value,
                  "ConstAstWalker is missing enter / leave / walk_children hooks for one or more registered AST node types");
    static_assert(ValidateWalkerCompleteness<AstWalker, AllAstNodeTypes>::value,
                  "AstWalker is missing enter / leave / walk_children hooks for one or more registered AST node types");

    template <typename Walker, typename Category>
    struct CheckWalkerSingleCategory
    {
        static constexpr bool value = requires(Walker& w, MaybeConst<std::is_const_v<std::remove_pointer_t<typename Walker::NodePtr>>, Category>& c)
        {
            { w.enter(c) } -> std::same_as<TraversalAction>;
            { w.leave(c) } -> std::same_as<void>;
        };
    };

    template <typename Walker, typename Tuple>
    struct ValidateCategoryWalkerCompleteness;

    template <typename Walker, typename... Categories>
    struct ValidateCategoryWalkerCompleteness<Walker, std::tuple<Categories...>>
    {
        static constexpr bool value = (CheckWalkerSingleCategory<Walker, Categories>::value && ...);
    };

    static_assert(ValidateCategoryWalkerCompleteness<ConstAstWalker, AstCategoryTypes>::value,
                  "ConstAstWalker is missing enter / leave category hooks for one or more registered category types in AstCategoryTypes");
    static_assert(ValidateCategoryWalkerCompleteness<AstWalker, AstCategoryTypes>::value,
                  "AstWalker is missing enter / leave category hooks for one or more registered category types in AstCategoryTypes");
}
