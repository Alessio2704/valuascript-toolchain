#include <iostream>
#include <fstream>
#include "core/compiler_context.h"
#include "core/main_orchestrator.h"
#include "core/diagnostic_formatter.h"

using namespace valuascript::compiler;

int main() {
    std::string file_path = "main.vs";

    const auto context = std::make_shared<CompilerContext>();
    context->settings.fail_fast = false;

    std::vector<CompilerStageArtifact> initial_artifacts = {
        {CompilerStageArtifactCode::FilePath, file_path}
    };

    try {
        MainOrchestrator orchestrator;
        orchestrator.run(*context, initial_artifacts);

        if (context->diagnostics.has_errors()) {
            DiagnosticFormatter::print_errors(context->diagnostics.get_errors(), context->source_manager);
            return 1;
        }

        std::cout << "Compilation successful!\n";
        return 0;

    } catch (const ValuaScriptException& ex) {
        std::vector<ValuaScriptException> single_error = { ex };
        DiagnosticFormatter::print_errors(single_error, context->source_manager);
        return 1;

    } catch (const std::exception& ex) {
        std::cerr << "Fatal compiler error: " << ex.what() << "\n";
        return 1;
    }
}


