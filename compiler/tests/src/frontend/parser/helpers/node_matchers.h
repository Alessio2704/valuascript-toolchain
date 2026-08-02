#pragma once

#include <gtest/gtest.h>
#include <concepts>
#include <type_traits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <utility>
#include <tuple>
#include <variant>
#include <span>
#include <compare>
#include <source_location>
#include "frontend/parser/ast.h"
#include "utils/demangle_name.h"

namespace valuascript::compiler::test
{
    struct StringStorage
    {
        std::variant<std::string_view, std::string> data_;

        constexpr StringStorage() : data_(std::in_place_index<0>, std::string_view{})
        {
        }

        constexpr StringStorage(const char* s) : data_(std::in_place_index<0>, std::string_view(s ? s : ""))
        {
        }

        constexpr StringStorage(std::string_view sv) : data_(std::in_place_index<0>, sv)
        {
        }

        StringStorage(std::string s) : data_(std::move(s))
        {
        }

        [[nodiscard]] operator std::string_view() const { return get(); }

        [[nodiscard]] std::string_view get() const
        {
            if (auto* sv = std::get_if<std::string_view>(&data_)) [[likely]] return *sv;
            return std::get<std::string>(data_);
        }

        [[nodiscard]] const char* data() const { return get().data(); }
        [[nodiscard]] size_t size() const { return get().size(); }
        [[nodiscard]] bool empty() const { return get().empty(); }

        [[nodiscard]] friend bool operator==(const StringStorage& lhs, std::string_view rhs) { return lhs.get() == rhs; }
        [[nodiscard]] friend std::strong_ordering operator<=>(const StringStorage& lhs, std::string_view rhs) { return lhs.get() <=> rhs; }
    };

    struct AnyMatcher;

    template <typename F, typename NodeT>
    concept HasNodeType = requires
    {
        typename std::decay_t<F>::node_type;
    };

    template <typename F, typename NodeT>
    concept IsCompatibleNodeVerifier =
        (HasNodeType<F, NodeT> && std::derived_from<typename std::decay_t<F>::node_type, NodeT>) ||
        (!HasNodeType<F, NodeT> && std::invocable<F, NodeT*>);

    template <typename T>
    concept ASTNodeConcept = std::derived_from<std::decay_t<T>, AstNode>;

    template <typename M, typename NodeT>
    concept ASTMatcher = requires(const std::decay_t<M>& m, NodeT* node) {
        { m(node) };
    };

    template <typename M>
    concept ExprMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeVerifier<M, Expression>;

    template <typename M>
    concept TypeNodeMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeVerifier<M, TypeAnnotation>;

    template <typename M>
    concept StmtMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeVerifier<M, Statement>;

    template <typename NodeT, size_t BufferSize = 64>
    class InlineVerifier
    {
    private:
        struct VTable
        {
            void (*invoker)(const std::byte*, NodeT*);
            void (*copy_ctor)(std::byte*, const std::byte*);
            void (*move_ctor)(std::byte*, std::byte*);
            void (*dtor)(std::byte*);
        };

        alignas(std::max_align_t) std::byte buffer_[BufferSize];
        const VTable* vtable_ = nullptr;

        template <typename DecayedF>
        static const VTable* get_inline_vtable()
        {
            static constexpr VTable vt{
                [](const std::byte* buf, NodeT* node)
                {
                    (*reinterpret_cast<const DecayedF*>(buf))(node);
                },
                [](std::byte* dst, const std::byte* src)
                {
                    std::construct_at(reinterpret_cast<DecayedF*>(dst), *reinterpret_cast<const DecayedF*>(src));
                },
                [](std::byte* dst, std::byte* src)
                {
                    std::construct_at(reinterpret_cast<DecayedF*>(dst), std::move(*reinterpret_cast<DecayedF*>(src)));
                },
                [](std::byte* buf)
                {
                    std::destroy_at(reinterpret_cast<DecayedF*>(buf));
                }
            };
            return &vt;
        }

        template <typename DecayedF>
        static const VTable* get_heap_vtable()
        {
            using SharedPtr = std::shared_ptr<DecayedF>;
            static constexpr VTable vt{
                [](const std::byte* buf, NodeT* node)
                {
                    (*(*reinterpret_cast<const SharedPtr*>(buf)))(node);
                },
                [](std::byte* dst, const std::byte* src)
                {
                    std::construct_at(reinterpret_cast<SharedPtr*>(dst), *reinterpret_cast<const SharedPtr*>(src));
                },
                [](std::byte* dst, std::byte* src)
                {
                    std::construct_at(reinterpret_cast<SharedPtr*>(dst), std::move(*reinterpret_cast<SharedPtr*>(src)));
                },
                [](std::byte* buf)
                {
                    std::destroy_at(reinterpret_cast<SharedPtr*>(buf));
                }
            };
            return &vt;
        }

    public:
        InlineVerifier() = default;

        InlineVerifier(std::nullptr_t)
        {
        }

        template <typename F>
            requires (!std::same_as<std::decay_t<F>, InlineVerifier> &&
                !std::same_as<std::decay_t<F>, std::nullptr_t> &&
                IsCompatibleNodeVerifier<F, NodeT>)
        InlineVerifier(F&& f)
        {
            using DecayedF = std::decay_t<F>;
            if constexpr (sizeof(DecayedF) <= BufferSize && alignof(DecayedF) <= alignof(std::max_align_t))
            {
                std::construct_at(reinterpret_cast<DecayedF*>(buffer_), std::forward<F>(f));
                vtable_ = get_inline_vtable<DecayedF>();
            }
            else
            {
                using SharedPtr = std::shared_ptr<DecayedF>;
                auto ptr = std::make_shared<DecayedF>(std::forward<F>(f));
                std::construct_at(reinterpret_cast<SharedPtr*>(buffer_), std::move(ptr));
                vtable_ = get_heap_vtable<DecayedF>();
            }
        }

        ~InlineVerifier()
        {
            reset();
        }

        InlineVerifier(const InlineVerifier& other)
        {
            if (other.vtable_) [[likely]]
            {
                other.vtable_->copy_ctor(buffer_, other.buffer_);
                vtable_ = other.vtable_;
            }
        }

        InlineVerifier(InlineVerifier&& other) noexcept
        {
            if (other.vtable_) [[likely]]
            {
                other.vtable_->move_ctor(buffer_, other.buffer_);
                vtable_ = other.vtable_;
                other.vtable_ = nullptr;
            }
        }

        InlineVerifier& operator=(const InlineVerifier& other)
        {
            if (this != &other) [[likely]]
            {
                reset();
                if (other.vtable_) [[likely]]
                {
                    other.vtable_->copy_ctor(buffer_, other.buffer_);
                    vtable_ = other.vtable_;
                }
            }
            return *this;
        }

        InlineVerifier& operator=(InlineVerifier&& other) noexcept
        {
            if (this != &other) [[likely]]
            {
                reset();
                if (other.vtable_) [[likely]]
                {
                    other.vtable_->move_ctor(buffer_, other.buffer_);
                    vtable_ = other.vtable_;
                    other.vtable_ = nullptr;
                }
            }
            return *this;
        }

        void operator()(NodeT* node) const
        {
            if (vtable_) [[likely]] vtable_->invoker(buffer_, node);
        }

        [[nodiscard]] explicit operator bool() const { return vtable_ != nullptr; }

        [[nodiscard]] friend bool operator==(const InlineVerifier& v, std::nullptr_t) { return !static_cast<bool>(v); }
        [[nodiscard]] friend bool operator==(std::nullptr_t, const InlineVerifier& v) { return !static_cast<bool>(v); }
        [[nodiscard]] friend bool operator!=(const InlineVerifier& v, std::nullptr_t) { return static_cast<bool>(v); }
        [[nodiscard]] friend bool operator!=(std::nullptr_t, const InlineVerifier& v) { return static_cast<bool>(v); }

        void reset()
        {
            if (vtable_) [[likely]]
            {
                vtable_->dtor(buffer_);
                vtable_ = nullptr;
            }
        }
    };

