#pragma once

#include "core.h"

namespace valuascript::compiler::test
{
    struct ModifierSpec;

    using ModifierVerifier = std::vector<ModifierSpec>;

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
