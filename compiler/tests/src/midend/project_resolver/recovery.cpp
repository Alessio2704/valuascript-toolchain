#include <gtest/gtest.h>
#include "project_resolver_test_base.h"

namespace valuascript::compiler::test
{
    using E = ProjectResolverErrorCode;

    class ProjectResolverRecoveryTest : public ProjectResolverTestBase
    {
    };

    TEST_F(ProjectResolverRecoveryTest, DirectCircularDependency)
    {
        std::string b_path = CreateFile("b.vs", "import \"a.vs\"");
        std::string a_path = CreateFile("a.vs", "import \"b.vs\"");

        ExpectResolverRecovery(a_path, {
            {.code = E::CircularImportDetected, .file_path = b_path, .line = 1, .column = 1, .line_end = 1, .column_end = 14}
        });
    }

    TEST_F(ProjectResolverRecoveryTest, SelfImport)
    {
        std::string a_path = CreateFile("a.vs", "import \"a.vs\"");

        ExpectResolverRecovery(a_path, {
            {.code = E::CircularImportDetected, .file_path = a_path, .line = 1, .column = 1, .line_end = 1, .column_end = 14}
        });
    }

    TEST_F(ProjectResolverRecoveryTest, DeepCircularDependency)
    {
        CreateFile("d.vs", "import \"b.vs\"");
        CreateFile("c.vs", "import \"d.vs\"");
        CreateFile("b.vs", "import \"c.vs\"");
        std::string d_path = (temp_dir / "d.vs").string();
        std::string a_path = CreateFile("a.vs", "import \"b.vs\"");

        std::string canonical_d = std::filesystem::weakly_canonical(d_path).string();

        ExpectResolverRecovery(a_path, {
            {.code = E::CircularImportDetected, .file_path = canonical_d, .line = 1, .column = 1, .line_end = 1, .column_end = 14}
        });
    }

    TEST_F(ProjectResolverRecoveryTest, MissingImportFile)
    {
        std::string a_path = CreateFile("a.vs", "import \"ghost.vs\"");

        ExpectResolverRecovery(a_path, {
            {.code = E::ImportFileNotFound, .file_path = a_path, .line = 1, .column = 1, .line_end = 1, .column_end = 18}
        });
    }

    TEST_F(ProjectResolverRecoveryTest, CollectsCircularAndMissingFileErrors)
    {
        std::string module_a_path = CreateFile("test_module_a.vs", "import \"test_main.vs\"\n");
        std::string main_path = CreateFile("test_main.vs",
            "import \"test_module_a.vs\"\n"
            "import \"test_missing_module.vs\"\n"
        );

        ExpectResolverRecovery(main_path, {
            {.code = E::CircularImportDetected, .file_path = module_a_path, .line = 1, .column = 1, .line_end = 1, .column_end = 22},
            {.code = E::ImportFileNotFound, .file_path = main_path, .line = 2, .column = 1, .line_end = 2, .column_end = 32}
        });
    }
}
