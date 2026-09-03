// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_CORE_IR_VALIDATOR_FUNCTIONAL_VALIDATOR_H_
#define SRC_TINT_LANG_CORE_IR_VALIDATOR_FUNCTIONAL_VALIDATOR_H_

#include <string>

#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/validator/validator.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::core::ir::validator {

class Functional {
  public:
    Functional(Module& ir, diag::List& diagnostics, ErrorSource error_source);
    ~Functional();

    void Validate();

    Functional(const Functional&) = delete;
    Functional(Functional&&) = delete;
    Functional& operator=(const Functional&) = delete;
    Functional& operator=(Functional&&) = delete;

  private:
    using SourceHelper = std::function<Source()>;

    /// ScopeStack holds a stack of values that are currently in scope
    struct ScopeStack {
        void Push() { stack_.Push({}); }
        void Pop() { stack_.Pop(); }
        void Add(const Value* value) { stack_.Back().Add(value); }
        bool Contains(const Value* value) {
            return stack_.Any([&](auto& v) { return v.Contains(value); });
        }
        bool IsEmpty() const { return stack_.IsEmpty(); }

      private:
        Vector<Hashset<const Value*, 8>, 4> stack_;
    };

    // Returns true if we're validating in the context of WGSL. The other option is we're validating
    // as IR. The primary difference is how const-eval checks are run as the semantics are
    // different.
    bool IsWGSLValidation() const;

    StyledText NameOf(const core::type::Type* ty);
    StyledText NameOf(const Value* value);

    Source SourceOf(const Function* func);
    Source SourceOf(const FunctionParam* param);
    Source SourceOf(const Instruction* inst);
    Source SourceOf(const Instruction* inst, size_t idx);

    diag::Diagnostic& AddError(Source src);
    diag::Diagnostic& AddError(const Function* func);
    diag::Diagnostic& AddError(const FunctionParam* param);
    diag::Diagnostic& AddError(const Instruction* inst);
    diag::Diagnostic& AddError(const Instruction* inst, size_t idx);

    diag::Diagnostic& AddNote(Source src);
    diag::Diagnostic& AddNote(const Block* blk);
    diag::Diagnostic& AddNote(const Function* func);
    diag::Diagnostic& AddNote(const Instruction* inst);
    diag::Diagnostic& AddNote(const Instruction* inst, size_t idx);

    ir::Disassembler& Disassemble();

    void CheckFunction(const Function* func);
    void CheckBlock(const Block* blk);
    void CheckInstruction(const Instruction* inst);

    void CheckContinue(const Continue* c);
    void CheckIf(const If* if_);
    void CheckLoop(const Loop* l);
    void CheckLoopBody(const Loop* loop);
    void CheckLoopContinuing(const Loop* loop);
    void CheckSwitch(const Switch* s);
    void CheckTerminator(const Terminator* b);

    Module& ir_;
    diag::List& diag_;
    ErrorSource error_source_;
    std::optional<ir::Disassembler> disassembler_;  // Use Disassemble()

    SymbolTable symbols_ = SymbolTable::Wrap(ir_.symbols);
    core::type::Manager type_mgr_ = core::type::Manager::Wrap(ir_.Types());
    core::ir::ReferencedModuleVars<const Module> referenced_module_vars_;

    Vector<const Block*, 8> block_stack_;
    Hashmap<const Loop*, const Continue*, 4> first_continues_;
};

}  // namespace tint::core::ir::validator

#endif  // SRC_TINT_LANG_CORE_IR_VALIDATOR_FUNCTIONAL_VALIDATOR_H_
