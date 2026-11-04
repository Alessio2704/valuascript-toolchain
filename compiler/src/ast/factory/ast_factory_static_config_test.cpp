#include <tuple>
#include <type_traits>
#include <string>

#include "ast/factory/ast_factory.h"
#include "ast/factory/ast_factory_config.h"
#include "ast/factory/ast_factory_config_validator.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler
{
    static_assert(AllNodeConfigsValidator<AllAstNodeTypes>::validate(),
                  "All AST NodeConfigs must be 100% compliant with AstNodeSchema!");

    template <typename Tuple>
    struct AllNodesConfigValidityCheck;

    template <typename... NodeTypes>
    struct AllNodesConfigValidityCheck<std::tuple<NodeTypes...>>
    {
        static consteval bool validate()
        {
            return (ValidNodeConfig<NodeTypes, NodeConfig<NodeTypes>> && ...);
        }
    };

    static_assert(AllNodesConfigValidityCheck<AllAstNodeTypes>::validate(),
                  "Every NodeConfig<T> in AllAstNodeTypes must satisfy ValidNodeConfig<T, NodeConfig<T>>!");

    struct EmptyConfig
    {
        static constexpr auto members = std::make_tuple();
    };

    template <typename Tuple>
    struct ConfigurableNodesRejectEmptyConfig;

    template <typename... NodeTypes>
    struct ConfigurableNodesRejectEmptyConfig<std::tuple<NodeTypes...>>
    {
        static consteval bool validate()
        {
            return ((std::tuple_size_v<decltype(NodeConfig<NodeTypes>::members)> > 0
                        ? !ValidNodeConfig<NodeTypes, EmptyConfig>
                        : true) && ...);
        }
    };

    static_assert(ConfigurableNodesRejectEmptyConfig<AllAstNodeTypes>::validate(),
                  "Nodes with configurable fields must reject an empty configuration!");

    struct PrimitiveFieldConfig
    {
        int fake_primitive = 0;
        static constexpr auto members = std::make_tuple(&PrimitiveFieldConfig::fake_primitive);
    };

    template <typename Tuple>
    struct ConfigurableNodesRejectPrimitiveFieldConfig;

    template <typename... NodeTypes>
    struct ConfigurableNodesRejectPrimitiveFieldConfig<std::tuple<NodeTypes...>>
    {
        static consteval bool validate()
        {
            return ((std::tuple_size_v<decltype(NodeConfig<NodeTypes>::members)> > 0
                        ? !ValidNodeConfig<NodeTypes, PrimitiveFieldConfig>
                        : true) && ...);
        }
    };

    static_assert(ConfigurableNodesRejectPrimitiveFieldConfig<AllAstNodeTypes>::validate(),
                  "All configurable AST nodes must reject raw primitive integer fields without valid descriptor wrappers!");

    struct BadConfigWithWrongOpPointer
    {
        OperatorFieldConfig<&UnaryExpression::op> op{.op = TokenType::Minus};
        static constexpr auto members = std::make_tuple(
            &BadConfigWithWrongOpPointer::op
        );
    };

    static_assert(!ValidNodeConfig<BinaryExpression, BadConfigWithWrongOpPointer>,
                  "Wrong member pointer on OperatorFieldConfig must fail compile-time validation!");

    struct BadConfigWithWrongFieldTypeForDocstring
    {
        VectorFieldConfig<&FunctionDefinition::modifiers> modifiers{.count = 2};
        NameFieldConfig<&FunctionDefinition::name> name{.prefix = "func_name"};
        VectorFieldConfig<&FunctionDefinition::parameters> parameters{.count = 3};
        VectorFieldConfig<&FunctionDefinition::return_types> return_types{.count = 2};
        VectorFieldConfig<&FunctionDefinition::body> body{.count = 3};
        NameFieldConfig<&FunctionDefinition::name> docstring{.prefix = "bad_doc"};
        static constexpr auto members = std::make_tuple(
            &BadConfigWithWrongFieldTypeForDocstring::modifiers,
            &BadConfigWithWrongFieldTypeForDocstring::name,
            &BadConfigWithWrongFieldTypeForDocstring::parameters,
            &BadConfigWithWrongFieldTypeForDocstring::return_types,
            &BadConfigWithWrongFieldTypeForDocstring::body,
            &BadConfigWithWrongFieldTypeForDocstring::docstring
        );
    };

    static_assert(!ValidNodeConfig<FunctionDefinition, BadConfigWithWrongFieldTypeForDocstring>,
                  "Wrong descriptor for docstring must fail compile-time validation!");

    struct BadConfigWithWrongOptionalFieldType
    {
        VectorFieldConfig<&FunctionParameter::modifiers> modifiers{.count = 2};
        NameFieldConfig<&FunctionParameter::name> name{.prefix = "param"};
        OptionalFieldConfig<&FunctionParameter::default_value, std::string> default_value{.value = "invalid"};
        static constexpr auto members = std::make_tuple(
            &BadConfigWithWrongOptionalFieldType::modifiers,
            &BadConfigWithWrongOptionalFieldType::name,
            &BadConfigWithWrongOptionalFieldType::default_value
        );
    };

    static_assert(!ValidNodeConfig<FunctionParameter, BadConfigWithWrongOptionalFieldType>,
                  "Wrong value_type for OptionalFieldConfig must fail compile-time validation!");

    template <typename CategoryTuple, typename AllTuple>
    struct CategoryConceptDisjointnessValidator
    {
        template <typename PredicateFunc>
        static consteval bool validate(PredicateFunc pred)
        {
            return []<typename... AllNodes>(PredicateFunc p, std::tuple<AllNodes...>*)
            {
                return (((valuascript::shared::tuple_contains_type_v<AllNodes, CategoryTuple>)
                            ? p.template operator()<AllNodes>()
                            : !p.template operator()<AllNodes>()) && ...);
            }(pred, static_cast<AllTuple*>(nullptr));
        }
    };

    static_assert(CategoryConceptDisjointnessValidator<AllExpressionNodeTypes, AllAstNodeTypes>::validate([]<typename T>() { return IsExpressionNode<T>; }),
                  "IsExpressionNode trait must evaluate true for AllExpressionNodeTypes and false for all other AST node types!");

    static_assert(CategoryConceptDisjointnessValidator<AllStatementNodeTypes, AllAstNodeTypes>::validate([]<typename T>() { return IsStatementNode<T>; }),
                  "IsStatementNode trait must evaluate true for AllStatementNodeTypes and false for all other AST node types!");

    static_assert(CategoryConceptDisjointnessValidator<AllTypeAnnotationNodeTypes, AllAstNodeTypes>::validate([]<typename T>() { return IsTypeAnnotationNode<T>; }),
                  "IsTypeAnnotationNode trait must evaluate true for AllTypeAnnotationNodeTypes and false for all other AST node types!");

    static_assert(CategoryConceptDisjointnessValidator<AllReassignmentTargetNodeTypes, AllAstNodeTypes>::validate([]<typename T>() { return IsReassignmentTarget<T>; }),
                  "IsReassignmentTarget trait must evaluate true for AllReassignmentTargetNodeTypes and false for all other AST node types!");

    template <typename KindWrapper, typename TypesTuple>
    struct KindMappingValidator;

    template <typename KindWrapper, typename... Types>
    struct KindMappingValidator<KindWrapper, std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return ((KindWrapper::template from<Types>() == Types::KIND) && ...);
        }
    };

    static_assert(KindMappingValidator<ExpressionKind, AllExpressionNodeTypes>::validate(),
                  "ExpressionKind::from<T>() must match T::KIND for every type in AllExpressionNodeTypes!");

    static_assert(KindMappingValidator<StatementKind, AllStatementNodeTypes>::validate(),
                  "StatementKind::from<T>() must match T::KIND for every type in AllStatementNodeTypes!");

    static_assert(KindMappingValidator<TypeAnnotationKind, AllTypeAnnotationNodeTypes>::validate(),
                  "TypeAnnotationKind::from<T>() must match T::KIND for every type in AllTypeAnnotationNodeTypes!");

    static_assert(KindMappingValidator<ReassignmentTargetKind, AllReassignmentTargetNodeTypes>::validate(),
                  "ReassignmentTargetKind::from<T>() must match T::KIND for every type in AllReassignmentTargetNodeTypes!");
}
