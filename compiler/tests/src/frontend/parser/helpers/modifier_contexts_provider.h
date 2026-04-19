#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct ModifierContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, const std::vector<ModifierSpec>&)> add_to_spec;
    };

    class ModifierContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& mod_code)
        {
            std::string res = templ;
            size_t pos = 0;
            while ((pos = res.find("{mods}", pos)) != std::string::npos)
            {
                res.replace(pos, 6, mod_code);
                pos += mod_code.length();
            }
            return res;
        }

        static std::vector<ModifierContext> get_all()
        {
            return {
                {
                    "assignment",
                    "{mods} let ctx_assign = 1\n", [](ProgramSpec& s, const std::vector<ModifierSpec>& m)
                    {
                        s.execution_steps.push_back(IsAssignment(m, {{"ctx_assign"}}, IsNumber("1")));
                    }
                },
                {
                    "function_definition",
                    "{mods} func ctx_func() -> void {}\n", [](ProgramSpec& s, const std::vector<ModifierSpec>& m)
                    {
                        s.functions.push_back(IsFunctionDef("ctx_func", m, {}, {IsType("void")}));
                    }
                },
                {
                    "function_parameter",
                    "func ctx_param({mods} p: int) -> void {}\n", [](ProgramSpec& s, const std::vector<ModifierSpec>& m)
                    {
                        s.functions.push_back(IsFunctionDef("ctx_param", {}, {ParamSpec{"p", m, IsType("int")}},
                                                            {IsType("void")}));
                    }
                },
                {
                    "struct_definition",
                    "{mods} struct ctx_struct {}\n", [](ProgramSpec& s, const std::vector<ModifierSpec>& m)
                    {
                        s.structs.push_back(IsStructDef("ctx_struct", m, {}));
                    }
                },
                {
                    "struct_field",
                    "struct ctx_s_field { {mods} f: int }\n", [](ProgramSpec& s, const std::vector<ModifierSpec>& m)
                    {
                        s.structs.push_back(IsStructDef("ctx_s_field", {}, {FieldSpec{"f", m, IsType("int")}}));
                    }
                },
                {
                    "enum_definition",
                    "{mods} enum ctx_enum: int { A = 1 }\n", [](ProgramSpec& s, const std::vector<ModifierSpec>& m)
                    {
                        s.enums.push_back(IsEnumDef("ctx_enum", m, IsType("int"),
                                                    {EnumCaseSpec{"A", {}, IsNumber("1")}}));
                    }
                },
                {
                    "enum_case",
                    "enum ctx_e_case: int { {mods} A = 1 }\n", [](ProgramSpec& s, const std::vector<ModifierSpec>& m)
                    {
                        s.enums.push_back(IsEnumDef("ctx_e_case", {}, IsType("int"),
                                                    {EnumCaseSpec{"A", m, IsNumber("1")}}));
                    }
                },
                {
                    "typealias_definition",
                    "{mods} typealias ctx_alias = int\n", [](ProgramSpec& s, const std::vector<ModifierSpec>& m)
                    {
                        s.type_aliases.push_back(IsTypeAlias("ctx_alias", m, IsType("int")));
                    }
                },
                {
                    "dict_item",
                    "let ctx_dict = { {mods} k: 1 }\n", [](ProgramSpec& s, const std::vector<ModifierSpec>& m)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_dict"}}, IsDict({DictItemSpec{"k", m, IsNumber("1")}})));
                    }
                }
            };
        }
    };
}
