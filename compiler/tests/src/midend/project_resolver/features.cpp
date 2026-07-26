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
}
