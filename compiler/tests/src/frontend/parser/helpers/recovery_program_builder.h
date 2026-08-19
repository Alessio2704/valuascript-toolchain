#pragma once

#include <string>
#include <vector>
#include <optional>
#include <functional>
#include "frontend/parser/node_matchers/node_matchers.h"
#include "sentinel_kind.h"
#include "test_structures.h"

namespace valuascript::compiler::test
{
    struct ConstructedRecoveryProgram
    {
        std::string full_code;
        ProgramSpec full_spec;
        std::string prefix_for_shifting;
        std::string path_name = "";
        std::optional<SentinelKind> post_kind = std::nullopt;
        bool is_post_modified = false;
        std::optional<SentinelKind> pre_kind = std::nullopt;
        bool is_pre_modified = false;
        std::optional<SentinelKind> inner_pre_kind = std::nullopt;
        bool is_inner_pre_modified = false;
        std::optional<SentinelKind> inner_post_kind = std::nullopt;
        bool is_inner_post_modified = false;
    };

    class RecoveryProgramBuilder
    {
    public:
        static std::vector<ConstructedRecoveryProgram> BuildRecoveryPrograms(
            std::string inner_code,
            ProgramSpec inner_spec,
            const std::string& inner_prefix,
            size_t seed,
            const std::vector<SentinelKind>& excluded_sentinels = {},
            const std::vector<SentinelKind>& accepted_sentinels = {},
            const std::string& path_name = "");

        static std::vector<ConstructedRecoveryProgram> BuildRecoveryPrograms(
            const ProcessingItem& item,
            size_t seed,
            std::optional<ProgramSpec> inner_spec_override = std::nullopt);

        template <typename Callback>
        static void ForEachRecoveryProgram(const ProcessingItem& item, size_t seed, Callback&& callback)
        {
            auto progs = BuildRecoveryPrograms(item, seed);
            for (const auto& prog : progs)
            {
                SCOPED_TRACE("Recovery Path: " + (prog.path_name.empty() ? item.path_name : prog.path_name));
                callback(prog);
            }
        }

        template <typename Callback>
        static void ForEachRecoveryProgram(const ProcessingItem& item, ProgramSpec inner_spec, size_t seed, Callback&& callback)
        {
            auto progs = BuildRecoveryPrograms(item, seed, std::move(inner_spec));
            for (const auto& prog : progs)
            {
                SCOPED_TRACE("Recovery Path: " + (prog.path_name.empty() ? item.path_name : prog.path_name));
                callback(prog);
            }
        }

        template <typename Callback>
        static void ForEachRecoveryProgram(std::string inner_code,
                                           ProgramSpec inner_spec,
                                           const std::string& inner_prefix,
                                           size_t seed,
                                           const std::vector<SentinelKind>& excluded_sentinels,
                                           const std::vector<SentinelKind>& accepted_sentinels,
                                           const std::string& path_name,
                                           Callback&& callback)
        {
            auto progs = BuildRecoveryPrograms(std::move(inner_code), std::move(inner_spec), inner_prefix, seed, excluded_sentinels, accepted_sentinels, path_name);
            for (const auto& prog : progs)
            {
                if (!prog.path_name.empty())
                {
                    SCOPED_TRACE("Recovery Path: " + prog.path_name);
                }
                callback(prog);
            }
        }

        template <typename Callback>
        static void ForEachRecoveryProgram(std::string inner_code,
                                           ProgramSpec inner_spec,
                                           const std::string& inner_prefix,
                                           size_t seed,
                                           Callback&& callback)
        {
            ForEachRecoveryProgram(std::move(inner_code), std::move(inner_spec), inner_prefix, seed, {}, {}, "", std::forward<Callback>(callback));
        }

        static void RunRecoveryScenario(ProcessingItem&& item,
                                        const std::vector<ParserExpectedError>& errors,
                                        size_t seed);
    };
}
