#include "ast/clone/ast_clone.h"
#include "ast/metadata/ast_node_registry.h"

namespace valuascript::compiler
{
    template <CloneableAstNode T>
    consteval bool verify_cloneable_ast_node()
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
            return (verify_cloneable_ast_node<Types>() && ...);
        }
    };

    static_assert(AstCloneCompletenessValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node in AllAstNodeTypes must be cloneable via clone_node()");
}
