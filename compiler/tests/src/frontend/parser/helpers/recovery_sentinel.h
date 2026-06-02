#pragma once

#include <string>
#include <vector>
#include <random>
#include <functional>
#include "construct_registry.h"
#include "spec_adder.h"

namespace valuascript::compiler::test
{
    struct RecoveryBlock
    {
        std::string source;
        std::function<void(ProgramSpec&)> add_to_spec;
    };

    class RecoverySentinel
    {
    private:
        template <typename T, typename AdderFunc>
        static void add_if_not_empty(std::vector<RecoveryBlock>& blocks, const std::vector<RegistryEntry<T>>& registry,
                                     AdderFunc adder)
        {
            for (const auto& entry : registry)
            {
                blocks.push_back({
                    entry.code,
                    [adder, verifier = entry.verifier](ProgramSpec& spec)
                    {
                        adder(spec, verifier);
                    }
                });
            }
        }

        static std::vector<RecoveryBlock> build_block_pool()
        {
            std::vector<RecoveryBlock> pool;
            add_if_not_empty(pool, ConstructRegistry::assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, ConstructRegistry::reassignments(), [](ProgramSpec& s, const ReassignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, ConstructRegistry::expr_stmts(), [](ProgramSpec& s, const ExprStmtVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, ConstructRegistry::returns(), [](ProgramSpec& s, const ReturnVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            add_if_not_empty(pool, ConstructRegistry::modified_assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, ConstructRegistry::modified_returns(), [](ProgramSpec& s, const ReturnVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            return pool;
        }

        static std::vector<RecoveryBlock> build_top_level_pool()
        {
            std::vector<RecoveryBlock> pool;
            add_if_not_empty(pool, ConstructRegistry::assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, ConstructRegistry::reassignments(), [](ProgramSpec& s, const ReassignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });
            add_if_not_empty(pool, ConstructRegistry::expr_stmts(), [](ProgramSpec& s, const ExprStmtVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            add_if_not_empty(pool, ConstructRegistry::imports(), [](ProgramSpec& s, const ImportVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::functions(), [](ProgramSpec& s, const FuncVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::enums(), [](ProgramSpec& s, const EnumVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::aliases(), [](ProgramSpec& s, const AliasVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::directives(), [](ProgramSpec& s, const DirectiveVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::structs(), [](ProgramSpec& s, const StructVerifier& v)
            {
                SpecAdder::add(s, v);
            });

            add_if_not_empty(pool, ConstructRegistry::modified_imports(), [](ProgramSpec& s, const ImportVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::modified_functions(), [](ProgramSpec& s, const FuncVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::modified_structs(), [](ProgramSpec& s, const StructVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::modified_enums(), [](ProgramSpec& s, const EnumVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::modified_aliases(), [](ProgramSpec& s, const AliasVerifier& v)
            {
                SpecAdder::add(s, v);
            });
            add_if_not_empty(pool, ConstructRegistry::modified_assignments(), [](ProgramSpec& s, const AssignmentVerifier& v)
            {
                SpecAdder::add(s, StmtVerifier(v));
            });

            return pool;
        }

    public:
        static RecoveryBlock generate_block_sentinel(size_t seed)
        {
            static std::vector<RecoveryBlock> pool = build_block_pool();

            if (pool.empty())
                return {
                    "", [](ProgramSpec&)
                    {
                    }
                };

            std::mt19937 rng(static_cast<unsigned int>(seed));
            std::uniform_int_distribution<uint64_t> dist(0, static_cast<uint64_t>(pool.size()) - 1);

            return pool[static_cast<size_t>(dist(rng))];
        }

        static RecoveryBlock generate_top_level_sentinel(size_t seed)
        {
            static std::vector<RecoveryBlock> pool = build_top_level_pool();

            if (pool.empty())
                return {
                    "", [](ProgramSpec&)
                    {
                    }
                };

            std::mt19937 rng(static_cast<unsigned int>(seed));
            std::uniform_int_distribution<uint64_t> dist(0, static_cast<uint64_t>(pool.size()) - 1);

            return pool[static_cast<size_t>(dist(rng))];
        }

        static const std::vector<RecoveryBlock>& get_all_block_sentinels()
        {
            static std::vector<RecoveryBlock> pool = build_block_pool();
            return pool;
        }

        static const std::vector<RecoveryBlock>& get_all_top_level_sentinels()
        {
            static std::vector<RecoveryBlock> pool = build_top_level_pool();
            return pool;
        }
    };
}
