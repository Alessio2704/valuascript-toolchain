#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include "frontend/parser/helpers/node_matchers.h"
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/helpers/recovery_sentinel.h"
#include "core/valuascript_exception.h"

namespace valuascript::compiler::test
{
    struct InvalidDeclarationConstructCase
    {
        std::string name;
        std::string code;
        UniversalVerifier verifier;
        InjectableType type;
        bool is_broken = false;
        std::vector<ValuascriptErrorCode> suppressed_errors = {};
        std::vector<std::string_view> skip_contexts = {};
    };

    struct InvalidDeclarationInBlockTestCase
    {
        std::string test_name;
        Context context;
        InvalidDeclarationConstructCase construct_case;
    };

    inline bool is_disallowed_placement(BlockContext block_ctx, InjectableType construct_type)
    {
        switch (block_ctx)
        {
        case BlockContext::FunctionBody:
            return construct_type == InjectableType::Function ||
                   construct_type == InjectableType::Struct ||
                   construct_type == InjectableType::Enum ||
                   construct_type == InjectableType::Extension ||
                   construct_type == InjectableType::TypeAlias ||
                   construct_type == InjectableType::Import ||
                   construct_type == InjectableType::Directive;
        case BlockContext::ExtensionBody:
            return construct_type == InjectableType::Import ||
                   construct_type == InjectableType::Directive ||
                   construct_type == InjectableType::WeakStatement ||
                   construct_type == InjectableType::Extension;
        case BlockContext::TopLevel:
            return construct_type == InjectableType::WeakStatement;
        default:
            return false;
        }
    }

    inline bool should_test_construct_in_context(const Context& ctx, const InvalidDeclarationConstructCase& construct)
    {
        bool is_skipped_for_ctx = (std::find(construct.skip_contexts.begin(), construct.skip_contexts.end(),
                                             ctx.name) != construct.skip_contexts.end());
        if (is_skipped_for_ctx)
        {
            return false;
        }

        if (is_nested_block_context(ctx.block_context) && construct.is_broken &&
            has_unclosed_brace(construct.code))
        {
            return false;
        }

        if (construct.is_broken && !is_valid_declaration_keyword(construct.type, construct.code))
        {
            return false;
        }

        return is_disallowed_placement(ctx.block_context, construct.type);
    }

    inline ConstructedRecoveryProgram build_invalid_declaration_program(const Context& ctx,
                                                                         const InvalidDeclarationConstructCase& construct,
                                                                         size_t seed)
    {
        std::vector<RecoveryBlock> pre, post;
        std::string inner_code = construct.code;

        if (ctx.block_context != BlockContext::TopLevel)
        {
            pre.push_back(RecoverySentinel::generate_block_sentinel(seed, ctx.block_context, {}, {}));
            post.push_back(RecoverySentinel::generate_block_sentinel(seed + 1, ctx.block_context, {}, {}));
            inner_code = pre[0].source + "\n  " + construct.code + "\n  " + post[0].source;
        }

        std::string wrapped_code = ctx.prefix + inner_code + ctx.suffix;

        UniversalVerifier inner_verifier = UniversalVerifier(NullVerifier{});
        if ((ctx.block_context == BlockContext::ExtensionBody || ctx.block_context == BlockContext::TopLevel) &&
            construct.type == InjectableType::WeakStatement)
        {
            inner_verifier = construct.verifier;
        }

        UniversalVerifier expected_v;
        if (ctx.transform_verifier_block)
        {
            expected_v = ctx.transform_verifier_block(inner_verifier, pre, post);
        }
        else if (ctx.transform_verifier)
        {
            expected_v = ctx.transform_verifier(inner_verifier);
        }
        else
        {
            expected_v = inner_verifier;
        }

        ProgramSpec inner_spec;
        std::visit([&](auto&& ver)
        {
            using V = std::decay_t<decltype(ver)>;
            if constexpr (std::is_same_v<V, ReturnVerifier>)
            {
                inner_spec.execution_steps.push_back(StmtVerifier(ver));
            }
            else
            {
                SpecAdder::add(inner_spec, ver);
            }
        }, expected_v);

        return ParserTestBase::BuildRecoveryProgram(wrapped_code, inner_spec, "", seed + 2);
    }

    inline SourceSpan compute_expected_span(const std::string& full_code, const std::string& construct_code, size_t search_start = 0)
    {
        SourceSpan span;
        size_t pos = full_code.find(construct_code, search_start);
        if (pos == std::string::npos) return span;

        size_t current_line = 1;
        size_t current_col = 1;

        for (size_t i = 0; i < pos; ++i)
        {
            if (full_code[i] == '\n')
            {
                current_line++;
                current_col = 1;
            }
            else
            {
                current_col++;
            }
        }

        span.line_start = current_line;
        span.column_start = current_col;

        size_t end_pos = pos + construct_code.length();
        for (size_t i = pos; i < end_pos; ++i)
        {
            if (full_code[i] == '\n')
            {
                current_line++;
                current_col = 1;
            }
            else
            {
                current_col++;
            }
        }

        span.line_end = current_line;
        span.column_end = (current_col > 1) ? (current_col - 1) : 1;

        return span;
    }

    std::vector<InvalidDeclarationInBlockTestCase> GenerateInvalidDeclarationInBlockTestCases();
}