    struct ModifierSpec;

    using ExprVerifier = InlineVerifier<Expression>;
    using TypeVerifier = InlineVerifier<TypeAnnotation>;
    using ImportVerifier = InlineVerifier<ImportStatement>;
    using DirectiveVerifier = InlineVerifier<Directive>;
    using FuncVerifier = InlineVerifier<FunctionDefinition>;
    using ExtVerifier = InlineVerifier<ExtensionDefinition>;
    using StructVerifier = InlineVerifier<StructDefinition>;
    using EnumVerifier = InlineVerifier<EnumDefinition>;
    using AliasVerifier = InlineVerifier<TypeAliasDefinition>;

    using ModifierVerifier = std::vector<ModifierSpec>;
    using AssignmentVerifier = InlineVerifier<Assignment>;
    using ReassignmentVerifier = InlineVerifier<Reassignment>;
    using ReturnVerifier = InlineVerifier<ReturnStatement>;
    using ExprStmtVerifier = InlineVerifier<ExpressionStatement>;

    struct ArgSpec
    {
        StringStorage label;
        ExprVerifier value_v = nullptr;
    };

    struct ModifierSpec
    {
        StringStorage name;
        std::vector<ArgSpec> args = {};
    };

    struct ParamSpec
    {
        StringStorage name;
        std::vector<ModifierSpec> modifiers = {};
        TypeVerifier type_v = nullptr;
        ExprVerifier default_v = nullptr;
    };

    struct FieldSpec
    {
        StringStorage name;
        std::vector<ModifierSpec> modifiers = {};
        TypeVerifier type_v = nullptr;
    };

    struct EnumCaseSpec
    {
        StringStorage name;
        std::vector<ModifierSpec> modifiers = {};
        ExprVerifier value_v = nullptr;
    };

    struct DictItemSpec
    {
        StringStorage key;
        std::vector<ModifierSpec> modifiers = {};
        ExprVerifier value_v = nullptr;
    };

    struct SwitchCaseSpec
    {
        std::vector<ModifierSpec> modifiers = {};
        std::vector<StringStorage> labels = {};
        ExprVerifier result_v = nullptr;
    };

    struct AssignmentTargetSpec
    {
        std::vector<ModifierSpec> modifiers = {};
        StringStorage name;
        TypeVerifier type_v = nullptr;
    };

    template <typename T>
        requires std::derived_from<T, AstNode>
    T* ExpectNode(AstNode* node, std::source_location loc = std::source_location::current())
    {
        if (!node) [[unlikely]]
        {
            ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                << "Expected AST node of type [" << get_demangled_name(typeid(T).name())
                << "], but got [nullptr].";
            return nullptr;
        }
        T* casted = ast_cast<T>(node);
        if (!casted) [[unlikely]]
        {
            ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                << "Expected AST type [" << get_demangled_name(typeid(T).name())
                << "], but got [" << get_demangled_name(typeid(*node).name()) << "].";
            return nullptr;
        }
        return casted;
    }

    struct StmtVerifier : public InlineVerifier<Statement>
    {
        StmtVerifier() = default;

        StmtVerifier(std::nullptr_t) : InlineVerifier<Statement>(nullptr)
        {
        }

        StmtVerifier(AssignmentVerifier v)
        {
            if (v) [[likely]]
            {
                *this = StmtVerifier([ver = std::move(v)](Statement* s)
                {
                    if (auto* casted = ExpectNode<Assignment>(s)) [[likely]] ver(casted);
                });
            }
        }

        StmtVerifier(ReassignmentVerifier v)
        {
            if (v) [[likely]]
            {
                *this = StmtVerifier([ver = std::move(v)](Statement* s)
                {
                    if (auto* casted = ExpectNode<Reassignment>(s)) [[likely]] ver(casted);
                });
            }
        }

        StmtVerifier(ReturnVerifier v)
        {
            if (v) [[likely]]
            {
                *this = StmtVerifier([ver = std::move(v)](Statement* s)
                {
                    if (auto* casted = ExpectNode<ReturnStatement>(s)) [[likely]] ver(casted);
                });
            }
        }

        StmtVerifier(ExprStmtVerifier v)
        {
            if (v) [[likely]]
            {
                *this = StmtVerifier([ver = std::move(v)](Statement* s)
                {
                    if (auto* casted = ExpectNode<ExpressionStatement>(s)) [[likely]] ver(casted);
                });
            }
        }

        template <typename F>
            requires (!std::derived_from<std::decay_t<F>, Statement> &&
                !std::same_as<std::decay_t<F>, StmtVerifier> &&
                !std::same_as<std::decay_t<F>, InlineVerifier<Statement>> &&
                !std::same_as<std::decay_t<F>, std::nullptr_t> &&
                IsCompatibleNodeVerifier<F, Statement>)
        StmtVerifier(F&& f) : InlineVerifier<Statement>(std::forward<F>(f))
        {
        }
    };

    struct ProgramSpec
    {
        std::vector<ImportVerifier> imports = {};
        std::vector<DirectiveVerifier> directives = {};
        std::vector<StmtVerifier> execution_steps = {};
        std::vector<FuncVerifier> functions = {};
        std::vector<StructVerifier> structs = {};
        std::vector<EnumVerifier> enums = {};
        std::vector<AliasVerifier> type_aliases = {};
        std::vector<ExtVerifier> extensions = {};
    };

    inline void ExpectNullNode(AstNode* node)
    {
        EXPECT_EQ(node, nullptr) << "Expected node to be null, but it was populated.";
    }

    struct NullVerifier
    {
        void operator()(AstNode* node) const { ExpectNullNode(node); }
        void operator()(TypeAnnotation* node) const { ExpectNullNode(node); }
    };

    struct AnyMatcher
    {
        void operator()(AstNode*) const
        {
        }

        void operator()(Expression*) const
        {
        }

        void operator()(Statement*) const
        {
        }

        void operator()(TypeAnnotation*) const
        {
        }

        explicit operator bool() const { return false; }
    };

    template <typename F>
    struct MatcherStorage
    {
        F verifier;
        bool has_value = true;

        MatcherStorage() : verifier{}, has_value(false)
        {
        }

        MatcherStorage(F v) : verifier(std::move(v)), has_value(true)
        {
        }

        MatcherStorage(std::nullptr_t) : verifier{}, has_value(false)
        {
        }

        template <typename NodeT>
        void operator()(NodeT* node) const
        {
            if (has_value) verifier(node);
        }

        explicit operator bool() const { return has_value; }
    };

    template <>
    struct MatcherStorage<AnyMatcher>
    {
        AnyMatcher verifier;
        bool has_value = false;

        MatcherStorage() : verifier{}, has_value(false)
        {
        }

        MatcherStorage(AnyMatcher v) : verifier(std::move(v)), has_value(false)
        {
        }

        MatcherStorage(std::nullptr_t) : verifier{}, has_value(false)
        {
        }

        template <typename NodeT>
        void operator()(NodeT*) const
        {
        }

        explicit operator bool() const { return false; }
    };

    inline void ExpectIdentifier(AstNode* node, std::string_view name)
    {
        if (auto id = ExpectNode<IdentifierAccess>(node))
        {
            EXPECT_EQ(id->name, name) << "Identifier name mismatch.";
        }
    }

    inline void ExpectNumber(AstNode* node, std::string_view val)
    {
        if (auto n = ExpectNode<NumberLiteral>(node))
        {
            EXPECT_EQ(n->value, val) << "Number literal value mismatch.";
        }
    }

    inline void ExpectString(AstNode* node, std::string_view val)
    {
        if (auto s = ExpectNode<StringLiteral>(node))
        {
            EXPECT_EQ(s->value, val) << "String literal value mismatch.";
        }
    }

    inline void ExpectBoolean(AstNode* node, bool val)
    {
        if (auto b = ExpectNode<BooleanLiteral>(node))
        {
            EXPECT_EQ(b->value, val) << "Boolean literal value mismatch.";
        }
    }

