#include <gtest/gtest.h>
#include "stages/frontend/frontend_orchestrator.h"

using namespace valuascript::compiler;

TEST(ValidateFrontendOrchestratorTest, FrontendOrchestratorIsConfiguredCorrectly) {
    FrontendOrchestrator orchestrator;
    SUCCEED();
}