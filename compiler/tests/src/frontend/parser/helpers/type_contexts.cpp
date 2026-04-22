#include "context_registry.h"
#include "spec_adder.h"

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_type_contexts()
    {
        using namespace SpecAdder;
        return {
            {
                "assignment_target", NestingLevel::BlockLevel, {InjectableType::TypeAnnotation}, "let ctx_assign: ",
                " = 1\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_assign", get_v<TypeVerifier>(v)}}, IsNumber("1")));
                }
            },
            {
                "multi_assignment_target_1", NestingLevel::BlockLevel, {InjectableType::TypeAnnotation}, "let ctx_m1: ",
                ", ctx_m2 = 1\n",
                [&](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_m1", get_v<TypeVerifier>(v)}, {"ctx_m2"}}, IsNumber("1")));
                }
            },
            {
                "multi_assignment_target_2", NestingLevel::BlockLevel, {InjectableType::TypeAnnotation},
                "let ctx_m1, ctx_m2: ",
                " = 1\n",
                [&](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_m1"}, {"ctx_m2", get_v<TypeVerifier>(v)}}, IsNumber("1")));
                }
            },
            {
                "typealias_target", NestingLevel::TopLevel, {InjectableType::TypeAnnotation}, "typealias ctx_alias = ",
                "\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsTypeAlias("ctx_alias", {}, get_v<TypeVerifier>(v)));
                }
            },

            {
                "function_parameter", NestingLevel::TopLevel, {InjectableType::TypeAnnotation},
                "func ctx_func_param(p: ", ") -> void {}\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsFunctionDef("ctx_func_param", {}, {ParamSpec{"p", {}, get_v<TypeVerifier>(v)}},
                                         {IsType("void")}));
                }
            },

            {
                "function_return", NestingLevel::TopLevel, {InjectableType::TypeAnnotation}, "func ctx_func_ret() -> ",
                " {}\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsFunctionDef("ctx_func_ret", {}, {}, {get_v<TypeVerifier>(v)}));
                }
            },

            {
                "function_multi_return", NestingLevel::TopLevel, {InjectableType::TypeAnnotation},
                "func ctx_func_multi_ret() -> ", ", int {}\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsFunctionDef("ctx_func_multi_ret", {}, {}, {get_v<TypeVerifier>(v), IsType("int")}));
                }
            },

            {
                "struct_field", NestingLevel::TopLevel, {InjectableType::TypeAnnotation}, "struct ctx_struct { f: ",
                " }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsStructDef("ctx_struct", {}, {FieldSpec{"f", {}, get_v<TypeVerifier>(v)}}));
                }
            },

            {
                "enum_underlying_type", NestingLevel::TopLevel, {InjectableType::TypeAnnotation}, "enum ctx_enum: ",
                " { A }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsEnumDef("ctx_enum", {}, get_v<TypeVerifier>(v), {{"A"}}));
                }
            }
        };
    }
}
