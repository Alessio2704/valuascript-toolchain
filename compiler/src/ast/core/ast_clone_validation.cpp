#include "ast/core/ast_clone.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename Tuple>
    struct ValidateCloneCompleteness;

    template <typename... Types>
    struct ValidateCloneCompleteness<std::tuple<Types...>>
    {
        static constexpr bool value = (
            (
                (requires(const Types* n)
                {
                    { clone_node(n) } -> std::same_as<std::unique_ptr<Types>>;
                }) ||
                (requires(const Types& n)
                {
                    { clone_node(n) } -> std::same_as<Types>;
                })
            ) && ...
        );
    };

    static_assert(ValidateCloneCompleteness<AllAstNodeTypes>::value,
                  "All registered AST node types in AllAstNodeTypes must be cloneable via clone_node()");
}
