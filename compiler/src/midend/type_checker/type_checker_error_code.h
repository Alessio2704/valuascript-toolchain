#pragma once
#include <string_view>

namespace valuascript::compiler
{
    enum class TypeCheckerErrorCode
    {
        TypeMismatch = 6001,
        CannotApplyBinaryOperator,
        CannotApplyUnaryOperator,
        NoSuchField,
        NotCallable,
        ArgumentCountMismatch,
        ArgumentLabelMismatch,
        ConditionNotBoolean,
        InvalidReturnType
    };

    std::string_view get_error_template(TypeCheckerErrorCode code);
}
