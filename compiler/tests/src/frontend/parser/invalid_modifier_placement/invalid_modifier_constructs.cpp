#include "invalid_modifier_constructs.h"
#include "frontend/parser/helpers/context_names.h"

namespace valuascript::compiler::test
{
    const std::vector<InvalidModifierConstructCase>& InvalidModifierConstructRegistry::cases()
    {
        static std::vector<InvalidModifierConstructCase> registry = []()
        {
            std::vector<InvalidModifierConstructCase> list;

            list.push_back({
                .name = "Directive",
                .code = "#value = 1",
                .verifier = IsDirective("value", IsNumber("1")),
                .type = InjectableType::Directive
            });

            list.push_back({
                .name = "Reassignment",
                .code = "x = 1",
                .verifier = IsReassignment(IsIdentifier("x"), IsNumber("1")),
                .type = InjectableType::StrongStatement
            });

            list.push_back({
                .name = "ExprStmt",
                .code = "foo()",
                .verifier = IsExprStmt(IsCall(IsIdentifier("foo"), {})),
                .type = InjectableType::StrongStatement
            });

            list.push_back({
                .name = "ExpressionWithInvalidModifier",
                .code = "42",
                .verifier = IsNumber("42"),
                .type = InjectableType::Expression
            });

            list.push_back({
                .name = "TypeAnnotationWithInvalidModifier",
                .code = "Int",
                .verifier = IsType("Int"),
                .type = InjectableType::TypeAnnotation
            });

            return list;
        }();
        return registry;
    }

    void InvalidModifierConstructRegistry::add(InvalidModifierConstructCase spec)
    {
        const_cast<std::vector<InvalidModifierConstructCase>&>(cases()).push_back(std::move(spec));
    }
}
