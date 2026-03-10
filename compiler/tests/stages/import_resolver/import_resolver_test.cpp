#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "ImportResolverBase.h"
#include "errors/valuascript_exception.h"
#include "stages/import_resolver/import_resolver_stage.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ImportResolverTest : public ::testing::Test {
protected:
    std::string temp_dir = "test_project_workspace";
    ImportResolverStage resolver;

    void SetUp() override {
        if (std::filesystem::exists(temp_dir)) {
            std::filesystem::remove_all(temp_dir);
        }
        std::filesystem::create_directory(temp_dir);
    }

    void TearDown() override {
        if (std::filesystem::exists(temp_dir)) {
            std::filesystem::remove_all(temp_dir);
        }
    }

    std::string create_file(const std::string& filename, const std::string& content) {
        std::filesystem::path full_path = std::filesystem::path(temp_dir) / filename;
        std::filesystem::create_directories(full_path.parent_path()); 
        
        std::ofstream out(full_path);
        out << content;
        out.close();
        
        return std::filesystem::weakly_canonical(full_path).string();
    }
};

TEST_F(ImportResolverTest, ResolvesLinearDependencyChain) {
    std::string c_path = create_file("c.vs", "let c_val = 30");
    std::string b_path = create_file("b.vs", "import \"c.vs\"\nlet b_val = 20");
    std::string a_path = create_file("a.vs", "import \"b.vs\"\nlet a_val = 10");

    auto project = test::run_resolver(a_path);

    EXPECT_EQ(project.modules.size(), 3);

    ASSERT_EQ(project.topological_order.size(), 3);
    EXPECT_EQ(project.topological_order[0], c_path);
    EXPECT_EQ(project.topological_order[1], b_path);
    EXPECT_EQ(project.topological_order[2], a_path);
}

TEST_F(ImportResolverTest, ResolvesDiamondDependencyGraph) {
    /*
    //     A
    //    / \
    //   B   C
    //    \ /
    //     D
    */

    std::string d_path = create_file("d.vs", "let d = 4");
    std::string c_path = create_file("c.vs", "import \"d.vs\"\nlet c = 3");
    std::string b_path = create_file("b.vs", "import \"d.vs\"\nlet b = 2");
    std::string a_path = create_file("a.vs", "import \"b.vs\"\nimport \"c.vs\"\nlet a = 1");

    auto project = test::run_resolver(a_path);

    // D should be parsed exactly once, meaning 4 total modules
    EXPECT_EQ(project.modules.size(), 4);

    // D must absolutely be the first file in the topological order
    ASSERT_EQ(project.topological_order.size(), 4);
    EXPECT_EQ(project.topological_order[0], d_path);
    
    // B and C order depends on AST import traversal order, but both must be before A
    EXPECT_EQ(project.topological_order[3], a_path);
}

TEST_F(ImportResolverTest, ResolvesRelativePathsAcrossDirectories) {
    // Tests std::filesystem path normalization
    // main.vs imports "utils/math.vs"
    // utils/math.vs imports "../constants.vs"
    
    std::string const_path = create_file("constants.vs", "let pi = 3.14");
    std::string math_path = create_file("utils/math.vs", "import \"../constants.vs\"\nlet double_pi = pi * 2");
    std::string main_path = create_file("main.vs", "import \"utils/math.vs\"\nlet area = double_pi");

    auto project = test::run_resolver(main_path);

    ASSERT_EQ(project.topological_order.size(), 3);
    EXPECT_EQ(project.topological_order[0], const_path);
    EXPECT_EQ(project.topological_order[1], math_path);
    EXPECT_EQ(project.topological_order[2], main_path);
}

TEST_F(ImportResolverTest, ThrowsOnDirectCircularDependency) {
    // A -> B -> A
    std::string b_path = create_file("b.vs", "import \"a.vs\"");
    std::string a_path = create_file("a.vs", "import \"b.vs\"");

    try {
        test::run_resolver(a_path);
        FAIL() << "Expected ValuaScriptException for circular import, but no exception was thrown.";
    } catch (const ValuaScriptException& e) {
        EXPECT_EQ(e.get_code(), ErrorCode::CircularImportDetected);
    } catch (...) {
        FAIL() << "Expected ValuaScriptException, but a different exception was thrown.";
    }
}

