#pragma once

#include "core.h"

namespace valuascript::compiler::test
{
    struct ModifierSpec;

    using ModifierVerifier = std::vector<ModifierSpec>;

    struct ArgSpec : public SpanSpecMixin<ArgSpec>
    {
        std::string label;
        ExprVerifier value_v = nullptr;
    };

    struct ModifierSpec : public SpanSpecMixin<ModifierSpec>
    {
        std::string name;
        std::vector<ArgSpec> args = {};
    };

    struct ParamSpec : public SpanSpecMixin<ParamSpec>
    {
        std::string name;
        std::vector<ModifierSpec> modifiers = {};
        TypeVerifier type_v = nullptr;
        ExprVerifier default_v = nullptr;
    };

    struct FieldSpec : public SpanSpecMixin<FieldSpec>
    {
        std::string name;
        std::vector<ModifierSpec> modifiers = {};
        TypeVerifier type_v = nullptr;
    };

    struct EnumCaseSpec : public SpanSpecMixin<EnumCaseSpec>
    {
        std::string name;
        std::vector<ModifierSpec> modifiers = {};
        ExprVerifier value_v = nullptr;
    };

    struct DictItemSpec : public SpanSpecMixin<DictItemSpec>
    {
        std::string key;
        std::vector<ModifierSpec> modifiers = {};
        ExprVerifier value_v = nullptr;
    };

    struct SwitchCaseSpec : public SpanMixin<SwitchCaseSpec>
    {
        std::vector<ModifierSpec> modifiers = {};
        std::vector<std::string> labels = {};
        ExprVerifier result_v = nullptr;
        std::vector<SourceSpan> label_spans = {};

        SwitchCaseSpec& with_label_spans(std::vector<SourceSpan> spans)
        {
            label_spans = std::move(spans);
            return *this;
        }
    };

    struct AssignmentTargetSpec : public SpanSpecMixin<AssignmentTargetSpec>
    {
        std::vector<ModifierSpec> modifiers = {};
        std::string name;
        TypeVerifier type_v = nullptr;
    };

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
                InlineVerifier<Statement>::operator=([ver = std::move(v)](Statement* s)
                {
                    if (auto* casted = ExpectNode<Assignment>(s)) [[likely]] ver(casted);
                });
            }
        }

        StmtVerifier(ReassignmentVerifier v)
        {
            if (v) [[likely]]
            {
                InlineVerifier<Statement>::operator=([ver = std::move(v)](Statement* s)
                {
                    if (auto* casted = ExpectNode<Reassignment>(s)) [[likely]] ver(casted);
                });
            }
        }

        StmtVerifier(ReturnVerifier v)
        {
            if (v) [[likely]]
            {
                InlineVerifier<Statement>::operator=([ver = std::move(v)](Statement* s)
                {
                    if (auto* casted = ExpectNode<ReturnStatement>(s)) [[likely]] ver(casted);
                });
            }
        }

        StmtVerifier(ExprStmtVerifier v)
        {
            if (v) [[likely]]
            {
                InlineVerifier<Statement>::operator=([ver = std::move(v)](Statement* s)
                {
                    if (auto* casted = ExpectNode<ExpressionStatement>(s)) [[likely]] ver(casted);
                });
            }
        }

        template <typename M>
            requires (StatementNode<typename std::decay_t<M>::node_type> &&
                      !std::same_as<typename std::decay_t<M>::node_type, Statement>)
        StmtVerifier(FluentNodeMatcher<M> fm)
            : StmtVerifier(InlineVerifier<typename std::decay_t<M>::node_type>(std::move(fm)))
        {
        }

        template <typename F>
            requires (!StatementNode<F> &&
                !std::same_as<std::decay_t<F>, StmtVerifier> &&
                !std::same_as<std::decay_t<F>, InlineVerifier<Statement>> &&
                !std::same_as<std::decay_t<F>, std::nullptr_t> &&
                !HasNodeType<F> &&
                std::invocable<F, Statement*>)
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
