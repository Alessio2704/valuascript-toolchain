#pragma once
#include "../helpers/node_matchers.h"
#include <vector>
#include <string>
#include <functional>

namespace valuascript::compiler::test {
    struct LanguageConstruct {
        std::string name;
        std::string source;
        std::function<void(ProgramSpec &)> add_to_spec;
    };

    class LanguageRegistry {
    public:
        static const std::vector<LanguageConstruct> &all() {
            static const std::vector<LanguageConstruct> constructs = {
                {
                    "import", "import \"rec_mod\"\n", [](ProgramSpec &s) {
                        s.imports.push_back(IsImport("\"rec_mod\""));
                    }
                },
                {
                    "dir_no_val", "#rec_dir_none\n", [](ProgramSpec &s) {
                        s.directives.push_back(IsDirective("rec_dir_none", nullptr));
                    }
                },
                {
                    "dir_val_eq", "#rec_dir_eq = 1\n", [](ProgramSpec &s) {
                        s.directives.push_back(IsDirective("rec_dir_eq", IsNumber("1")));
                    }
                },
                {
                    "dir_val_no_eq", "#rec_dir_no_eq 2\n", [](ProgramSpec &s) {
                        s.directives.push_back(IsDirective("rec_dir_no_eq", IsNumber("2")));
                    }
                },
                {
                    "func", "@rec_mod func rec_f(p: int) -> int { return p }\n", [](ProgramSpec &s) {
                        s.functions.push_back(IsFunctionDef("rec_f", {{"rec_mod"}}, {ParamSpec{"p", {}, IsType("int")}},
                                                            {IsType("int")}, {IsReturn({IsIdentifier("p")})},
                                                            std::nullopt));
                    }
                },
                {
                    "struct", "@rec_mod struct RecS { f: int }\n", [](ProgramSpec &s) {
                        s.structs.push_back(IsStructDef("RecS", {{"rec_mod"}}, {{"f", {}, IsType("int")}}));
                    }
                },
                {
                    "enum", "@rec_mod enum RecE: int { C = 1 }\n", [](ProgramSpec &s) {
                        s.enums.push_back(IsEnumDef("RecE", {{"rec_mod"}}, IsType("int"), {{"C", {}, IsNumber("1")}}));
                    }
                },
                {
                    "alias", "@rec_mod typealias RecA = int\n", [](ProgramSpec &s) {
                        s.type_aliases.push_back(IsTypeAlias("RecA", {{"rec_mod"}}, IsType("int")));
                    }
                },
                {
                    "let_single", "@rec_mod let rec_x = 100\n", [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsAssignment({{"rec_mod"}}, {{"rec_x"}}, IsNumber("100")));
                    }
                },
                {
                    "let_multi", "let rec_m1, rec_m2: int = 50\n", [](ProgramSpec &s) {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"rec_m1"}, {"rec_m2", IsType("int")}}, IsNumber("50")));
                    }
                },
                {
                    "reassign_id", "rec_x = 200\n", [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsReassignment(IsIdentifier("rec_x"), IsNumber("200")));
                    }
                },
                {
                    "reassign_dot", "rec_obj.prop = 300\n", [](ProgramSpec &s) {
                        s.execution_steps.push_back(
                            IsReassignment(IsDot(IsIdentifier("rec_obj"), "prop"), IsNumber("300")));
                    }
                },
                {
                    "reassign_bracket", "rec_arr[0] = 400\n", [](ProgramSpec &s) {
                        s.execution_steps.push_back(
                            IsReassignment(IsBracket(IsIdentifier("rec_arr"), IsNumber("0")), IsNumber("400")));
                    }
                },
                {
                    "reassign_self_dot", "self.prop = 500\n", [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsReassignment(IsDot(IsSelf(), "prop"), IsNumber("500")));
                    }
                },
                {
                    "reassign_self_bracket", "self[1] = 600\n", [](ProgramSpec &s) {
                        s.execution_steps.
                                push_back(IsReassignment(IsBracket(IsSelf(), IsNumber("1")), IsNumber("600")));
                    }
                },
                {
                    "call_id", "rec_f()\n", [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsExprStmt(IsCall(IsIdentifier("rec_f"), {})));
                    }
                },
                {
                    "call_method", "rec_obj.method(a: 1)\n", [](ProgramSpec &s) {
                        s.execution_steps.push_back(
                            IsExprStmt(IsCall(IsDot(IsIdentifier("rec_obj"), "method"), {{"a", IsNumber("1")}})));
                    }
                },
                {
                    "call_self", "self.method()\n", [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsExprStmt(IsCall(IsDot(IsSelf(), "method"), {})));
                    }
                }
            };
            return constructs;
        }
    };
}
