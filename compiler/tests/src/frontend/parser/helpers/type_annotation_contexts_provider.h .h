#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct TypeAnnotationContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, TypeVerifier)> add_to_spec;
    };

    class TypeAnnotationContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& type_code)
        {
            std::string res = templ;
            size_t pos = 0;
            while ((pos = res.find("{type}", pos)) != std::string::npos)
            {
                res.replace(pos, 6, type_code);
                pos += type_code.length();
            }
            return res;
        }

        static std::vector<TypeAnnotationContext> get_all()
        {
            return {
                {
                    "assignment_target",
                    "let ctx_assign: {type} = 1\n", [](ProgramSpec& s, const TypeVerifier& v)
                    {
                        s.execution_steps.push_back(IsAssignment({}, {{"ctx_assign", v}}, IsNumber("1")));
                    }
                },
                {
                    "typealias_target",
                    "typealias ctx_alias = {type}\n", [](ProgramSpec& s, const TypeVerifier& v)
                    {
                        s.type_aliases.push_back(IsTypeAlias("ctx_alias", {}, v));
                    }
                },
                {
                    "function_parameter",
                    "func ctx_func_param(p: {type}) -> void {}\n", [](ProgramSpec& s, const TypeVerifier& v)
                    {
                        s.functions.push_back(IsFunctionDef("ctx_func_param", {}, {ParamSpec{"p", {}, v}},
                                                            {IsType("void")}));
                    }
                },
                {
                    "function_return",
                    "func ctx_func_ret() -> {type} {}\n", [](ProgramSpec& s, const TypeVerifier& v)
                    {
                        s.functions.push_back(IsFunctionDef("ctx_func_ret", {}, {}, {v}));
                    }
                },
                {
                    "function_multi_return",
                    "func ctx_func_multi_ret() -> {type}, int {}\n", [](ProgramSpec& s, const TypeVerifier& v)
                    {
                        s.functions.push_back(IsFunctionDef("ctx_func_multi_ret", {}, {}, {v, IsType("int")}));
                    }
                },
                {
                    "struct_field",
                    "struct ctx_struct { f: {type} }\n", [](ProgramSpec& s, const TypeVerifier& v)
                    {
                        s.structs.push_back(IsStructDef("ctx_struct", {}, {FieldSpec{"f", {}, v}}));
                    }
                },
                {
                    "enum_underlying_type",
                    "enum ctx_enum: {type} { A }\n", [](ProgramSpec& s, const TypeVerifier& v)
                    {
                        s.enums.push_back(IsEnumDef("ctx_enum", {}, v, {{"A"}}));
                    }
                }
            };
        }
    };
}
