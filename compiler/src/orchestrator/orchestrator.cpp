#include "orchestrator/orchestrator.h"

#include <iostream>
#include <ostream>

#include "compiler_stage/compiler_stage.h"
#include <stdexcept>
#include <string>
#include <memory>

#include "errors/valuascript_exception.h"
#include "stages/file_reader/file_reader_stage.h"
#include "stages/lexer/lexer_stage.h"
#include "stages/parser/parser_stage.h"

namespace valuascript::compiler {

    void Orchestrator::initialise_file_path(const std::string &file_path) {
        artifacts_.emplace_back(CompilerStageArtifactCode::FilePath, file_path);
    };

    Orchestrator::Orchestrator() {
        stages_.clear();
        artifacts_.clear();

        add_stage(std::make_unique<FileReaderStage>());
        add_stage(std::make_unique<LexerStage>());
        add_stage(std::make_unique<ParserStage>());

        validate_stages_pipeline(stages_);
    }

    void Orchestrator::run(const std::string &file_path) {
        try {
            initialise_file_path(file_path);

            for (const auto &stage: stages_) {
                artifacts_.push_back(stage->run(artifacts_));
            }
        } catch (const ValuaScriptException &e) {
            std::cerr << "Compilation Error: " << e.what() << std::endl;
        } catch (const std::logic_error &e) {
            std::cerr << "INTERNAL COMPILER ERROR: " << e.what() << std::endl;
            std::cerr << "Please report this bug to the maintainers." << std::endl;
        }
    }
}
