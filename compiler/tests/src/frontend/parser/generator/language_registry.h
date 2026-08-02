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
                    .name = "import", .source = "import \"rec_mod\"\n", .add_to_spec = [](ProgramSpec &s) {
                        s.imports.push_back(IsImport("\"rec_mod\""));
                    }
                },
                {
                    .name = "dir_no_val", .source = "#rec_dir_none\n", .add_to_spec = [](ProgramSpec &s) {
                        s.directives.push_back(IsDirective("rec_dir_none", nullptr));
                    }
                },
                {
                    .name = "dir_val_eq", .source = "#rec_dir_eq = 1\n", .add_to_spec = [](ProgramSpec &s) {
                        s.directives.push_back(IsDirective("rec_dir_eq", IsNumber("1")));
                    }
                },
                {
                    .name = "dir_val_no_eq", .source = "#rec_dir_no_eq 2\n", .add_to_spec = [](ProgramSpec &s) {
                        s.directives.push_back(IsDirective("rec_dir_no_eq", IsNumber("2")));
                    }
                },
                {
                    .name = "func", .source = "@rec_mod func rec_f(p: int) -> int { return p }\n", .add_to_spec = [](ProgramSpec &s) {
                        s.functions.push_back(IsFunctionDef("rec_f", {{"rec_mod"}}, {ParamSpec{.name = "p", .type_v = IsType("int")}},
                                                            {IsType("int")}, {IsReturn({IsIdentifier("p")})},
                                                            std::nullopt));
                    }
                },
                {
                    .name = "struct", .source = "@rec_mod struct RecS { f: int }\n", .add_to_spec = [](ProgramSpec &s) {
                        s.structs.push_back(IsStructDef("RecS", {{"rec_mod"}}, {{"f", {}, IsType("int")}}));
                    }
                },
                {
                    .name = "enum", .source = "@rec_mod enum RecE: int { C = 1 }\n", .add_to_spec = [](ProgramSpec &s) {
                        s.enums.push_back(IsEnumDef("RecE", {{"rec_mod"}}, IsType("int"), {{"C", {}, IsNumber("1")}}));
                    }
                },
                {
                    .name = "alias", .source = "@rec_mod typealias RecA = int\n", .add_to_spec = [](ProgramSpec &s) {
                        s.type_aliases.push_back(IsTypeAlias("RecA", {{"rec_mod"}}, IsType("int")));
                    }
                },
                {
                    .name = "let_single", .source = "@rec_mod let rec_x = 100\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsAssignment({{"rec_mod"}}, {{"rec_x"}}, IsNumber("100")));
                    }
                },
                {
                    .name = "let_multi", .source = "let rec_m1, rec_m2: int = 50\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"rec_m1"}, {"rec_m2", IsType("int")}}, IsNumber("50")));
                    }
                },
                {
                    .name = "reassign_id", .source = "rec_x = 200\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsReassignment(IsIdentifier("rec_x"), IsNumber("200")));
                    }
                },
                {
                    .name = "reassign_dot", .source = "rec_obj.prop = 300\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.push_back(
                            IsReassignment(IsDot(IsIdentifier("rec_obj"), "prop"), IsNumber("300")));
                    }
                },
                {
                    .name = "reassign_bracket", .source = "rec_arr[0] = 400\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.push_back(
                            IsReassignment(IsBracket(IsIdentifier("rec_arr"), IsNumber("0")), IsNumber("400")));
                    }
                },
                {
                    .name = "reassign_self_dot", .source = "self.prop = 500\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsReassignment(IsDot(IsSelf(), "prop"), IsNumber("500")));
                    }
                },
                {
                    .name = "reassign_self_bracket", .source = "self[1] = 600\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.
                                push_back(IsReassignment(IsBracket(IsSelf(), IsNumber("1")), IsNumber("600")));
                    }
                },
                {
                    .name = "call_id", .source = "rec_f()\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsExprStmt(IsCall(IsIdentifier("rec_f"), {})));
                    }
                },
                {
                    .name = "call_method", .source = "rec_obj.method(a: 1)\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.push_back(
                            IsExprStmt(IsCall(IsDot(IsIdentifier("rec_obj"), "method"), {{"a", IsNumber("1")}})));
                    }
                },
                {
                    .name = "call_self", .source = "self.method()\n", .add_to_spec = [](ProgramSpec &s) {
                        s.execution_steps.push_back(IsExprStmt(IsCall(IsDot(IsSelf(), "method"), {})));
                    }
                }
            };
            return constructs;
        }
    };
}
