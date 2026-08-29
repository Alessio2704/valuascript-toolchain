#include "ast_validity_test_helper.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/metadata/ast_node_registry.h"

namespace valuascript::compiler::test
{
    template <typename T>
    concept ValidValidityTestTarget =
        ConcreteAstNode<T> &&
        DirectlyValidatable<T> &&
        requires {
            { test_single_node_validity<T>() };
        };

    template <ValidValidityTestTarget T>
    consteval bool verify_validity_test_target()
    {
        return true;
    }

    template <typename Tuple>
    struct AstValidityCoverageValidator;

    template <typename... Types>
    struct AstValidityCoverageValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_validity_test_target<Types>() && ...);
        }
    };

    static_assert(AstValidityCoverageValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node type in AllAstNodeTypes must be covered by the validity test suite");
}
