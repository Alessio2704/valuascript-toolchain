#include "ast/core/ast_clone.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename T>
    concept CloneableNode = requires(const T* node_ptr, const T& node_ref)
    {
        { clone_node(node_ptr) } -> std::same_as<std::unique_ptr<T>>;
    } || requires(const T& node_ref)
    {
        { clone_node(node_ref) } -> std::same_as<T>;
    };

    template <CloneableNode T>
    consteval bool verify_cloneable_node()
    {
        return true;
    }

    template <typename Tuple>
    struct AstCloneCompletenessValidator;

    template <typename... Types>
    struct AstCloneCompletenessValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_cloneable_node<Types>() && ...);
        }
    };

    static_assert(AstCloneCompletenessValidator<AllAstNodeTypes>::validate(),
                  "All registered AST node types in AllAstNodeTypes must be cloneable via clone_node()");
}