    inline void ExpectPercentage(AstNode* node, std::string_view val)
    {
        if (auto p = ExpectNode<PercentageLiteral>(node))
        {
            EXPECT_EQ(p->value, val) << "Percentage literal value mismatch.";
        }
    }

    inline void ExpectSelf(AstNode* node)
    {
        ExpectNode<SelfExpression>(node);
    }

    inline void ExpectArguments(std::span<const std::pair<std::string, std::unique_ptr<Expression>>> actual,
                                std::span<const ArgSpec> specs)
    {
        ASSERT_EQ(actual.size(), specs.size()) << "Arg count mismatch.";
        for (size_t i = 0; i < specs.size(); i++)
        {
            EXPECT_EQ(actual[i].first, specs[i].label.get()) << "Argument label mismatch at index " << i << ".";
            if (specs[i].value_v) specs[i].value_v(actual[i].second.get());
        }
    }

    inline void ExpectModifiers(std::span<const Modifier> actual, std::span<const ModifierSpec> specs)
    {
        ASSERT_EQ(actual.size(), specs.size()) << "Modifier count mismatch.";
        for (size_t i = 0; i < specs.size(); i++)
        {
            EXPECT_EQ(actual[i].name, specs[i].name.get()) << "Modifier name mismatch at index " << i << ".";
            ExpectArguments(actual[i].arguments, specs[i].args);
        }
    }

    template <ExprMatcher L, ExprMatcher R>
    inline void ExpectBinary(AstNode* node, TokenType op, const L& l_v, const R& r_v)
    {
        if (auto b = ExpectNode<BinaryExpression>(node))
        {
            EXPECT_EQ(b->op, op) << "Binary expression operator mismatch.";
            if (l_v) l_v(b->left.get());
            if (r_v) r_v(b->right.get());
        }
    }

    template <ExprMatcher R>
    inline void ExpectUnary(AstNode* node, TokenType op, const R& r_v)
    {
        if (auto u = ExpectNode<UnaryExpression>(node))
        {
            EXPECT_EQ(u->op, op) << "Unary expression operator mismatch.";
            if (r_v) r_v(u->right.get());
        }
    }

    template <ExprMatcher I>
    inline void ExpectGrouping(AstNode* node, const I& inner_v)
    {
        if (auto g = ExpectNode<GroupingExpression>(node))
        {
            if (inner_v) inner_v(g->expression.get());
        }
    }

    template <ExprMatcher C, ExprMatcher T, ExprMatcher E>
    inline void ExpectConditional(AstNode* node, const C& c_v, const T& t_v, const E& e_v)
    {
        if (auto cond = ExpectNode<ConditionalExpression>(node))
        {
            if (c_v) c_v(cond->condition.get());
            if (t_v) t_v(cond->then_branch.get());
            if (e_v) e_v(cond->else_branch.get());
        }
    }

    template <ExprMatcher T>
    inline void ExpectCall(AstNode* node, const T& target_v, std::span<const ArgSpec> args)
    {
        if (auto c = ExpectNode<FunctionCall>(node))
        {
            if (target_v) target_v(c->target.get());
            ExpectArguments(c->arguments, args);
        }
    }

    template <ExprMatcher T, ExprMatcher I>
    inline void ExpectBracketAccess(AstNode* node, const T& target_v, const I& index_v)
    {
        if (auto b = ExpectNode<BracketAccess>(node))
        {
            if (target_v) target_v(b->target.get());
            if (index_v) index_v(b->index.get());
        }
    }

    template <ExprMatcher T>
    inline void ExpectDotAccess(AstNode* node, const T& target_v, std::string_view prop)
    {
        if (auto d = ExpectNode<DotAccess>(node))
        {
            if (target_v) target_v(d->target.get());
            EXPECT_EQ(d->property_name, prop) << "Dot access property name mismatch.";
        }
    }

    template <ExprMatcher T, ExprMatcher D>
    inline void ExpectSwitch(AstNode* node, const T& target_v, std::span<const SwitchCaseSpec> cases,
                             std::span<const ModifierSpec> default_mods,
                             const D& def_v)
    {
        if (auto sw = ExpectNode<SwitchExpression>(node))
        {
            if (target_v) target_v(sw->target.get());
            ASSERT_EQ(sw->cases.size(), cases.size()) << "Switch cases count mismatch.";
            for (size_t i = 0; i < cases.size(); i++)
            {
                ExpectModifiers(sw->cases[i].modifiers, cases[i].modifiers);
                ASSERT_EQ(sw->cases[i].identifiers.size(), cases[i].labels.size());
                for (size_t l = 0; l < cases[i].labels.size(); l++)
                {
                    EXPECT_EQ(sw->cases[i].identifiers[l], cases[i].labels[l].get()) << "Switch case label mismatch.";
                }
                if (cases[i].result_v) cases[i].result_v(sw->cases[i].result.get());
            }
            ExpectModifiers(sw->default_modifiers, default_mods);
            if (def_v) def_v(sw->default_case.get());
        }
    }

    inline void ExpectTensor(AstNode* node, std::span<const ExprVerifier> elements)
    {
        if (auto t = ExpectNode<TensorLiteral>(node))
        {
            ASSERT_EQ(t->elements.size(), elements.size()) << "Tensor elements count mismatch.";
            for (size_t i = 0; i < elements.size(); i++)
            {
                if (elements[i]) elements[i](t->elements[i].get());
            }
        }
    }

    inline void ExpectTuple(AstNode* node, std::span<const ExprVerifier> elements)
    {
        if (auto t = ExpectNode<TupleLiteral>(node))
        {
            ASSERT_EQ(t->elements.size(), elements.size()) << "Tuple elements count mismatch.";
            for (size_t i = 0; i < elements.size(); i++)
            {
                if (elements[i]) elements[i](t->elements[i].get());
            }
        }
    }

    inline void ExpectDict(AstNode* node, std::span<const DictItemSpec> items)
    {
        if (auto d = ExpectNode<DictLiteral>(node))
        {
            ASSERT_EQ(d->elements.size(), items.size()) << "Dictionary items count mismatch.";
            for (size_t i = 0; i < items.size(); i++)
            {
                EXPECT_EQ(d->elements[i].key, items[i].key.get()) << "Dictionary item key mismatch at index " << i << ".";
                ExpectModifiers(d->elements[i].modifiers, items[i].modifiers);
                if (items[i].value_v) items[i].value_v(d->elements[i].value.get());
            }
        }
    }

    inline void ExpectType(TypeAnnotation* node, std::string_view name,
                           std::span<const TypeVerifier> generics = {})
    {
        ASSERT_NE(node, nullptr) << "Expected TypeAnnotation node, but got nullptr.";
        EXPECT_EQ(node->name, name) << "TypeAnnotation name mismatch.";
        ASSERT_EQ(node->generic_args.size(), generics.size()) << "Generic arg count mismatch for type '" << name << "'.";
        for (size_t i = 0; i < generics.size(); i++)
        {
            if (generics[i]) generics[i](node->generic_args[i].get());
        }
    }

    inline void ExpectTupleType(TypeAnnotation* node, std::span<const TypeVerifier> elements)
    {
        if (auto t = ExpectNode<TupleTypeAnnotation>(node))
        {
            ASSERT_EQ(t->element_types.size(), elements.size()) << "TupleType element count mismatch.";
            for (size_t i = 0; i < elements.size(); i++)
            {
                if (elements[i]) elements[i](t->element_types[i].get());
            }
        }
    }

    template <ExprMatcher V>
    inline void ExpectAssignment(Statement* stmt, std::span<const AssignmentTargetSpec> targets,
                                 const V& val_v)
    {
        if (auto a = ExpectNode<Assignment>(stmt))
        {
            ASSERT_EQ(a->targets.size(), targets.size()) << "Assignment targets count mismatch.";
            for (size_t i = 0; i < targets.size(); i++)
            {
                ExpectModifiers(a->targets[i].modifiers, targets[i].modifiers);
                EXPECT_EQ(a->targets[i].name, targets[i].name.get()) << "Assignment target name mismatch at index " << i << ".";
                if (targets[i].type_v) targets[i].type_v(a->targets[i].type.get());
            }
            if (val_v) val_v(a->value.get());
        }
    }

