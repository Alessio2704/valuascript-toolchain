#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace valuascript::compiler
{
    class Type;
    using TypePtr = std::shared_ptr<Type>;

    enum class TypeKind : uint8_t
    {
        Void,
        Bool,
        Int,
        Float,
        String,
        Tensor,
        Tuple,
        Dict,
        Struct,
        Enum,
        Function,
        Unknown
    };

    inline std::string_view type_kind_to_string(TypeKind kind) noexcept
    {
        switch (kind)
        {
        case TypeKind::Void: return "void";
        case TypeKind::Bool: return "bool";
        case TypeKind::Int: return "int";
        case TypeKind::Float: return "float";
        case TypeKind::String: return "string";
        case TypeKind::Tensor: return "tensor";
        case TypeKind::Tuple: return "tuple";
        case TypeKind::Dict: return "dict";
        case TypeKind::Struct: return "struct";
        case TypeKind::Enum: return "enum";
        case TypeKind::Function: return "function";
        case TypeKind::Unknown: return "unknown";
        }
        return "unknown";
    }

    struct FunctionParamType
    {
        std::string label;
        TypePtr type;
    };

    class Type : public std::enable_shared_from_this<Type>
    {
    private:
        TypeKind kind_;
        std::string name_;
        std::vector<TypePtr> element_types_;                     // For Tuple
        std::vector<FunctionParamType> param_types_;             // For Function
        TypePtr return_type_ = nullptr;                          // For Function
        std::unordered_map<std::string, TypePtr> struct_fields_; // For Struct
        std::vector<FunctionParamType> struct_field_list_;        // Ordered for Struct constructor
        TypePtr underlying_type_ = nullptr;                      // For Enum

    public:
        explicit Type(TypeKind kind, std::string name = "")
            : kind_(kind), name_(std::move(name))
        {
        }

        [[nodiscard]] TypeKind get_kind() const noexcept { return kind_; }
        [[nodiscard]] const std::string& get_name() const noexcept { return name_; }

        [[nodiscard]] bool is_numeric() const noexcept
        {
            return kind_ == TypeKind::Int || kind_ == TypeKind::Float;
        }

        [[nodiscard]] bool is_boolean() const noexcept
        {
            return kind_ == TypeKind::Bool;
        }

        [[nodiscard]] bool is_string() const noexcept
        {
            return kind_ == TypeKind::String;
        }

        [[nodiscard]] bool is_void() const noexcept
        {
            return kind_ == TypeKind::Void;
        }

        [[nodiscard]] bool is_unknown() const noexcept
        {
            return kind_ == TypeKind::Unknown;
        }

        [[nodiscard]] bool is_tuple() const noexcept
        {
            return kind_ == TypeKind::Tuple;
        }

        [[nodiscard]] bool is_struct() const noexcept
        {
            return kind_ == TypeKind::Struct;
        }

        [[nodiscard]] bool is_enum() const noexcept
        {
            return kind_ == TypeKind::Enum;
        }

        [[nodiscard]] bool is_callable() const noexcept
        {
            return kind_ == TypeKind::Function || kind_ == TypeKind::Struct;
        }

        // Tuple element types
        [[nodiscard]] const std::vector<TypePtr>& get_element_types() const noexcept { return element_types_; }
        void set_element_types(std::vector<TypePtr> types) { element_types_ = std::move(types); }

        // Function parameter and return types
        [[nodiscard]] const std::vector<FunctionParamType>& get_param_types() const noexcept
        {
            if (kind_ == TypeKind::Struct)
            {
                return struct_field_list_;
            }
            return param_types_;
        }
        void set_param_types(std::vector<FunctionParamType> params) { param_types_ = std::move(params); }
        [[nodiscard]] TypePtr get_return_type() const noexcept
        {
            if (kind_ == TypeKind::Struct)
            {
                return const_cast<Type*>(this)->shared_from_this();
            }
            return return_type_;
        }
        void set_return_type(TypePtr ret) { return_type_ = std::move(ret); }

        // Struct fields
        [[nodiscard]] const std::unordered_map<std::string, TypePtr>& get_struct_fields() const noexcept { return struct_fields_; }
        void set_struct_field(std::string field_name, TypePtr field_type)
        {
            struct_fields_[field_name] = field_type;
            struct_field_list_.push_back({.label = field_name, .type = field_type});
        }
        [[nodiscard]] TypePtr get_field_type(const std::string& field_name) const
        {
            auto it = struct_fields_.find(field_name);
            if (it != struct_fields_.end())
            {
                return it->second;
            }
            return nullptr;
        }

        // Enum underlying type
        [[nodiscard]] TypePtr get_underlying_type() const noexcept { return underlying_type_; }
        void set_underlying_type(TypePtr type) { underlying_type_ = std::move(type); }

        /// Checks if this type is assignable to the target type.
        [[nodiscard]] bool is_assignable_to(const Type& target) const
        {
            // Unknown is a poison pill type for error recovery; assignable to/from anything
            if (is_unknown() || target.is_unknown())
            {
                return true;
            }

            if (kind_ != target.kind_)
            {
                // Int can implicitly promote to Float in numeric expressions
                if (kind_ == TypeKind::Int && target.kind_ == TypeKind::Float)
                {
                    return true;
                }
                return false;
            }

            switch (kind_)
            {
            case TypeKind::Void:
            case TypeKind::Bool:
            case TypeKind::Int:
            case TypeKind::Float:
            case TypeKind::String:
            case TypeKind::Tensor:
                return true;

            case TypeKind::Struct:
            case TypeKind::Enum:
                return name_ == target.name_;

            case TypeKind::Tuple:
                if (element_types_.size() != target.element_types_.size())
                {
                    return false;
                }
                for (size_t i = 0; i < element_types_.size(); ++i)
                {
                    if (!element_types_[i]->is_assignable_to(*target.element_types_[i]))
                    {
                        return false;
                    }
                }
                return true;

            case TypeKind::Function:
                if (param_types_.size() != target.param_types_.size())
                {
                    return false;
                }
                for (size_t i = 0; i < param_types_.size(); ++i)
                {
                    if (param_types_[i].label != target.param_types_[i].label)
                    {
                        return false;
                    }
                    if (!target.param_types_[i].type->is_assignable_to(*param_types_[i].type))
                    {
                        return false;
                    }
                }
                if (return_type_ && target.return_type_)
                {
                    return return_type_->is_assignable_to(*target.return_type_);
                }
                return return_type_ == target.return_type_;

            default:
                return false;
            }
        }

        [[nodiscard]] std::string to_string() const
        {
            switch (kind_)
            {
            case TypeKind::Void: return "void";
            case TypeKind::Bool: return "bool";
            case TypeKind::Int: return "int";
            case TypeKind::Float: return "float";
            case TypeKind::String: return "string";
            case TypeKind::Tensor: return "tensor";
            case TypeKind::Unknown: return "unknown";
            case TypeKind::Struct: return name_.empty() ? "struct" : name_;
            case TypeKind::Enum: return name_.empty() ? "enum" : name_;
            case TypeKind::Tuple:
            {
                std::ostringstream oss;
                oss << "(";
                for (size_t i = 0; i < element_types_.size(); ++i)
                {
                    if (i > 0) oss << ", ";
                    oss << (element_types_[i] ? element_types_[i]->to_string() : "unknown");
                }
                oss << ")";
                return oss.str();
            }
            case TypeKind::Function:
            {
                std::ostringstream oss;
                oss << "func(";
                for (size_t i = 0; i < param_types_.size(); ++i)
                {
                    if (i > 0) oss << ", ";
                    if (!param_types_[i].label.empty())
                    {
                        oss << param_types_[i].label << ": ";
                    }
                    oss << (param_types_[i].type ? param_types_[i].type->to_string() : "unknown");
                }
                oss << ") -> " << (return_type_ ? return_type_->to_string() : "void");
                return oss.str();
            }
            default:
                return std::string(type_kind_to_string(kind_));
            }
        }

        [[nodiscard]] bool operator==(const Type& other) const
        {
            return is_assignable_to(other) && other.is_assignable_to(*this);
        }
    };

    // Factory functions for canonical singleton/convenience types
    inline TypePtr make_void_type() { return std::make_shared<Type>(TypeKind::Void); }
    inline TypePtr make_bool_type() { return std::make_shared<Type>(TypeKind::Bool); }
    inline TypePtr make_int_type() { return std::make_shared<Type>(TypeKind::Int); }
    inline TypePtr make_float_type() { return std::make_shared<Type>(TypeKind::Float); }
    inline TypePtr make_string_type() { return std::make_shared<Type>(TypeKind::String); }
    inline TypePtr make_tensor_type() { return std::make_shared<Type>(TypeKind::Tensor); }
    inline TypePtr make_unknown_type() { return std::make_shared<Type>(TypeKind::Unknown); }

    inline TypePtr make_struct_type(std::string name)
    {
        return std::make_shared<Type>(TypeKind::Struct, std::move(name));
    }

    inline TypePtr make_enum_type(std::string name, TypePtr underlying = make_int_type())
    {
        auto enum_t = std::make_shared<Type>(TypeKind::Enum, std::move(name));
        enum_t->set_underlying_type(std::move(underlying));
        return enum_t;
    }

    inline TypePtr make_tuple_type(std::vector<TypePtr> elements)
    {
        auto tuple_t = std::make_shared<Type>(TypeKind::Tuple);
        tuple_t->set_element_types(std::move(elements));
        return tuple_t;
    }

    inline TypePtr make_function_type(std::vector<FunctionParamType> params, TypePtr ret)
    {
        auto fn_t = std::make_shared<Type>(TypeKind::Function);
        fn_t->set_param_types(std::move(params));
        fn_t->set_return_type(std::move(ret));
        return fn_t;
    }
}
