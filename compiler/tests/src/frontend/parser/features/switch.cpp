#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("SimpleSwitchWithOneCase",
                "switch (x) { case A -> 1 }",
                IsSwitch(
                    IsIdentifier("x"),
                    {
                        SwitchCaseSpec{{"A"}, IsNumber("1")}
                    }
                ));

            reg("MultipleLabelsInOneCase",
                "switch (x) { case A, B, C -> 1 }",
                IsSwitch(
                    IsIdentifier("x"),
                    {
                        SwitchCaseSpec{{"A", "B", "C"}, IsNumber("1")}
                    }
                ));

            reg("MultipleCases",
                "switch (x) { case A -> 1 case B -> 2 }",
                IsSwitch(
                    IsIdentifier("x"),
                    {
                        SwitchCaseSpec{{"A"}, IsNumber("1")},
                        SwitchCaseSpec{{"B"}, IsNumber("2")}
                    }
                ));

            reg("DefaultCaseOnly",
                "switch (x) { default -> 0 }",
                IsSwitch(
                    IsIdentifier("x"),
                    {},
                    IsNumber("0")
                ));

            reg("FullSwitchWithDefault",
                "switch (x) { case A -> 1 case B -> 2 default -> 0 }",
                IsSwitch(
                    IsIdentifier("x"),
                    {
                        SwitchCaseSpec{{"A"}, IsNumber("1")},
                        SwitchCaseSpec{{"B"}, IsNumber("2")}
                    },
                    IsNumber("0")
                ));

            reg("NestedSwitchExpressions",
                "switch (x) { case A -> switch (y) { case B -> 1 default -> 2 } default -> 3 }",
                IsSwitch(
                    IsIdentifier("x"),
                    {
                        SwitchCaseSpec{
                            {"A"},
                            IsSwitch(
                                IsIdentifier("y"),
                                {
                                    SwitchCaseSpec{{"B"}, IsNumber("1")}
                                },
                                IsNumber("2")
                            )
                        }
                    },
                    IsNumber("3")
                ));

            reg("SwitchMultilineFormatting",
                "switch (state) {\n"
                "  case Active, Pending -> \"ok\"\n"
                "  case Error -> \"fail\"\n"
                "  default -> \"unknown\"\n"
                "}",
                IsSwitch(
                    IsIdentifier("state"),
                    {
                        SwitchCaseSpec{{"Active", "Pending"}, IsString("\"ok\"")},
                        SwitchCaseSpec{{"Error"}, IsString("\"fail\"")}
                    },
                    IsString("\"unknown\"")
                ));

            return true;
        }();
    }
}
