#pragma once

namespace valuascript::compiler::test::SpecAdder
{
    inline void add(ProgramSpec& s, const ImportVerifier& v) { s.imports.emplace_back(v); }
    inline void add(ProgramSpec& s, const DirectiveVerifier& v) { s.directives.emplace_back(v); }
    inline void add(ProgramSpec& s, const FuncVerifier& v) { s.functions.emplace_back(v); }
    inline void add(ProgramSpec& s, const StructVerifier& v) { s.structs.emplace_back(v); }
    inline void add(ProgramSpec& s, const EnumVerifier& v) { s.enums.emplace_back(v); }
    inline void add(ProgramSpec& s, const AliasVerifier& v) { s.type_aliases.emplace_back(v); }
    inline void add(ProgramSpec& s, const StmtVerifier& v) { s.execution_steps.emplace_back(v); }
    inline void add(ProgramSpec& s, const ExprStmtVerifier& v) { s.execution_steps.emplace_back(v); }

    inline void add(ProgramSpec&, const ExprVerifier&)
    {
    }

    inline void add(ProgramSpec&, const ModifierVerifier&)
    {
    }

    inline void add(ProgramSpec&, const TypeVerifier&)
    {
    }

    inline void add(ProgramSpec&, const ReturnVerifier&)
    {
    }

    template <typename T>
    T get_v(UniversalVerifier v)
    {
        return std::get<T>(v);
    }
}
