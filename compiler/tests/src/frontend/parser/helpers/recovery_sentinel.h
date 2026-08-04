#pragma once

#include <string>
#include <vector>
#include <random>
#include <functional>
#include "construct_registry.h"
#include "spec_adder.h"
#include "deterministic_sampler.h"

namespace valuascript::compiler::test
{
    struct RecoveryBlock
    {
        std::optional<SentinelKind> kind = std::nullopt;
        std::string source;
        std::function<void(ProgramSpec&)> add_to_spec;
    };

    class RecoverySentinel
    {
    private:
        template <typename T, typename AdderFunc>
        static void add_if_not_empty(std::vector<RecoveryBlock>& blocks, SentinelKind kind,
                                     const std::vector<RegistryEntry<T>>& registry,
                                     AdderFunc adder)
        {
            for (const auto& entry : registry)
            {
                blocks.push_back({
                    kind,
                    entry.code,
                    [adder, verifier = entry.verifier](ProgramSpec& spec)
                    {
                        adder(spec, verifier);
                    }
                });
            }
        }

        static std::vector<RecoveryBlock> build_function_block_pool()
        {
            std::vector<RecoveryBlock> pool;
            add_if_not_empty(pool, SentinelKind::Assignment, ConstructRegistry::assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, SentinelKind::Reassignment, ConstructRegistry::reassignments(), [](ProgramSpec& s, const ReassignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, SentinelKind::ExprStmt, ConstructRegistry::expr_stmts(), [](ProgramSpec& s, const ExprStmtVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, SentinelKind::Return, ConstructRegistry::returns(), [](ProgramSpec& s, const ReturnVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            add_if_not_empty(pool, SentinelKind::Assignment, ConstructRegistry::modified_assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, SentinelKind::Return, ConstructRegistry::modified_returns(), [](ProgramSpec& s, const ReturnVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            return pool;
        }

        static std::vector<RecoveryBlock> build_extension_block_pool()
        {
            std::vector<RecoveryBlock> pool;
            add_if_not_empty(pool, SentinelKind::Assignment, ConstructRegistry::assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, SentinelKind::Reassignment, ConstructRegistry::reassignments(), [](ProgramSpec& s, const ReassignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, SentinelKind::ExprStmt, ConstructRegistry::expr_stmts(), [](ProgramSpec& s, const ExprStmtVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            add_if_not_empty(pool, SentinelKind::Function, ConstructRegistry::functions(), [](ProgramSpec& s, const FuncVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Struct, ConstructRegistry::structs(), [](ProgramSpec& s, const StructVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Enum, ConstructRegistry::enums(), [](ProgramSpec& s, const EnumVerifier& v)
            {
                SpecAdder::add(s, v);
            });

            add_if_not_empty(pool, SentinelKind::Assignment, ConstructRegistry::modified_assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            return pool;
        }

        static std::vector<RecoveryBlock> build_top_level_pool()
        {
            std::vector<RecoveryBlock> pool;
            add_if_not_empty(pool, SentinelKind::Assignment, ConstructRegistry::assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, SentinelKind::Reassignment, ConstructRegistry::reassignments(), [](ProgramSpec& s, const ReassignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, SentinelKind::ExprStmt, ConstructRegistry::expr_stmts(), [](ProgramSpec& s, const ExprStmtVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            add_if_not_empty(pool, SentinelKind::Import, ConstructRegistry::imports(), [](ProgramSpec& s, const ImportVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Function, ConstructRegistry::functions(), [](ProgramSpec& s, const FuncVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Enum, ConstructRegistry::enums(), [](ProgramSpec& s, const EnumVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Alias, ConstructRegistry::aliases(), [](ProgramSpec& s, const AliasVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Directive, ConstructRegistry::directives(), [](ProgramSpec& s, const DirectiveVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Struct, ConstructRegistry::structs(), [](ProgramSpec& s, const StructVerifier& v)
            {
                SpecAdder::add(s, v);
            });

            add_if_not_empty(pool, SentinelKind::Import, ConstructRegistry::modified_imports(), [](ProgramSpec& s, const ImportVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Function, ConstructRegistry::modified_functions(), [](ProgramSpec& s, const FuncVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Struct, ConstructRegistry::modified_structs(), [](ProgramSpec& s, const StructVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Enum, ConstructRegistry::modified_enums(), [](ProgramSpec& s, const EnumVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Alias, ConstructRegistry::modified_aliases(), [](ProgramSpec& s, const AliasVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, SentinelKind::Assignment, ConstructRegistry::modified_assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            return pool;
        }

        static std::vector<RecoveryBlock> filter_pool(const std::vector<RecoveryBlock>& base_pool,
                                                     const std::vector<SentinelKind>& excluded_kinds,
                                                     const std::vector<SentinelKind>& accepted_kinds = {})
        {
            bool has_accepted_match = false;
            if (!accepted_kinds.empty())
            {
                for (const auto& block : base_pool)
                {
                    if (block.kind.has_value() &&
                        std::find(accepted_kinds.begin(), accepted_kinds.end(), *block.kind) != accepted_kinds.end())
                    {
                        has_accepted_match = true;
                        break;
                    }
                }
            }

            std::vector<RecoveryBlock> filtered;
            for (const auto& block : base_pool)
            {
                if (has_accepted_match)
                {
                    if (block.kind.has_value() &&
                        std::find(accepted_kinds.begin(), accepted_kinds.end(), *block.kind) != accepted_kinds.end())
                    {
                        filtered.push_back(block);
                    }
                    continue;
                }
                if (!block.kind.has_value() ||
                    std::find(excluded_kinds.begin(), excluded_kinds.end(), *block.kind) == excluded_kinds.end())
                {
                    filtered.push_back(block);
                }
            }
            return filtered;
        }

    public:
        static const std::vector<RecoveryBlock>& get_block_pool(BlockContext ctx_type)
        {
            switch (ctx_type)
            {
            case BlockContext::ExtensionBody:
                {
                    static std::vector<RecoveryBlock> extension_pool = build_extension_block_pool();
                    return extension_pool;
                }
            case BlockContext::TopLevel:
                {
                    static std::vector<RecoveryBlock> top_level_pool = build_top_level_pool();
                    return top_level_pool;
                }
            case BlockContext::FunctionBody:
            default:
                {
                    static std::vector<RecoveryBlock> function_pool = build_function_block_pool();
                    return function_pool;
                }
            }
        }

        static bool is_sentinel_supported_in_block(BlockContext ctx_type, SentinelKind kind)
        {
            const auto& base_pool = get_block_pool(ctx_type);
            for (const auto& block : base_pool)
            {
                if (block.kind.has_value() && *block.kind == kind)
                {
                    return true;
                }
            }
            return false;
        }

        static RecoveryBlock generate_block_sentinel(size_t seed, BlockContext ctx_type,
                                                     const std::vector<SentinelKind>& excluded_kinds = {},
                                                     const std::vector<SentinelKind>& accepted_kinds = {})
        {
            const auto& base_pool = get_block_pool(ctx_type);
            std::vector<RecoveryBlock> pool = filter_pool(base_pool, excluded_kinds, accepted_kinds);

            if (pool.empty())
                return {
                    std::nullopt, "", [](ProgramSpec&)
                    {
                    }
                };

            return DeterministicSampler::sample_element_rng(pool, seed);
        }

    };
}
