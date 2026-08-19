#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstImportDirectiveSpanTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstImportDirectiveSpanTest, DirectiveWithoutValue)
    {
        std::string code = "#debug";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .directives = {
                IsDirective("debug")
                    .with_name_span(1, 2, 1, 7)
                    .with_span(1, 1, 1, 7)
            }
        });
    }

    TEST_F(AstImportDirectiveSpanTest, DirectiveWithStringValue)
    {
        std::string code = "#version = \"1.0\"";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .directives = {
                IsDirective("version", IsString("\"1.0\"").with_span(1, 12, 1, 17))
                    .with_name_span(1, 2, 1, 9)
                    .with_span(1, 1, 1, 17)
            }
        });
    }

    TEST_F(AstImportDirectiveSpanTest, DirectiveWithNumericValue)
    {
        std::string code = "#opt_level = 3";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .directives = {
                IsDirective("opt_level", IsNumber("3").with_span(1, 14, 1, 15))
                    .with_name_span(1, 2, 1, 11)
                    .with_span(1, 1, 1, 15)
            }
        });
    }

    TEST_F(AstImportDirectiveSpanTest, ImportStatementSimple)
    {
        std::string code = "import \"std/math\"";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .imports = {
                IsImport("\"std/math\"", {})
                    .with_name_span(1, 8, 1, 18)
                    .with_span(1, 1, 1, 18)
            }
        });
    }

    TEST_F(AstImportDirectiveSpanTest, ImportStatementWithSingleModifierAndArg)
    {
        std::string code =
                "@deprecated(since: \"2.0\")\n"
                "import \"core/math\"";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .imports = {
                IsImport("\"core/math\"", {
                    ModifierSpec{
                        .name = "deprecated",
                        .args = {
                            ArgSpec{
                                .label = "since",
                                .value_v = IsString("\"2.0\"").with_span(1, 20, 1, 25)
                            }.with_name_span(1, 13, 1, 18)
                             .with_span(1, 13, 1, 25)
                        }
                    }.with_name_span(1, 2, 1, 12)
                     .with_span(1, 1, 1, 26)
                }).with_name_span(2, 8, 2, 19)
                  .with_span(1, 1, 2, 19)
            }
        });
    }

    TEST_F(AstImportDirectiveSpanTest, ImportStatementWithMultipleModifiersAndArgs)
    {
        std::string code =
                "@export\n"
                "@cached(ttlSec: 60)\n"
                "import \"core/engine\"";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .imports = {
                IsImport("\"core/engine\"", {
                    ModifierSpec{.name = "export"}
                        .with_name_span(1, 2, 1, 8)
                        .with_span(1, 1, 1, 8),
                    ModifierSpec{
                        .name = "cached",
                        .args = {
                            ArgSpec{
                                .label = "ttlSec",
                                .value_v = IsNumber("60").with_span(2, 17, 2, 19)
                            }.with_name_span(2, 9, 2, 15)
                             .with_span(2, 9, 2, 19)
                        }
                    }.with_name_span(2, 2, 2, 8)
                     .with_span(2, 1, 2, 20)
                }).with_name_span(3, 8, 3, 21)
                  .with_span(1, 1, 3, 21)
            }
        });
    }
}