TEST_F(ImportResolverTest, ThrowsOnSelfImport) {
    // A -> A
    std::string a_path = create_file("a.vs", "import \"a.vs\"");

    try {
        test::run_resolver(a_path);
        FAIL() << "Expected ValuaScriptException for self import, but no exception was thrown.";
    } catch (const ValuaScriptException& e) {
        EXPECT_EQ(e.get_code(), ErrorCode::CircularImportDetected);
    }
}

TEST_F(ImportResolverTest, ThrowsOnDeepCircularDependency) {
    // A -> B -> C -> D -> B
    std::string d_path = create_file("d.vs", "import \"b.vs\"");
    std::string c_path = create_file("c.vs", "import \"d.vs\"");
    std::string b_path = create_file("b.vs", "import \"c.vs\"");
    std::string a_path = create_file("a.vs", "import \"b.vs\"");

    try {
        test::run_resolver(a_path);
        FAIL() << "Expected ValuaScriptException for deep circular import, but no exception was thrown.";
    } catch (const ValuaScriptException& e) {
        EXPECT_EQ(e.get_code(), ErrorCode::CircularImportDetected);
    }
}

TEST_F(ImportResolverTest, ThrowsOnMissingFile) {
    // A -> NonExistent
    std::string a_path = create_file("a.vs", "import \"ghost.vs\"");

    try {
        test::run_resolver(a_path);
        FAIL() << "Expected ValuaScriptException for missing file, but no exception was thrown.";
    } catch (const ValuaScriptException& e) {
        EXPECT_EQ(e.get_code(), ErrorCode::ImportFileNotFound);
    }
}

TEST_F(ImportResolverTest, ResolvesComplexRelativePathBacktracking) {
    // Structure:
    // root/
    //   main.vs
    //   core/
    //     base.vs
    //   features/
    //     deep/
    //       feature.vs

    // feature.vs imports base.vs using complex backtracking
    std::string base_path = create_file("core/base.vs", "let version = 1.0");
    std::string feature_path = create_file("features/deep/feature.vs", "import \"../../core/base.vs\"\nlet f = version");
    std::string main_path = create_file("main.vs", "import \"features/deep/feature.vs\"\nlet m = f");

    auto project = test::run_resolver(main_path);

    ASSERT_EQ(project.topological_order.size(), 3);
    EXPECT_EQ(project.topological_order[0], base_path);
    EXPECT_EQ(project.topological_order[1], feature_path);
    EXPECT_EQ(project.topological_order[2], main_path);
}

TEST_F(ImportResolverTest, NormalizesRedundantPathsToSameModule) {
    // main.vs imports both "utils.vs" and "./utils.vs"
    // The resolver must realize these are the exact same physical file.

    std::string utils_path = create_file("utils.vs", "let u = 10");
    std::string main_path = create_file("main.vs", "import \"utils.vs\"\nimport \"./utils.vs\"\nlet m = u");

    auto project = test::run_resolver(main_path);

    // It should only parse 2 files, not 3
    EXPECT_EQ(project.modules.size(), 2);

    ASSERT_EQ(project.topological_order.size(), 2);
    EXPECT_EQ(project.topological_order[0], utils_path);
    EXPECT_EQ(project.topological_order[1], main_path);
}

TEST_F(ImportResolverTest, ResolvesMassiveStarTopology) {
    // main -> [mod1, mod2, mod3, mod4, mod5] -> core

    std::string core_path = create_file("core.vs", "let core_val = 100");

    std::vector<std::string> intermediate_paths;
    std::string main_content;

    // Generate 5 intermediate files that all import core.vs
    for (int i = 1; i <= 5; ++i) {
        std::string mod_name = "mod" + std::to_string(i) + ".vs";
        std::string mod_content = "import \"core.vs\"\nlet m" + std::to_string(i) + " = core_val";

        intermediate_paths.push_back(create_file(mod_name, mod_content));
        main_content += "import \"" + mod_name + "\"\n";
    }

    std::string main_path = create_file("main.vs", main_content);

    auto project = test::run_resolver(main_path);

    // 1 main + 5 intermediates + 1 core = 7 total modules
    EXPECT_EQ(project.modules.size(), 7);
    ASSERT_EQ(project.topological_order.size(), 7);

    // Core MUST be the absolute bottom of the dependency graph (index 0)
    EXPECT_EQ(project.topological_order[0], core_path);

    // Main MUST be the absolute top of the dependency graph (index 6)
    EXPECT_EQ(project.topological_order[6], main_path);
}