    template <ExprMatcher T, ExprMatcher V>
    inline void ExpectReassignment(Statement* stmt, const T& target_v, const V& val_v)
    {
        if (auto r = ExpectNode<Reassignment>(stmt))
        {
            if (target_v) target_v(r->target.get());
            if (val_v) val_v(r->value.get());
        }
    }

    inline void ExpectReturn(Statement* stmt,
                             std::span<const ModifierSpec> modifiers,
                             std::span<const ExprVerifier> values)
    {
        if (auto r = ExpectNode<ReturnStatement>(stmt))
        {
            ExpectModifiers(r->modifiers, modifiers);
            ASSERT_EQ(r->values.size(), values.size()) << "Return values count mismatch.";
            for (size_t i = 0; i < values.size(); i++)
            {
                if (values[i]) values[i](r->values[i].get());
            }
        }
    }

    template <ExprMatcher E>
    inline void ExpectExprStmt(Statement* stmt, const E& expr_v)
    {
        if (auto es = ExpectNode<ExpressionStatement>(stmt))
        {
            if (expr_v) expr_v(es->expr.get());
        }
    }

    inline void ExpectFunctionDef(FunctionDefinition* f, std::string_view name,
                                  std::span<const ModifierSpec> modifiers,
                                  std::span<const ParamSpec> params,
                                  std::span<const TypeVerifier> returns,
                                  std::span<const StmtVerifier> body,
                                  const std::optional<StringStorage>& docstring)
    {
        ASSERT_NE(f, nullptr) << "Expected FunctionDefinition node, but got nullptr.";
        EXPECT_EQ(f->name, name) << "FunctionDefinition name mismatch.";
        ExpectModifiers(f->modifiers, modifiers);
        if (docstring.has_value())
        {
            EXPECT_EQ(f->docstring, docstring->get()) << "FunctionDefinition docstring mismatch for function '" << name << "'.";
        }
        else
        {
            EXPECT_EQ(f->docstring, std::nullopt) << "FunctionDefinition docstring mismatch for function '" << name << "'.";
        }

        ASSERT_EQ(f->parameters.size(), params.size()) << "FunctionDefinition parameters count mismatch for function '"
            << name << "'.";
        for (size_t i = 0; i < params.size(); i++)
        {
            EXPECT_EQ(f->parameters[i].name, params[i].name.get()) << "Function parameter name mismatch at index " << i << " for function '" << name << "'.";
            ExpectModifiers(f->parameters[i].modifiers, params[i].modifiers);
            if (params[i].type_v) params[i].type_v(f->parameters[i].type.get());
            if (params[i].default_v) params[i].default_v(f->parameters[i].default_value.get());
        }

        ASSERT_EQ(f->return_types.size(), returns.size()) <<
            "FunctionDefinition return types count mismatch for function '" << name << "'.";
        for (size_t i = 0; i < returns.size(); i++)
        {
            if (returns[i]) returns[i](f->return_types[i].get());
        }

        ASSERT_EQ(f->body.size(), body.size()) << "FunctionDefinition body statements count mismatch for function '" <<
            name << "'.";
        for (size_t i = 0; i < body.size(); i++)
        {
            if (body[i]) body[i](f->body[i].get());
        }
    }

    inline void ExpectExtensionDef(ExtensionDefinition* e,
                                   std::span<const ModifierSpec> modifiers,
                                   const TypeVerifier& target,
                                   const ProgramSpec& spec)
    {
        ASSERT_NE(e, nullptr) << "Expected ExtensionDefinition node, but got nullptr.";
        ExpectModifiers(e->modifiers, modifiers);
        if (target) target(e->target_type.get());

        ASSERT_EQ(e->execution_steps.size(), spec.execution_steps.size()) << "Execution steps count mismatch in Extension.";
        for (size_t i = 0; i < spec.execution_steps.size(); i++)
            if (spec.execution_steps[i]) spec.execution_steps[i](e->execution_steps[i].get());

        ASSERT_EQ(e->function_definitions.size(), spec.functions.size()) << "Functions count mismatch in Extension.";
        for (size_t i = 0; i < spec.functions.size(); i++)
            if (spec.functions[i]) spec.functions[i](e->function_definitions[i].get());

        ASSERT_EQ(e->struct_definitions.size(), spec.structs.size()) << "Structs count mismatch in Extension.";
        for (size_t i = 0; i < spec.structs.size(); i++)
            if (spec.structs[i]) spec.structs[i](e->struct_definitions[i].get());

        ASSERT_EQ(e->enum_definitions.size(), spec.enums.size()) << "Enums count mismatch in Extension.";
        for (size_t i = 0; i < spec.enums.size(); i++)
            if (spec.enums[i]) spec.enums[i](e->enum_definitions[i].get());

        ASSERT_EQ(e->type_aliases.size(), spec.type_aliases.size()) << "Type aliases count mismatch in Extension.";
        for (size_t i = 0; i < spec.type_aliases.size(); i++)
            if (spec.type_aliases[i]) spec.type_aliases[i](e->type_aliases[i].get());
    }

    inline void ExpectStructDef(StructDefinition* s, std::string_view name,
                                std::span<const ModifierSpec> modifiers,
                                std::span<const FieldSpec> fields)
    {
        ASSERT_NE(s, nullptr) << "Expected StructDefinition node, but got nullptr.";
        EXPECT_EQ(s->name, name) << "StructDefinition name mismatch.";
        ExpectModifiers(s->modifiers, modifiers);
        ASSERT_EQ(s->fields.size(), fields.size()) << "StructDefinition fields count mismatch for struct '" << name << "'.";
        for (size_t i = 0; i < fields.size(); i++)
        {
            EXPECT_EQ(s->fields[i].name, fields[i].name.get()) << "Struct field name mismatch at index " << i << " for struct '" << name << "'.";
            ExpectModifiers(s->fields[i].modifiers, fields[i].modifiers);
            if (fields[i].type_v) fields[i].type_v(s->fields[i].type.get());
        }
    }

    inline void ExpectEnumDef(EnumDefinition* e, std::string_view name,
                              std::span<const ModifierSpec> modifiers,
                              const TypeVerifier& und_v,
                              std::span<const EnumCaseSpec> cases)
    {
        ASSERT_NE(e, nullptr) << "Expected EnumDefinition node, but got nullptr.";
        EXPECT_EQ(e->name, name) << "EnumDefinition name mismatch.";
        ExpectModifiers(e->modifiers, modifiers);
        if (und_v) und_v(e->underlying_type.get());
        ASSERT_EQ(e->cases.size(), cases.size()) << "EnumDefinition cases count mismatch for enum '" << name << "'.";
        for (size_t i = 0; i < cases.size(); i++)
        {
            EXPECT_EQ(e->cases[i].name, cases[i].name.get()) << "Enum case name mismatch at index " << i << " for enum '" << name << "'.";
            ExpectModifiers(e->cases[i].modifiers, cases[i].modifiers);
            if (cases[i].value_v) cases[i].value_v(e->cases[i].value.get());
        }
    }

    inline void ExpectTypeAlias(TypeAliasDefinition* a, std::string_view name,
                                std::span<const ModifierSpec> modifiers,
                                const TypeVerifier& target_v)
    {
        ASSERT_NE(a, nullptr) << "Expected TypeAliasDefinition node, but got nullptr.";
        EXPECT_EQ(a->name, name) << "TypeAliasDefinition name mismatch.";
        ExpectModifiers(a->modifiers, modifiers);
        if (target_v) target_v(a->target_type.get());
    }

