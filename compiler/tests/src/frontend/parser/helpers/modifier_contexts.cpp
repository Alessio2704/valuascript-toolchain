#include "context_registry.h"
#include "spec_adder.h"

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_modifier_contexts()
    {
        return {
            {
                "assignment", NestingLevel::BlockLevel, {InjectableType::Modifier}, " ", " let ctx_assign = 1\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    SpecAdder::add(s, IsAssignment(SpecAdder::get_v<ModifierVerifier>(v), {{"ctx_assign"}}, IsNumber("1")));
                }
            },

            {
                "function_definition", NestingLevel::TopLevel, {InjectableType::Modifier}, " ",
                " func ctx_func() -> void {}\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    SpecAdder::add(s, IsFunctionDef("ctx_func", SpecAdder::get_v<ModifierVerifier>(v), {}, {IsType("void")}));
                }
            },

            {
                "function_parameter", NestingLevel::TopLevel, {InjectableType::Modifier}, "func ctx_param(",
                " p: int) -> void {}\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    SpecAdder::add(s, IsFunctionDef("ctx_param", {}, {ParamSpec{"p", SpecAdder::get_v<ModifierVerifier>(v), IsType("int")}},
                                         {IsType("void")}));
                }
            },

            {
                "struct_definition", NestingLevel::TopLevel, {InjectableType::Modifier}, " ", " struct ctx_struct {}\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    SpecAdder::add(s, IsStructDef("ctx_struct", SpecAdder::get_v<ModifierVerifier>(v), {}));
                }
            },

            {
                "struct_field", NestingLevel::TopLevel, {InjectableType::Modifier}, "struct ctx_s_field { ",
                " f: int }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    SpecAdder::add(s, IsStructDef("ctx_s_field", {}, {FieldSpec{"f", SpecAdder::get_v<ModifierVerifier>(v), IsType("int")}}));
                }
            },

            {
                "enum_definition", NestingLevel::TopLevel, {InjectableType::Modifier}, " ",
                " enum ctx_enum: int { A = 1 }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    SpecAdder::add(s, IsEnumDef("ctx_enum", SpecAdder::get_v<ModifierVerifier>(v), IsType("int"),
                                     {{"A", {}, IsNumber("1")}}));
                }
            },

            {
                "enum_case", NestingLevel::TopLevel, {InjectableType::Modifier}, "enum ctx_e_case: int { ",
                " A = 1 }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    SpecAdder::add(s, IsEnumDef("ctx_e_case", {}, IsType("int"),
                                     {{"A", SpecAdder::get_v<ModifierVerifier>(v), IsNumber("1")}}));
                }
            },

            {
                "typealias_definition", NestingLevel::TopLevel, {InjectableType::Modifier}, " ",
                " typealias ctx_alias = int\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    SpecAdder::add(s, IsTypeAlias("ctx_alias", SpecAdder::get_v<ModifierVerifier>(v), IsType("int")));
                }
            },

            {
                "dict_item", NestingLevel::BlockLevel, {InjectableType::Modifier}, "let ctx_dict = { ", " k: 1 }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    SpecAdder::add(s, IsAssignment({}, {{"ctx_dict"}}, IsDict({
                                            DictItemSpec{"k", SpecAdder::get_v<ModifierVerifier>(v), IsNumber("1")}
                                        })));
                }
            }
        };
    }
}
