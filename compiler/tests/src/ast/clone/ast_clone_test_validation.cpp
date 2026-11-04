#include "ast_clone_test_helper.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/metadata/ast_node_registry.h"

namespace valuascript::compiler::test
{
    template <typename T>
    concept ValidCloneTestTarget =
        ConcreteAstNode<T> &&
        HasAstNodeSchema<T> &&
        HasValidCloneSampleFactory<T>;

    template <ValidCloneTestTarget T>
    consteval bool verify_clone_test_target()
    {
        return true;
    }

    template <typename Tuple>
    struct AstCloneCoverageValidator;

    template <typename... Types>
    struct AstCloneCoverageValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_clone_test_target<Types>() && ...);
        }
    };

    static_assert(AstCloneCoverageValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node type in AllAstNodeTypes must have an implemented AstCloneSampleFactory and AstNodeSchema specialization");
}
