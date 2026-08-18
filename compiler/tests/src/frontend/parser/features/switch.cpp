#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ExprVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "SimpleSwitchWithOneCase",
                .code = "switch (x) { case A -> 1 }",
                .verifier = IsSwitch(
                    IsIdentifier("x"),
                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "EmptySwitch",
                .code = "switch (x) { }",
                .verifier = IsSwitch(IsIdentifier("x"), std::vector<SwitchCaseSpec>{})
            });

            reg({
                .name = "MultipleLabelsInOneCase",
                .code = "switch (x) { case A, B, C -> 1 }",
                .verifier = IsSwitch(
                    IsIdentifier("x"),
                    SwitchCaseSpec{.labels = {"A", "B", "C"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "MultipleCases",
                .code = "switch (x) { case A -> 1 case B -> 2 }",
                .verifier = IsSwitch(
                    IsIdentifier("x"),
                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")},
                    SwitchCaseSpec{.labels = {"B"}, .result_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DefaultCaseOnly",
                .code = "switch (x) { default -> 0 }",
                .verifier = IsSwitch(
                    IsIdentifier("x"),
                    std::vector<SwitchCaseSpec>{},
                    IsNumber("0")
                )
            });

            reg({
                .name = "FullSwitchWithDefault",
                .code = "switch (x) { case A -> 1 case B -> 2 default -> 0 }",
                .verifier = IsSwitch(
                    IsIdentifier("x"),
                    std::vector<SwitchCaseSpec>{
                        SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")},
                        SwitchCaseSpec{.labels = {"B"}, .result_v = IsNumber("2")}
                    },
                    {},
                    IsNumber("0")
                )
            });

            reg({
                .name = "NestedSwitchExpressions",
                .code = "switch (x) { case A -> switch (y) { case B -> 1 default -> 2 } default -> 3 }",
                .verifier = IsSwitch(
                    IsIdentifier("x"),
                    std::vector<SwitchCaseSpec>{
                        SwitchCaseSpec{
                            .labels = {"A"},
                            .result_v = IsSwitch(
                                IsIdentifier("y"),
                                std::vector<SwitchCaseSpec>{
                                    SwitchCaseSpec{.labels = {"B"}, .result_v = IsNumber("1")}
                                },
                                {},
                                IsNumber("2")
                            )
                        }
                    },
                    {},
                    IsNumber("3")
                )
            });

            reg({
                .name = "SwitchMultilineFormatting",
                .code = "switch (state) {\n"
                "  case Active, Pending -> \"ok\"\n"
                "  case Error -> \"fail\"\n"
                "  default -> \"unknown\"\n"
                "}",
                .verifier = IsSwitch(
                    IsIdentifier("state"),
                    std::vector<SwitchCaseSpec>{
                        SwitchCaseSpec{.labels = {"Active", "Pending"}, .result_v = IsString("\"ok\"")},
                        SwitchCaseSpec{.labels = {"Error"}, .result_v = IsString("\"fail\"")}
                    },
                    IsString("\"unknown\"")
                )
            });

            return true;
        }();
    }
}