    inline void ExpectImport(ImportStatement* imp,
                             std::span<const ModifierSpec> modifiers,
                             std::string_view path)
    {
        ASSERT_NE(imp, nullptr) << "Expected ImportStatement node, but got nullptr.";
        ExpectModifiers(imp->modifiers, modifiers);
        EXPECT_EQ(imp->path, path) << "ImportStatement path mismatch.";
    }

    template <ExprMatcher V>
    inline void ExpectDirective(Directive* dir, std::string_view name, const V& val_v)
    {
        ASSERT_NE(dir, nullptr) << "Expected Directive node, but got nullptr.";
        EXPECT_EQ(dir->name, name) << "Directive name mismatch.";
        if (val_v) val_v(dir->value.get());
    }

    inline void ExpectProgram(const Program* p, const ProgramSpec& spec)
    {
        ASSERT_NE(p, nullptr) << "Expected Program node, but got nullptr.";

        ASSERT_EQ(p->import_statements.size(), spec.imports.size()) << "Program import count mismatch.";
        for (size_t i = 0; i < spec.imports.size(); i++)
        {
            if (spec.imports[i]) spec.imports[i](p->import_statements[i].get());
        }

        ASSERT_EQ(p->directives.size(), spec.directives.size()) << "Program directive count mismatch.";
        for (size_t i = 0; i < spec.directives.size(); i++)
        {
            if (spec.directives[i]) spec.directives[i](p->directives[i].get());
        }

        ASSERT_EQ(p->execution_steps.size(), spec.execution_steps.size()) << "Program execution step count mismatch.";
        for (size_t i = 0; i < spec.execution_steps.size(); i++)
        {
            if (spec.execution_steps[i]) spec.execution_steps[i](p->execution_steps[i].get());
        }

        ASSERT_EQ(p->function_definitions.size(), spec.functions.size()) << "Program function count mismatch.";
        for (size_t i = 0; i < spec.functions.size(); i++)
        {
            if (spec.functions[i]) spec.functions[i](p->function_definitions[i].get());
        }

        ASSERT_EQ(p->struct_definitions.size(), spec.structs.size()) << "Program struct count mismatch.";
        for (size_t i = 0; i < spec.structs.size(); i++)
        {
            if (spec.structs[i]) spec.structs[i](p->struct_definitions[i].get());
        }

        ASSERT_EQ(p->enum_definitions.size(), spec.enums.size()) << "Program enum count mismatch.";
        for (size_t i = 0; i < spec.enums.size(); i++)
        {
            if (spec.enums[i]) spec.enums[i](p->enum_definitions[i].get());
        }

        ASSERT_EQ(p->type_aliases.size(), spec.type_aliases.size()) << "Program type alias count mismatch.";
        for (size_t i = 0; i < spec.type_aliases.size(); i++)
        {
            if (spec.type_aliases[i]) spec.type_aliases[i](p->type_aliases[i].get());
        }
    }

    inline NullVerifier IsNull() { return NullVerifier{}; }
    inline NullVerifier IsNullType() { return NullVerifier{}; }

    template <void (*ExpectFn)(AstNode*, std::string_view)>
    struct SingleValueMatcher
    {
        using node_type = Expression;
        StringStorage value;
        void operator()(Expression* node) const { ExpectFn(node, value.get()); }
    };

    using NumberMatcher = SingleValueMatcher<ExpectNumber>;
    using StringMatcher = SingleValueMatcher<ExpectString>;
    using PercentageMatcher = SingleValueMatcher<ExpectPercentage>;
    using IdentifierMatcher = SingleValueMatcher<ExpectIdentifier>;

    inline ExprVerifier IsNumber(StringStorage value)
    {
        return ExprVerifier(NumberMatcher{std::move(value)});
    }

    inline ExprVerifier IsString(StringStorage val)
    {
        return ExprVerifier(StringMatcher{std::move(val)});
    }

    struct BooleanMatcher
    {
        using node_type = Expression;
        bool value;
        void operator()(Expression* node) const { ExpectBoolean(node, value); }
    };

    inline ExprVerifier IsBoolean(bool val) { return ExprVerifier(BooleanMatcher{val}); }

    inline ExprVerifier IsPercentage(StringStorage val)
    {
        return ExprVerifier(PercentageMatcher{std::move(val)});
    }

    inline ExprVerifier IsIdentifier(StringStorage val)
    {
        return ExprVerifier(IdentifierMatcher{std::move(val)});
    }

    struct SelfMatcher
    {
        using node_type = Expression;
        void operator()(Expression* node) const { ExpectSelf(node); }
    };

    inline ExprVerifier IsSelf() { return ExprVerifier(SelfMatcher{}); }

    template <ExprMatcher L = AnyMatcher, ExprMatcher R = AnyMatcher>
    struct BinaryMatcher
    {
        using node_type = Expression;
        TokenType op;
        MatcherStorage<L> left_v;
        MatcherStorage<R> right_v;

        void operator()(Expression* node) const
        {
            if (auto b = ExpectNode<BinaryExpression>(node))
            {
                EXPECT_EQ(b->op, op) << "Binary expression operator mismatch.";
                left_v(b->left.get());
                right_v(b->right.get());
            }
        }
    };

    template <ExprMatcher L = AnyMatcher, ExprMatcher R = AnyMatcher>
    inline ExprVerifier IsBinary(TokenType op, L&& l = {}, R&& r = {})
    {
        return ExprVerifier(BinaryMatcher<std::decay_t<L>, std::decay_t<R>>{
            op, std::forward<L>(l), std::forward<R>(r)
        });
    }

    template <ExprMatcher R = AnyMatcher>
    struct UnaryMatcher
    {
        using node_type = Expression;
        TokenType op;
        MatcherStorage<R> right_v;

        void operator()(Expression* node) const
        {
            if (auto u = ExpectNode<UnaryExpression>(node))
            {
                EXPECT_EQ(u->op, op) << "Unary expression operator mismatch.";
                right_v(u->right.get());
            }
        }
    };

    template <ExprMatcher R = AnyMatcher>
    inline ExprVerifier IsUnary(TokenType op, R&& r = {})
    {
        return ExprVerifier(UnaryMatcher<std::decay_t<R>>{op, std::forward<R>(r)});
    }

    template <ExprMatcher I = AnyMatcher>
    struct GroupingMatcher
    {
        using node_type = Expression;
        MatcherStorage<I> inner_v;

        void operator()(Expression* node) const
        {
            if (auto g = ExpectNode<GroupingExpression>(node))
            {
                inner_v(g->expression.get());
            }
        }
    };

    template <ExprMatcher I = AnyMatcher>
    inline ExprVerifier IsGrouping(I&& inner = {})
    {
        return ExprVerifier(GroupingMatcher<std::decay_t<I>>{std::forward<I>(inner)});
    }

    template <ExprMatcher C = AnyMatcher, ExprMatcher T = AnyMatcher, ExprMatcher E = AnyMatcher>
    struct ConditionalMatcher
    {
        using node_type = Expression;
        MatcherStorage<C> cond_v;
        MatcherStorage<T> then_v;
        MatcherStorage<E> else_v;

        void operator()(Expression* node) const
        {
            if (auto cond = ExpectNode<ConditionalExpression>(node))
            {
                cond_v(cond->condition.get());
                then_v(cond->then_branch.get());
                else_v(cond->else_branch.get());
            }
        }
    };

    template <ExprMatcher C = AnyMatcher, ExprMatcher T = AnyMatcher, ExprMatcher E = AnyMatcher>
    inline ExprVerifier IsConditional(C&& condition = {}, T&& then_expr = {}, E&& else_expr = {})
    {
        return ExprVerifier(ConditionalMatcher<std::decay_t<C>, std::decay_t<T>, std::decay_t<E>>{
            std::forward<C>(condition), std::forward<T>(then_expr), std::forward<E>(else_expr)
        });
    }

    template <ExprMatcher T = AnyMatcher>
    struct CallMatcher
    {
        using node_type = Expression;
        MatcherStorage<T> target_v;
        std::vector<ArgSpec> args;

