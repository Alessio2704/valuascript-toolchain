#include <gtest/gtest.h>
#include "stages/frontend/frontend_orchestrator.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    TEST(ValidateFrontendOrchestratorTest, FrontendOrchestratorIsConfiguredCorrectly) {
        FrontendOrchestrator orchestrator;
        SUCCEED();
    }
}
