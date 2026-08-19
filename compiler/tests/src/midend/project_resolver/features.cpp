#include <gtest/gtest.h>
#include "project_resolver_test_base.h"

namespace valuascript::compiler::test
{
    class ProjectResolverHappyPathTest : public ProjectResolverTestBase
    {
    };

    TEST_F(ProjectResolverHappyPathTest, LinearDependencyChain)
    {
        std::string c_path = CreateFile("c.vs", "let c_val = 30");
        std::string b_path = CreateFile("b.vs", "import \"c.vs\"\nlet b_val = 20");
        std::string a_path = CreateFile("a.vs", "import \"b.vs\"\nlet a_val = 10");

        ExpectResolverSuccess(a_path, {c_path, b_path, a_path});
    }

    TEST_F(ProjectResolverHappyPathTest, DiamondDependencyGraph)
    {
        /*
        //     A
        //    / \
        //   B   C
        //    \ /
        //     D
        */
        std::string d_path = CreateFile("d.vs", "let d = 4");
        CreateFile("c.vs", "import \"d.vs\"\nlet c = 3");
        CreateFile("b.vs", "import \"d.vs\"\nlet b = 2");
        std::string a_path = CreateFile("a.vs", "import \"b.vs\"\nimport \"c.vs\"\nlet a = 1");

        ExpectResolverGraph(a_path, 4, d_path, a_path);
    }

    TEST_F(ProjectResolverHappyPathTest, RelativePathsAcrossDirectories)
    {
        std::string const_path = CreateFile("constants.vs", "let pi = 3.14");
        std::string math_path = CreateFile("utils/math.vs", "import \"../constants.vs\"\nlet double_pi = pi * 2");
        std::string main_path = CreateFile("main.vs", "import \"utils/math.vs\"\nlet area = double_pi");

        ExpectResolverSuccess(main_path, {const_path, math_path, main_path});
    }

    TEST_F(ProjectResolverHappyPathTest, ComplexRelativePathBacktracking)
    {
        std::string base_path = CreateFile("core/base.vs", "let version = 1.0");
        std::string feature_path = CreateFile("features/deep/feature.vs", "import \"../../core/base.vs\"\nlet f = version");
        std::string main_path = CreateFile("main.vs", "import \"features/deep/feature.vs\"\nlet m = f");

        ExpectResolverSuccess(main_path, {base_path, feature_path, main_path});
    }

    TEST_F(ProjectResolverHappyPathTest, NormalizesRedundantPathsToSameModule)
    {
        std::string utils_path = CreateFile("utils.vs", "let u = 10");
        std::string main_path = CreateFile("main.vs", "import \"utils.vs\"\nimport \"./utils.vs\"\nlet m = u");

        ExpectResolverSuccess(main_path, {utils_path, main_path});
    }

    TEST_F(ProjectResolverHappyPathTest, MassiveStarTopology)
    {
        std::string core_path = CreateFile("core.vs", "let core_val = 100");

        std::string main_content;
        for (int i = 1; i <= 5; ++i)
        {
            std::string mod_name = "mod" + std::to_string(i) + ".vs";
            std::string mod_content = "import \"core.vs\"\nlet m" + std::to_string(i) + " = core_val";

            CreateFile(mod_name, mod_content);
            main_content += "import \"" + mod_name + "\"\n";
        }

        std::string main_path = CreateFile("main.vs", main_content);

        ExpectResolverGraph(main_path, 7, core_path, main_path);
    }

    TEST_F(ProjectResolverHappyPathTest, ReverseDependencyGraphTracking)
    {
        /*
        //     A
        //    / \
        //   B   C
        //    \ /
        //     D
        */
        std::string d_path = CreateFile("d.vs", "let d = 4");
        std::string c_path = CreateFile("c.vs", "import \"d.vs\"\nlet c = 3");
        std::string b_path = CreateFile("b.vs", "import \"d.vs\"\nlet b = 2");
        std::string a_path = CreateFile("a.vs", "import \"b.vs\"\nimport \"c.vs\"\nlet a = 1");

        ResolvedProjectArtifact project = RunResolver(a_path);

        // d.vs is imported by b.vs and c.vs
        ASSERT_TRUE(project.reverse_imports.contains(d_path));
        EXPECT_EQ(project.reverse_imports[d_path].size(), 2);
        EXPECT_NE(std::find(project.reverse_imports[d_path].begin(), project.reverse_imports[d_path].end(), b_path),
                  project.reverse_imports[d_path].end());
        EXPECT_NE(std::find(project.reverse_imports[d_path].begin(), project.reverse_imports[d_path].end(), c_path),
                  project.reverse_imports[d_path].end());

        // b.vs is imported by a.vs
        ASSERT_TRUE(project.reverse_imports.contains(b_path));
        EXPECT_EQ(project.reverse_imports[b_path].size(), 1);
        EXPECT_EQ(project.reverse_imports[b_path][0], a_path);

        // c.vs is imported by a.vs
        ASSERT_TRUE(project.reverse_imports.contains(c_path));
        EXPECT_EQ(project.reverse_imports[c_path].size(), 1);
        EXPECT_EQ(project.reverse_imports[c_path][0], a_path);

        // a.vs is entry and has no incoming reverse imports
        EXPECT_FALSE(project.reverse_imports.contains(a_path));
    }

    TEST_F(ProjectResolverHappyPathTest, CanonicalTargetPathOnImportStatements)
    {
        std::string math_path = CreateFile("utils/math.vs", "let pi = 3.14");
        std::string main_path = CreateFile("main.vs", "import \"utils/math.vs\"\nlet r = pi");

        ResolvedProjectArtifact project = RunResolver(main_path);

        ASSERT_TRUE(project.modules.contains(main_path));
        const auto& main_ast = project.modules[main_path];
        ASSERT_EQ(main_ast->import_statements.size(), 1);

        const auto& import_stmt = main_ast->import_statements[0];
        ASSERT_TRUE(import_stmt->resolved_canonical_path.has_value());
        EXPECT_EQ(import_stmt->resolved_canonical_path.value(), math_path);
    }
}