        void operator()(Expression* node) const
        {
            if (auto c = ExpectNode<FunctionCall>(node))
            {
                target_v(c->target.get());
                ExpectArguments(c->arguments, args);
            }
        }
    };

    template <typename T, typename U>
    ExprVerifier IsCall(T&&, std::initializer_list<U>) = delete;

    template <ExprMatcher T = AnyMatcher>
    inline ExprVerifier IsCall(T&& target, std::vector<ArgSpec> args = {})
    {
        return ExprVerifier(CallMatcher<std::decay_t<T>>{std::forward<T>(target), std::move(args)});
    }

    template <ExprMatcher T = AnyMatcher, typename... ArgSpecs>
        requires (sizeof...(ArgSpecs) > 0 && !(sizeof...(ArgSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<ArgSpecs...>>>, std::vector<ArgSpec>>))
    inline ExprVerifier IsCall(T&& target, ArgSpecs&&... args)
    {
        std::vector<ArgSpec> arg_list = {std::forward<ArgSpecs>(args)...};
        return ExprVerifier(CallMatcher<std::decay_t<T>>{std::forward<T>(target), std::move(arg_list)});
    }

    template <ExprMatcher T = AnyMatcher, ExprMatcher I = AnyMatcher>
    struct BracketMatcher
    {
        using node_type = Expression;
        MatcherStorage<T> target_v;
        MatcherStorage<I> index_v;

        void operator()(Expression* node) const
        {
            if (auto b = ExpectNode<BracketAccess>(node))
            {
                target_v(b->target.get());
                index_v(b->index.get());
            }
        }
    };

    template <ExprMatcher T = AnyMatcher, ExprMatcher I = AnyMatcher>
    inline ExprVerifier IsBracket(T&& target, I&& index)
    {
        return ExprVerifier(BracketMatcher<std::decay_t<T>, std::decay_t<I>>{
            std::forward<T>(target), std::forward<I>(index)
        });
    }

    template <ExprMatcher T = AnyMatcher>
    struct DotMatcher
    {
        using node_type = Expression;
        MatcherStorage<T> target_v;
        StringStorage property;

        void operator()(Expression* node) const
        {
            if (auto d = ExpectNode<DotAccess>(node))
            {
                target_v(d->target.get());
                EXPECT_EQ(d->property_name, property.get()) << "Dot access property name mismatch.";
            }
        }
    };

    template <ExprMatcher T = AnyMatcher>
    inline ExprVerifier IsDot(T&& target, StringStorage property)
    {
        return ExprVerifier(DotMatcher<std::decay_t<T>>{std::forward<T>(target), std::move(property)});
    }

    template <ExprMatcher T = AnyMatcher, ExprMatcher D = AnyMatcher>
    struct SwitchMatcher
    {
        using node_type = Expression;
        MatcherStorage<T> target_v;
        std::vector<SwitchCaseSpec> cases;
        std::vector<ModifierSpec> default_mods;
        MatcherStorage<D> default_v;

        void operator()(Expression* node) const
        {
            ExpectSwitch(node, target_v, cases, default_mods, default_v);
        }
    };

    template <typename T, typename U>
    ExprVerifier IsSwitch(T&&, std::initializer_list<U>) = delete;

    template <ExprMatcher T = AnyMatcher, ExprMatcher D = AnyMatcher>
    inline ExprVerifier IsSwitch(T&& t, std::vector<SwitchCaseSpec> cases, std::vector<ModifierSpec> default_mods,
                                 D&& default_expr = {})
    {
        return ExprVerifier(SwitchMatcher<std::decay_t<T>, std::decay_t<D>>{
            std::forward<T>(t), std::move(cases), std::move(default_mods), std::forward<D>(default_expr)
        });
    }

    template <ExprMatcher T = AnyMatcher, ExprMatcher D = AnyMatcher>
    inline ExprVerifier IsSwitch(T&& t, std::vector<SwitchCaseSpec> cases, D&& default_expr = {})
    {
        return IsSwitch(std::forward<T>(t), std::move(cases), {}, std::forward<D>(default_expr));
    }

    template <ExprMatcher T = AnyMatcher, typename... Cases>
        requires (sizeof...(Cases) > 0 && !(sizeof...(Cases) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Cases...>>>, std::vector<SwitchCaseSpec>>))
    inline ExprVerifier IsSwitch(T&& t, Cases&&... cases)
    {
        std::vector<SwitchCaseSpec> case_list = {std::forward<Cases>(cases)...};
        return ExprVerifier(SwitchMatcher<std::decay_t<T>, AnyMatcher>{
            std::forward<T>(t), std::move(case_list), {}, AnyMatcher{}
        });
    }

    template <typename ASTNodeT, typename... Matchers>
    struct SequenceVariadicMatcher
    {
        using node_type = Expression;
        std::tuple<Matchers...> elements;

        void operator()(Expression* node) const
        {
            if (auto t = ExpectNode<ASTNodeT>(node))
            {
                ASSERT_EQ(t->elements.size(), sizeof...(Matchers)) << get_demangled_name(typeid(ASTNodeT).name()) <<
 " elements count mismatch.";
                size_t idx = 0;
                std::apply([&](const auto&... m)
                {
                    ((m(t->elements[idx++].get())), ...);
                }, elements);
            }
        }
    };

    template <typename... Matchers>
    using TensorVariadicMatcher = SequenceVariadicMatcher<TensorLiteral, Matchers...>;

    template <typename T = ExprVerifier>
    ExprVerifier IsTensor(std::initializer_list<T>) = delete;

    struct TensorVectorMatcher
    {
        using node_type = Expression;
        std::vector<ExprVerifier> elements;

        void operator()(Expression* node) const
        {
            ExpectTensor(node, elements);
        }
    };

    inline ExprVerifier IsTensor(std::vector<ExprVerifier> elements)
    {
        return ExprVerifier(TensorVectorMatcher{std::move(elements)});
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<ExprVerifier>>))
    inline ExprVerifier IsTensor(Matchers&&... matchers)
    {
        return ExprVerifier(TensorVariadicMatcher<std::decay_t<Matchers>...>{
            std::make_tuple(std::forward<Matchers>(matchers)...)
        });
    }

    inline ExprVerifier IsTensor()
    {
        return ExprVerifier(TensorVectorMatcher{});
    }

    template <typename... Matchers>
    using TupleVariadicMatcher = SequenceVariadicMatcher<TupleLiteral, Matchers...>;

    template <typename T = ExprVerifier>
    ExprVerifier IsTuple(std::initializer_list<T>) = delete;

    struct TupleVectorMatcher
    {
        using node_type = Expression;
        std::vector<ExprVerifier> elements;

        void operator()(Expression* node) const
        {
            ExpectTuple(node, elements);
        }
    };

