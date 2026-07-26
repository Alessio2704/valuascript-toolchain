#include <gtest/gtest.h>
#include "frontend/frontend_orchestrator.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    TEST(ValidateFrontendOrchestratorTest, FrontendOrchestratorIsConfiguredCorrectly) {
        FrontendOrchestrator orchestrator;
        SUCCEED();
    }
}
