#pragma once

#include "core.h"

namespace valuascript::compiler::test
{
    struct ModifierSpec;

    using ModifierVerifier = std::vector<ModifierSpec>;

    struct ArgSpec
    {
        std::string label;
        ExprVerifier value_v = nullptr;
        std::optional<SourceSpan> span = std::nullopt;
        std::optional<SourceSpan> name_span = std::nullopt;

        ArgSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        ArgSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                           size_t start_offset, size_t length)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        ArgSpec& with_span(const SourceSpan& s)
        {
            span = s;
            return *this;
        }

        ArgSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        ArgSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                size_t start_offset, size_t length)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        ArgSpec& with_name_span(const SourceSpan& s)
        {
            name_span = s;
            return *this;
        }
    };

    struct ModifierSpec
    {
        std::string name;
        std::vector<ArgSpec> args = {};
        std::optional<SourceSpan> span = std::nullopt;
        std::optional<SourceSpan> name_span = std::nullopt;

        ModifierSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        ModifierSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                size_t start_offset, size_t length)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        ModifierSpec& with_span(const SourceSpan& s)
        {
            span = s;
            return *this;
        }

        ModifierSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        ModifierSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                     size_t start_offset, size_t length)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        ModifierSpec& with_name_span(const SourceSpan& s)
        {
            name_span = s;
            return *this;
        }
    };

    struct ParamSpec
    {
        std::string name;
        std::vector<ModifierSpec> modifiers = {};
        TypeVerifier type_v = nullptr;
        ExprVerifier default_v = nullptr;
        std::optional<SourceSpan> span = std::nullopt;
        std::optional<SourceSpan> name_span = std::nullopt;

        ParamSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        ParamSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                             size_t start_offset, size_t length)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        ParamSpec& with_span(const SourceSpan& s)
        {
            span = s;
            return *this;
        }

        ParamSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        ParamSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                  size_t start_offset, size_t length)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        ParamSpec& with_name_span(const SourceSpan& s)
        {
            name_span = s;
            return *this;
        }
    };

    struct FieldSpec
    {
        std::string name;
        std::vector<ModifierSpec> modifiers = {};
        TypeVerifier type_v = nullptr;
        std::optional<SourceSpan> span = std::nullopt;
        std::optional<SourceSpan> name_span = std::nullopt;

        FieldSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        FieldSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                             size_t start_offset, size_t length)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        FieldSpec& with_span(const SourceSpan& s)
        {
            span = s;
            return *this;
        }

        FieldSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        FieldSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                  size_t start_offset, size_t length)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        FieldSpec& with_name_span(const SourceSpan& s)
        {
            name_span = s;
            return *this;
        }
    };

    struct EnumCaseSpec
    {
        std::string name;
        std::vector<ModifierSpec> modifiers = {};
        ExprVerifier value_v = nullptr;
        std::optional<SourceSpan> span = std::nullopt;
        std::optional<SourceSpan> name_span = std::nullopt;

        EnumCaseSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        EnumCaseSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                size_t start_offset, size_t length)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        EnumCaseSpec& with_span(const SourceSpan& s)
        {
            span = s;
            return *this;
        }

        EnumCaseSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        EnumCaseSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                     size_t start_offset, size_t length)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        EnumCaseSpec& with_name_span(const SourceSpan& s)
        {
            name_span = s;
            return *this;
        }
    };

    struct DictItemSpec
    {
        std::string key;
        std::vector<ModifierSpec> modifiers = {};
        ExprVerifier value_v = nullptr;
        std::optional<SourceSpan> span = std::nullopt;
        std::optional<SourceSpan> name_span = std::nullopt;

        DictItemSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        DictItemSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                size_t start_offset, size_t length)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        DictItemSpec& with_span(const SourceSpan& s)
        {
            span = s;
            return *this;
        }

        DictItemSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        DictItemSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                     size_t start_offset, size_t length)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        DictItemSpec& with_name_span(const SourceSpan& s)
        {
            name_span = s;
            return *this;
        }
    };

    struct SwitchCaseSpec
    {
        std::vector<ModifierSpec> modifiers = {};
        std::vector<std::string> labels = {};
        ExprVerifier result_v = nullptr;
        std::optional<SourceSpan> span = std::nullopt;
        std::vector<SourceSpan> label_spans = {};

        SwitchCaseSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        SwitchCaseSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                  size_t start_offset, size_t length)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        SwitchCaseSpec& with_span(const SourceSpan& s)
        {
            span = s;
            return *this;
        }

        SwitchCaseSpec& with_label_spans(std::vector<SourceSpan> spans)
        {
            label_spans = std::move(spans);
            return *this;
        }
    };

    struct AssignmentTargetSpec
    {
        std::vector<ModifierSpec> modifiers = {};
        std::string name;
        TypeVerifier type_v = nullptr;
        std::optional<SourceSpan> span = std::nullopt;
        std::optional<SourceSpan> name_span = std::nullopt;

        AssignmentTargetSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        AssignmentTargetSpec& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                        size_t start_offset, size_t length)
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        AssignmentTargetSpec& with_span(const SourceSpan& s)
        {
            span = s;
            return *this;
        }

        AssignmentTargetSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return *this;
        }

        AssignmentTargetSpec& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                             size_t start_offset, size_t length)
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return *this;
        }

        AssignmentTargetSpec& with_name_span(const SourceSpan& s)
        {
            name_span = s;
            return *this;
        }
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
                !HasNodeType<F, Statement> &&
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
