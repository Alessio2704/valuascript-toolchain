#include "ast_validity.h"
#include "ast_node_registry.h"

namespace valuascript::compiler
{
    template <ValidatableAstNode T>
    consteval bool verify_validatable_ast_node()
    {
        return true;
    }

    template <typename Tuple>
    struct AstValidityCompletenessValidator;

    template <typename... Types>
    struct AstValidityCompletenessValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_validatable_ast_node<Types>() && ...);
        }
    };

    static_assert(AstValidityCompletenessValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node in AllAstNodeTypes must implement [[nodiscard]] bool is_valid() const noexcept");
}
