#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <concepts>
#include <type_traits>

#include "ast/validity/ast_validity.h"
#include "ast/metadata/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename T>
    concept CanCallAstIsValid = requires(const T& val)
    {
        { ast_is_valid(val) } -> std::same_as<bool>;
    };

    template <ValidatableAstNode T>
    consteval bool verify_validatable_ast_node()
    {
        return true;
    }

    template <typename Tuple>
    struct AstValidityCompletenessValidator;

    template <typename... Types>
    struct AstValidityCompletenessValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_validatable_ast_node<Types>() && ...);
        }
    };

    static_assert(AstValidityCompletenessValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node in AllAstNodeTypes must implement [[nodiscard]] bool is_valid() const noexcept");

    template <typename T>
    consteval bool verify_positive_ast_validity()
    {
        static_assert(CanCallAstIsValid<T>, "Type T must support ast_is_valid(const T&)");
        static_assert(CanCallAstIsValid<T*>, "Type T* must support ast_is_valid(const T*)");
        static_assert(CanCallAstIsValid<const T*>, "Type const T* must support ast_is_valid(const T*)");
        static_assert(CanCallAstIsValid<std::unique_ptr<T>>, "std::unique_ptr<T> must support ast_is_valid(const std::unique_ptr<T>&)");
        static_assert(CanCallAstIsValid<std::vector<std::unique_ptr<T>>>, "std::vector<std::unique_ptr<T>> must support ast_is_valid(const std::vector<std::unique_ptr<T>>&)");
        static_assert(CanCallAstIsValid<std::vector<T*>>, "std::vector<T*> must support ast_is_valid(const std::vector<T*>&)");
        return true;
    }

    template <typename Tuple>
    struct PositiveAstValidityValidator;

    template <typename... Types>
    struct PositiveAstValidityValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_positive_ast_validity<Types>() && ...);
        }
    };

    using ValidAstLeafTypes = std::tuple<SourceSpan, NodeName, AstNode>;

    static_assert(PositiveAstValidityValidator<AllAstNodeTypes>::validate(),
                  "All registered AST nodes must support ast_is_valid across pointers, unique_ptr, and vectors");
    static_assert(PositiveAstValidityValidator<ValidAstLeafTypes>::validate(),
                  "All AST leaf and base types must support ast_is_valid across pointers, unique_ptr, and vectors");

    using NonAstPrimitiveTypes = std::tuple<
        int,
        bool,
        float,
        double,
        char,
        size_t,
        std::string,
        const char*
    >;

    template <typename T>
    consteval bool verify_negative_ast_validity()
    {
        static_assert(!CanCallAstIsValid<T>, "Non-AST type T must be rejected by ast_is_valid");
        static_assert(!CanCallAstIsValid<T*>, "Non-AST pointer T* must be rejected by ast_is_valid");
        static_assert(!CanCallAstIsValid<const T*>, "Non-AST pointer const T* must be rejected by ast_is_valid");
        static_assert(!CanCallAstIsValid<std::unique_ptr<T>>, "Non-AST unique_ptr<T> must be rejected by ast_is_valid");
        static_assert(!CanCallAstIsValid<std::vector<T>>, "Non-AST vector<T> must be rejected by ast_is_valid");
        static_assert(!CanCallAstIsValid<std::vector<std::unique_ptr<T>>>, "Non-AST vector<unique_ptr<T>> must be rejected by ast_is_valid");
        static_assert(!CanCallAstIsValid<std::vector<T*>>, "Non-AST vector<T*> must be rejected by ast_is_valid");
        return true;
    }

    template <typename Tuple>
    struct NegativeAstValidityValidator;

    template <typename... Types>
    struct NegativeAstValidityValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_negative_ast_validity<Types>() && ...);
        }
    };

    static_assert(NegativeAstValidityValidator<NonAstPrimitiveTypes>::validate(),
                  "Non-AST types must be rejected by ast_is_valid at compile-time across all container/pointer combinations");
}
