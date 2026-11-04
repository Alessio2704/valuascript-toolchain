#include "type_checker_error_code.h"

namespace valuascript::compiler
{
    std::string_view get_error_template(TypeCheckerErrorCode code)
    {
        switch (code)
        {
        case TypeCheckerErrorCode::TypeMismatch:
            return "Type Error: Type mismatch: expected '{}', got '{}'.";
        case TypeCheckerErrorCode::CannotApplyBinaryOperator:
            return "Type Error: Cannot apply binary operator '{}' to types '{}' and '{}'.";
        case TypeCheckerErrorCode::CannotApplyUnaryOperator:
            return "Type Error: Cannot apply unary operator '{}' to type '{}'.";
        case TypeCheckerErrorCode::NoSuchField:
            return "Type Error: Type '{}' has no field named '{}'.";
        case TypeCheckerErrorCode::NotCallable:
            return "Type Error: Expression of type '{}' is not callable.";
        case TypeCheckerErrorCode::ArgumentCountMismatch:
            return "Type Error: Function '{}' expects {} arguments, but got {}.";
        case TypeCheckerErrorCode::ArgumentLabelMismatch:
            return "Type Error: Argument label mismatch: expected '{}', got '{}'.";
        case TypeCheckerErrorCode::ConditionNotBoolean:
            return "Type Error: Condition expression must have boolean type, got '{}'.";
        case TypeCheckerErrorCode::InvalidReturnType:
            return "Type Error: Function '{}' expected return type '{}', got '{}'.";
        }
        return "Unknown Type Error";
    }
}
