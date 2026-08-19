#include "file_reader_error_code.h"

namespace valuascript::compiler
{
    std::string_view get_error_template(FileReaderErrorCode code)
    {
        switch (code)
        {
        case FileReaderErrorCode::FileNotFound:
            return "FileReaderStage Error: Cannot open file at path '{}'.";
        }
        return "Unknown FileReader Error";
    }
}
