#include "ast/factory/ast_factory.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/metadata/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename T>
    concept FactoryGeneratableNode =
        ConcreteAstNode<T> &&
        HasAstNodeSchema<T> &&
        requires(int depth) {
            { AstFactory<T>::create(depth) };
        };

    template <FactoryGeneratableNode T>
    consteval bool verify_factory_generatable()
    {
        return true;
    }

    template <typename Tuple>
    struct AstFactoryCompletenessValidator;

    template <typename... Types>
    struct AstFactoryCompletenessValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_factory_generatable<Types>() && ...);
        }
    };

    static_assert(AstFactoryCompletenessValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node type in AllAstNodeTypes must have an implemented AstFactory specialization");
}
