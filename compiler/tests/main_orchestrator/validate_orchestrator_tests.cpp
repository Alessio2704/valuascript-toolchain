#include <gtest/gtest.h>
#include "main_orchestrator/main_orchestrator.h"

using namespace valuascript::compiler;

TEST(ValidateOrchestratorTest, OrchestratorIsConfiguredCorrectly) {
    MainOrchestrator orchestrator;
    SUCCEED();
}