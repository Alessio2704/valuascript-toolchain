#include "ast/factory/ast_factory.h"
#include "ast/factory/ast_factory_config.h"
#include "ast/factory/ast_factory_config_validator.h"

namespace valuascript::compiler
{
    static_assert(AllNodeConfigsValidator<AllAstNodeTypes>::validate(),
                  "All AST NodeConfigs must be 100% compliant with AstNodeSchema!");

    struct BadConfigWithPrimitiveInt
    {
        int modifiers = 2;
        NameFieldConfig<&FunctionParameter::name> name{.prefix = "param"};
        static constexpr auto members = std::make_tuple(
            &BadConfigWithPrimitiveInt::modifiers,
            &BadConfigWithPrimitiveInt::name
        );
    };

    static_assert(!ValidNodeConfig<FunctionParameter, BadConfigWithPrimitiveInt>,
                  "Primitive int instead of VectorFieldConfig must fail compile-time validation!");

    struct BadConfigWithWrongDescriptor
    {
        NameFieldConfig<&FunctionParameter::name> modifiers{.prefix = "mod"};
        NameFieldConfig<&FunctionParameter::name> name{.prefix = "param"};
        static constexpr auto members = std::make_tuple(
            &BadConfigWithWrongDescriptor::modifiers,
            &BadConfigWithWrongDescriptor::name
        );
    };

    static_assert(!ValidNodeConfig<FunctionParameter, BadConfigWithWrongDescriptor>,
                  "NameFieldConfig on a vector field must fail compile-time validation!");

    struct BadConfigWithSwappedPointer
    {
        VectorFieldConfig<&FunctionParameter::modifiers> modifiers{.count = 2};
        NameFieldConfig<&IdentifierAccess::name> name{.prefix = "param"};
        static constexpr auto members = std::make_tuple(
            &BadConfigWithSwappedPointer::modifiers,
            &BadConfigWithSwappedPointer::name
        );
    };

    static_assert(!ValidNodeConfig<FunctionParameter, BadConfigWithSwappedPointer>,
                  "Wrong member pointer on a field must fail compile-time validation!");

    struct BadConfigWithMissingField
    {
        NameFieldConfig<&FunctionParameter::name> name{.prefix = "param"};
        static constexpr auto members = std::make_tuple(
            &BadConfigWithMissingField::name
        );
    };

    static_assert(!ValidNodeConfig<FunctionParameter, BadConfigWithMissingField>,
                  "Missing a required schema member must fail compile-time validation!");

    struct BadConfigMissingDefaultValue
    {
        VectorFieldConfig<&FunctionParameter::modifiers> modifiers{.count = 2};
        NameFieldConfig<&FunctionParameter::name> name{.prefix = "param"};
        static constexpr auto members = std::make_tuple(
            &BadConfigMissingDefaultValue::modifiers,
            &BadConfigMissingDefaultValue::name
        );
    };

    static_assert(!ValidNodeConfig<FunctionParameter, BadConfigMissingDefaultValue>,
                  "Missing default_value in FunctionParameter config must fail compile-time validation!");

    struct BadConfigWithWrongOpPointer
    {
        OperatorFieldConfig<&UnaryExpression::op> op{.op = TokenType::Minus};
        static constexpr auto members = std::make_tuple(
            &BadConfigWithWrongOpPointer::op
        );
    };

    static_assert(!ValidNodeConfig<BinaryExpression, BadConfigWithWrongOpPointer>,
                  "Wrong member pointer on OperatorFieldConfig must fail compile-time validation!");

    static_assert(ValidNodeConfig<FunctionParameter, NodeConfig<FunctionParameter>>);
    static_assert(ValidNodeConfig<FunctionDefinition, NodeConfig<FunctionDefinition>>);
    static_assert(ValidNodeConfig<BinaryExpression, NodeConfig<BinaryExpression>>);
    static_assert(ValidNodeConfig<BooleanLiteral, NodeConfig<BooleanLiteral>>);
    static_assert(ValidNodeConfig<EnumCase, NodeConfig<EnumCase>>);
    static_assert(ValidNodeConfig<SwitchExpression, NodeConfig<SwitchExpression>>);
    static_assert(ValidNodeConfig<Program, NodeConfig<Program>>);

    static_assert(IsReassignmentTarget<IdentifierAccess>);
    static_assert(IsReassignmentTarget<DotAccess>);
    static_assert(IsReassignmentTarget<BracketAccess>);
    static_assert(!IsReassignmentTarget<StringLiteral>);
    static_assert(!IsReassignmentTarget<NumberLiteral>);
    static_assert(!IsReassignmentTarget<Assignment>);
    static_assert(!IsReassignmentTarget<FunctionDefinition>);

    static_assert(IsExpressionNode<NumberLiteral>);
    static_assert(IsExpressionNode<StringLiteral>);
    static_assert(!IsExpressionNode<Assignment>);
    static_assert(!IsExpressionNode<TypeAnnotation>);

    static_assert(IsStatementNode<Assignment>);
    static_assert(IsStatementNode<Reassignment>);
    static_assert(IsStatementNode<ExpressionStatement>);
    static_assert(IsStatementNode<ReturnStatement>);
    static_assert(!IsStatementNode<NumberLiteral>);
    static_assert(!IsStatementNode<FunctionDefinition>);

    static_assert(IsTypeAnnotationNode<TypeAnnotation>);
    static_assert(IsTypeAnnotationNode<TupleTypeAnnotation>);
    static_assert(!IsTypeAnnotationNode<NumberLiteral>);
    static_assert(!IsTypeAnnotationNode<Assignment>);

    static_assert(ReassignmentTargetKind::from<IdentifierAccess>() == AstKind::IdentifierAccess);
    static_assert(ReassignmentTargetKind::from<DotAccess>() == AstKind::DotAccess);
    static_assert(ReassignmentTargetKind::from<BracketAccess>() == AstKind::BracketAccess);
    static_assert(ExpressionKind::from<NumberLiteral>() == AstKind::NumberLiteral);
    static_assert(StatementKind::from<Assignment>() == AstKind::Assignment);
    static_assert(TypeAnnotationKind::from<TypeAnnotation>() == AstKind::TypeAnnotation);
}