    inline ExprVerifier IsTuple(std::vector<ExprVerifier> elements)
    {
        return ExprVerifier(TupleVectorMatcher{std::move(elements)});
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<ExprVerifier>>))
    inline ExprVerifier IsTuple(Matchers&&... matchers)
    {
        return ExprVerifier(TupleVariadicMatcher<std::decay_t<Matchers>...>{
            std::make_tuple(std::forward<Matchers>(matchers)...)
        });
    }

    inline ExprVerifier IsTuple()
    {
        return ExprVerifier(TupleVectorMatcher{});
    }

    template <typename T = DictItemSpec>
    ExprVerifier IsDict(std::initializer_list<T>) = delete;

    template <typename... Matchers>
    struct DictVariadicMatcher
    {
        using node_type = Expression;
        std::tuple<Matchers...> items;

        void operator()(Expression* node) const
        {
            std::vector<DictItemSpec> item_vec;
            item_vec.reserve(sizeof...(Matchers));
            std::apply([&](const auto&... m)
            {
                (item_vec.push_back(DictItemSpec(m)), ...);
            }, items);
            ExpectDict(node, item_vec);
        }
    };

    struct DictMatcher
    {
        using node_type = Expression;
        std::vector<DictItemSpec> items;
        void operator()(Expression* node) const { ExpectDict(node, items); }
    };

    inline ExprVerifier IsDict(std::vector<DictItemSpec> items)
    {
        return ExprVerifier(DictMatcher{std::move(items)});
    }

    inline ExprVerifier IsDict()
    {
        return ExprVerifier(DictMatcher{});
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<DictItemSpec>>))
    inline ExprVerifier IsDict(Matchers&&... matchers)
    {
        return ExprVerifier(DictVariadicMatcher<std::decay_t<Matchers>...>{
            std::make_tuple(std::forward<Matchers>(matchers)...)
        });
    }

    template <typename T = TypeVerifier>
    TypeVerifier IsType(StringStorage name, std::initializer_list<T>) = delete;

    template <typename... Matchers>
    struct TypeVariadicMatcher
    {
        using node_type = TypeAnnotation;
        StringStorage name;
        std::tuple<Matchers...> generics;

        void operator()(TypeAnnotation* t) const
        {
            std::vector<TypeVerifier> gen_vec;
            gen_vec.reserve(sizeof...(Matchers));
            std::apply([&](const auto&... m)
            {
                (gen_vec.push_back(TypeVerifier(m)), ...);
            }, generics);
            ExpectType(t, name.get(), gen_vec);
        }
    };

    struct TypeMatcher
    {
        using node_type = TypeAnnotation;
        StringStorage name;
        std::vector<TypeVerifier> generics;
        void operator()(TypeAnnotation* t) const { ExpectType(t, name.get(), generics); }
    };

    inline TypeVerifier IsType(StringStorage name, std::vector<TypeVerifier> generics = {})
    {
        return TypeVerifier(TypeMatcher{std::move(name), std::move(generics)});
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<TypeVerifier>>))
    inline TypeVerifier IsType(StringStorage name, Matchers&&... matchers)
    {
        return TypeVerifier(TypeVariadicMatcher<std::decay_t<Matchers>...>{
            std::move(name), std::make_tuple(std::forward<Matchers>(matchers)...)
        });
    }

    template <typename T = TypeVerifier>
    TypeVerifier IsTupleType(std::initializer_list<T>) = delete;

    template <typename... Matchers>
    struct TupleTypeVariadicMatcher
    {
        using node_type = TypeAnnotation;
        std::tuple<Matchers...> elements;

        void operator()(TypeAnnotation* node) const
        {
            if (auto t = ExpectNode<TupleTypeAnnotation>(node))
            {
                ASSERT_EQ(t->element_types.size(), sizeof...(Matchers)) << "TupleTypeAnnotation element count mismatch.";
                size_t idx = 0;
                std::apply([&](const auto&... m)
                {
                    ((m(t->element_types[idx++].get())), ...);
                }, elements);
            }
        }
    };

    struct TupleTypeMatcher
    {
        using node_type = TypeAnnotation;
        std::vector<TypeVerifier> elements;
        void operator()(TypeAnnotation* t) const { ExpectTupleType(t, elements); }
    };

    inline TypeVerifier IsTupleType(std::vector<TypeVerifier> elements = {})
    {
        return TypeVerifier(TupleTypeMatcher{std::move(elements)});
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<TypeVerifier>>))
    inline TypeVerifier IsTupleType(Matchers&&... matchers)
    {
        return TypeVerifier(TupleTypeVariadicMatcher<std::decay_t<Matchers>...>{
            std::make_tuple(std::forward<Matchers>(matchers)...)
        });
    }

    template <typename V = AnyMatcher>
    struct AssignmentMatcher
    {
        using node_type = Assignment;
        std::vector<AssignmentTargetSpec> targets;
        MatcherStorage<V> value;

        void operator()(Statement* s) const
        {
            if (auto a = ExpectNode<Assignment>(s))
            {
                ASSERT_EQ(a->targets.size(), targets.size()) << "Assignment targets count mismatch.";
                for (size_t i = 0; i < targets.size(); i++)
                {
                    ExpectModifiers(a->targets[i].modifiers, targets[i].modifiers);
                    EXPECT_EQ(a->targets[i].name, targets[i].name.get()) << "Assignment target name mismatch at index "
 << i << ".";
                    if (targets[i].type_v) targets[i].type_v(a->targets[i].type.get());
                }
                value(a->value.get());
            }
        }
    };

    template <typename V = AnyMatcher>
    inline AssignmentVerifier IsAssignment(std::vector<AssignmentTargetSpec> targets, V&& value = {})
    {
        return AssignmentVerifier(AssignmentMatcher<std::decay_t<V>>{std::move(targets), std::forward<V>(value)});
    }

    template <typename T = AnyMatcher, typename V = AnyMatcher>
    struct ReassignmentMatcher
    {
        using node_type = Reassignment;
        MatcherStorage<T> target_v;
        MatcherStorage<V> val_v;

        void operator()(Statement* s) const
        {
            if (auto r = ExpectNode<Reassignment>(s))
            {
                target_v(r->target.get());
                val_v(r->value.get());
            }
        }
    };

    template <typename T = AnyMatcher, typename V = AnyMatcher>
    inline ReassignmentVerifier IsReassignment(T&& target = {}, V&& value = {})
    {
        return ReassignmentVerifier(ReassignmentMatcher<std::decay_t<T>, std::decay_t<V>>{
            std::forward<T>(target), std::forward<V>(value)
        });
    }

    template <typename T = ExprVerifier>
    ReturnVerifier IsReturn(std::initializer_list<T>) = delete;

    template <typename... Matchers>
    struct ReturnVariadicMatcher
    {
        using node_type = ReturnStatement;
        std::tuple<Matchers...> values;

        void operator()(Statement* s) const
        {
            if (auto r = ExpectNode<ReturnStatement>(s))
            {
                ASSERT_EQ(r->values.size(), sizeof...(Matchers)) << "Return values count mismatch.";
                size_t idx = 0;
                std::apply([&](const auto&... m)
                {
                    ((m(r->values[idx++].get())), ...);
                }, values);
            }
        }
    };

    struct ReturnMatcher
    {
        using node_type = ReturnStatement;
        std::vector<ModifierSpec> modifiers;
        std::vector<ExprVerifier> values;

        void operator()(Statement* s) const
        {
            if (auto r = ExpectNode<ReturnStatement>(s))
            {
                ExpectReturn(r, modifiers, values);
            }
        }
    };

    inline ReturnVerifier IsReturn(std::vector<ModifierSpec> modifiers, std::vector<ExprVerifier> values)
    {
        return ReturnVerifier(ReturnMatcher{std::move(modifiers), std::move(values)});
    }

    inline ReturnVerifier IsReturn(std::vector<ExprVerifier> values = {})
    {
        return IsReturn({}, std::move(values));
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<ExprVerifier>>))
    inline ReturnVerifier IsReturn(Matchers&&... matchers)
    {
        return ReturnVerifier(ReturnVariadicMatcher<std::decay_t<Matchers>...>{
            std::make_tuple(std::forward<Matchers>(matchers)...)
        });
    }

    template <typename E = AnyMatcher>
    struct ExprStmtMatcher
    {
        using node_type = ExpressionStatement;
        MatcherStorage<E> expr_v;

        void operator()(Statement* s) const
        {
            if (auto es = ExpectNode<ExpressionStatement>(s))
            {
                expr_v(es->expr.get());
            }
        }
    };

    template <typename E = AnyMatcher>
    inline ExprStmtVerifier IsExprStmt(E&& expr = {})
    {
        return ExprStmtVerifier(ExprStmtMatcher<std::decay_t<E>>{std::forward<E>(expr)});
    }

    struct FunctionDefMatcher
    {
        using node_type = FunctionDefinition;
        StringStorage name;
        std::vector<ModifierSpec> modifiers;
        std::vector<ParamSpec> params;
        std::vector<TypeVerifier> returns;
        std::vector<StmtVerifier> body;
        std::optional<StringStorage> docstring;

        void operator()(FunctionDefinition* f) const
        {
            ExpectFunctionDef(f, name.get(), modifiers, params, returns, body, docstring);
        }
    };

    inline FuncVerifier IsFunctionDef(StringStorage name,
                                      std::vector<ModifierSpec> modifiers = {},
                                      std::vector<ParamSpec> params = {},
                                      std::vector<TypeVerifier> returns = {},
                                      std::vector<StmtVerifier> body = {},
                                      std::optional<StringStorage> docstring = std::nullopt)
    {
        return FuncVerifier(FunctionDefMatcher{
            std::move(name), std::move(modifiers), std::move(params),
            std::move(returns), std::move(body), std::move(docstring)
        });
    }

    struct StructDefMatcher
    {
        using node_type = StructDefinition;
        StringStorage name;
        std::vector<ModifierSpec> modifiers;
        std::vector<FieldSpec> fields;

        void operator()(StructDefinition* s) const
        {
            ExpectStructDef(s, name.get(), modifiers, fields);
        }
    };

    template <typename T>
    StructVerifier IsStructDef(StringStorage, std::initializer_list<T>) = delete;
    template <typename T>
    StructVerifier IsStructDef(StringStorage, std::vector<ModifierSpec>, std::initializer_list<T>) = delete;

    inline StructVerifier IsStructDef(StringStorage name, std::vector<ModifierSpec> modifiers = {},
                                      std::vector<FieldSpec> fields = {})
    {
        return StructVerifier(StructDefMatcher{std::move(name), std::move(modifiers), std::move(fields)});
    }

    template <typename... FieldSpecs>
        requires (sizeof...(FieldSpecs) > 0 && !(sizeof...(FieldSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<FieldSpecs...>>>, std::vector<FieldSpec>>))
    inline StructVerifier IsStructDef(StringStorage name, FieldSpecs&&... fields)
    {
        std::vector<FieldSpec> field_list = {std::forward<FieldSpecs>(fields)...};
        return StructVerifier(StructDefMatcher{std::move(name), {}, std::move(field_list)});
    }

    template <typename... FieldSpecs>
        requires (sizeof...(FieldSpecs) > 0 && !(sizeof...(FieldSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<FieldSpecs...>>>, std::vector<FieldSpec>>))
    inline StructVerifier IsStructDef(StringStorage name, std::vector<ModifierSpec> modifiers, FieldSpecs&&... fields)
    {
        std::vector<FieldSpec> field_list = {std::forward<FieldSpecs>(fields)...};
        return StructVerifier(StructDefMatcher{std::move(name), std::move(modifiers), std::move(field_list)});
    }

    struct EnumDefMatcher
    {
        using node_type = EnumDefinition;
        StringStorage name;
        std::vector<ModifierSpec> modifiers;
        TypeVerifier type;
        std::vector<EnumCaseSpec> cases;

        void operator()(EnumDefinition* e) const
        {
            ExpectEnumDef(e, name.get(), modifiers, type, cases);
        }
    };

    template <typename T>
    EnumVerifier IsEnumDef(StringStorage, std::initializer_list<T>) = delete;
    template <typename T>
    EnumVerifier IsEnumDef(StringStorage, std::vector<ModifierSpec>, TypeVerifier, std::initializer_list<T>) = delete;

    inline EnumVerifier IsEnumDef(StringStorage name, std::vector<ModifierSpec> modifiers = {},
                                  TypeVerifier type = nullptr,
                                  std::vector<EnumCaseSpec> cases = {})
    {
        return EnumVerifier(EnumDefMatcher{std::move(name), std::move(modifiers), std::move(type), std::move(cases)});
    }

    template <typename... EnumCaseSpecs>
        requires (sizeof...(EnumCaseSpecs) > 0 && !(sizeof...(EnumCaseSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<EnumCaseSpecs...>>>, std::vector<EnumCaseSpec>>))
    inline EnumVerifier IsEnumDef(StringStorage name, EnumCaseSpecs&&... cases)
    {
        std::vector<EnumCaseSpec> case_list = {std::forward<EnumCaseSpecs>(cases)...};
        return EnumVerifier(EnumDefMatcher{std::move(name), {}, nullptr, std::move(case_list)});
    }

    template <typename... EnumCaseSpecs>
        requires (sizeof...(EnumCaseSpecs) > 0 && !(sizeof...(EnumCaseSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<EnumCaseSpecs...>>>, std::vector<EnumCaseSpec>>))
    inline EnumVerifier IsEnumDef(StringStorage name, std::vector<ModifierSpec> modifiers, TypeVerifier type, EnumCaseSpecs&&... cases)
    {
        std::vector<EnumCaseSpec> case_list = {std::forward<EnumCaseSpecs>(cases)...};
        return EnumVerifier(EnumDefMatcher{std::move(name), std::move(modifiers), std::move(type), std::move(case_list)});
    }

    struct TypeAliasMatcher
    {
        using node_type = TypeAliasDefinition;
        StringStorage name;
        std::vector<ModifierSpec> modifiers;
        TypeVerifier target;

        void operator()(TypeAliasDefinition* a) const
        {
            ExpectTypeAlias(a, name.get(), modifiers, target);
        }
    };

    inline AliasVerifier IsTypeAlias(StringStorage name, std::vector<ModifierSpec> modifiers = {},
                                     TypeVerifier target = nullptr)
    {
        return AliasVerifier(TypeAliasMatcher{std::move(name), std::move(modifiers), std::move(target)});
    }

    struct ExtensionDefMatcher
    {
        using node_type = ExtensionDefinition;
        std::vector<ModifierSpec> modifiers;
        TypeVerifier target;
        ProgramSpec spec;

        void operator()(ExtensionDefinition* e) const
        {
            ExpectExtensionDef(e, modifiers, target, spec);
        }
    };

    inline ExtVerifier IsExtensionDef(std::vector<ModifierSpec> modifiers = {},
                                      TypeVerifier target = nullptr,
                                      ProgramSpec spec = {})
    {
        return ExtVerifier(ExtensionDefMatcher{std::move(modifiers), std::move(target), std::move(spec)});
    }

    struct ImportMatcher
    {
        using node_type = ImportStatement;
        StringStorage path;
        std::vector<ModifierSpec> modifiers;

        void operator()(ImportStatement* i) const
        {
            ExpectImport(i, modifiers, path.get());
        }
    };

    inline ImportVerifier IsImport(StringStorage path, std::vector<ModifierSpec> modifiers = {})
    {
        return ImportVerifier(ImportMatcher{std::move(path), std::move(modifiers)});
    }

    template <typename V = AnyMatcher>
    struct DirectiveMatcher
    {
        using node_type = Directive;
        StringStorage name;
        MatcherStorage<V> value;

        void operator()(Directive* d) const
        {
            if (auto dir = ExpectNode<Directive>(d))
            {
                EXPECT_EQ(dir->name, name.get()) << "Directive name mismatch.";
                value(dir->value.get());
            }
        }
    };

    template <typename V = AnyMatcher>
    inline DirectiveVerifier IsDirective(StringStorage name, V&& value = {})
    {
        return DirectiveVerifier(DirectiveMatcher<std::decay_t<V>>{std::move(name), std::forward<V>(value)});
    }

    inline ProgramSpec MergeSpecs(ProgramSpec base, ProgramSpec extension)
    {
        base.imports.insert(base.imports.end(), extension.imports.begin(), extension.imports.end());
        base.directives.insert(base.directives.end(), extension.directives.begin(), extension.directives.end());
        base.execution_steps.insert(base.execution_steps.end(), extension.execution_steps.begin(),
                                    extension.execution_steps.end());
        base.functions.insert(base.functions.end(), extension.functions.begin(), extension.functions.end());
        base.structs.insert(base.structs.end(), extension.structs.begin(), extension.structs.end());
        base.enums.insert(base.enums.end(), extension.enums.begin(), extension.enums.end());
        base.type_aliases.insert(base.type_aliases.end(), extension.type_aliases.begin(), extension.type_aliases.end());
        base.extensions.insert(base.extensions.end(), extension.extensions.begin(), extension.extensions.end());
        return base;
    }
}
