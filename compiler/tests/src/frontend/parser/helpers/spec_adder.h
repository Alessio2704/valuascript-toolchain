#pragma once
#include "context_infrastructure.h"
#include <type_traits>

namespace valuascript::compiler::test
{
    class SpecAdder
    {
    public:
        SpecAdder() = delete;

        static void add(ProgramSpec&, const NullVerifier&)
        {
        }

        static void add(ProgramSpec&, const std::string&)
        {
        }

        static void add(ProgramSpec& s, const ImportVerifier& v) { s.imports.emplace_back(v); }
        static void add(ProgramSpec& s, const DirectiveVerifier& v) { s.directives.emplace_back(v); }
        static void add(ProgramSpec& s, const FuncVerifier& v) { s.functions.emplace_back(v); }
        static void add(ProgramSpec& s, const StructVerifier& v) { s.structs.emplace_back(v); }
        static void add(ProgramSpec& s, const EnumVerifier& v) { s.enums.emplace_back(v); }
        static void add(ProgramSpec& s, const AliasVerifier& v) { s.type_aliases.emplace_back(v); }
        static void add(ProgramSpec& s, const StmtVerifier& v) { s.execution_steps.emplace_back(v); }
        static void add(ProgramSpec& s, const ExprStmtVerifier& v) { s.execution_steps.emplace_back(v); }

        static void add(ProgramSpec&, const ExprVerifier&)
        {
        }

        static void add(ProgramSpec&, const ModifierVerifier&)
        {
        }

        static void add(ProgramSpec&, const TypeVerifier&)
        {
        }

        static void add(ProgramSpec&, const ReturnVerifier&)
        {
        }

        template <typename T>
        static T get_v(const UniversalVerifier& v)
        {
            if (std::holds_alternative<NullVerifier>(v))
            {
                if constexpr (std::is_constructible_v<T, NullVerifier>)
                {
                    return T(NullVerifier{});
                }
                else
                {
                    return T{};
                }
            }
            return std::get<T>(v);
        }

        static std::string get_id(const UniversalVerifier& v)
        {
            if (std::holds_alternative<std::string>(v))
            {
                return std::get<std::string>(v);
            }
            return "unknown";
        }
    };
}
