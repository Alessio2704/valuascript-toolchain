#include "ast/equality/ast_equality.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <ComparableAstNode T>
    consteval bool verify_comparable_ast_node()
    {
        return true;
    }

    template <typename Tuple>
    struct AstEqualityCompletenessValidator;

    template <typename... Types>
    struct AstEqualityCompletenessValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_comparable_ast_node<Types>() && ...);
        }
    };

    static_assert(AstEqualityCompletenessValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node in AllAstNodeTypes must be comparable via ast_equals()");
}
