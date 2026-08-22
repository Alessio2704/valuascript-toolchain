#include "ast_sample_factory.h"
#include "ast/core/ast_node_schema.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler::test
{
    template <typename T>
    concept SampleGeneratableNode =
        ConcreteAstNode<T> &&
        HasAstNodeSchema<T> &&
        requires(int depth) {
            { AstSampleFactory<T>::create(depth) };
        };

    template <SampleGeneratableNode T>
    consteval bool verify_sample_generatable()
    {
        return true;
    }

    template <typename Tuple>
    struct AstSampleCompletenessValidator;

    template <typename... Types>
    struct AstSampleCompletenessValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_sample_generatable<Types>() && ...);
        }
    };

    static_assert(AstSampleCompletenessValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node type in AllAstNodeTypes must have an implemented AstSampleFactory specialization");
}
