#include <gtest/gtest.h>
#include "../../../src/core/main_orchestrator.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    TEST(ValidateMainOrchestratorTest, MainOrchestratorIsConfiguredCorrectly) {
        MainOrchestrator orchestrator;
        SUCCEED();
    }
}
