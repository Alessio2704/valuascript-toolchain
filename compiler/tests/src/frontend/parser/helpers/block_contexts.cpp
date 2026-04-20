#include "context_registry.h"
#include "spec_adder.h"
#include <cassert>

namespace valuascript::compiler::test
{
    std::vector<Context> get_block_contexts()
    {
        return {
            {
                "function_body_wrapper", NestingLevel::BlockLevel,
                {InjectableType::Statement, InjectableType::Return},
                "func ctx_wrapper() -> void {\n  ", "\n}\n",
                [](ProgramSpec& s, UniversalVerifier v)
                {
                    std::visit(overloaded{
                                   [&](StmtVerifier& sv)
                                   {
                                       SpecAdder::add(s, IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {sv}));
                                   },
                                   [&](ReturnVerifier& rv)
                                   {
                                       SpecAdder::add(s, IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {rv}));
                                   },
                                   [](auto&)
                                   {
                                       assert(
                                           false &&
                                           "Block context received an InjectableType verifier it does not support. "
                                           "Check allowed_atoms in block_contexts.cpp.");
                                   }
                               }, v);
                }
            }
        };
    }
}
