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

#include "src/tint/lang/wgsl/writer/raise/workgroup_uniform_load.h"

#include <initializer_list>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"
#include "src/tint/utils/containers/hashset.h"

namespace tint::wgsl::writer::raise {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// A workgroupBarrier and the instructions that follow it, up to the first instruction that
    /// cannot be moved across a barrier.
    struct BarrierSpan {
        /// The barrier that starts the span.
        core::ir::Instruction* start = nullptr;
        /// The loads in the span that can be replaced with workgroupUniformLoad.
        Vector<core::ir::Instruction*, 4> loads;
        /// The barrier that ends the span, or nullptr if the span ends with another instruction.
        core::ir::Instruction* end = nullptr;
    };

    /// Process the module.
    void Process() {
        // Decide which spans to replace before changing any of them, as the barrier that ends one
        // span can start the next.
        Vector<BarrierSpan, 8> spans;
        for (auto* inst : ir.Instructions()) {
            if (IsWorkgroupBarrier(inst)) {
                if (auto span = SpanFrom(inst); ShouldReplace(span)) {
                    spans.Push(std::move(span));
                }
            }
        }

        for (auto& span : spans) {
            for (auto* load : span.loads) {
                ReplaceLoad(load);
            }
            // The start of this span may have already been destroyed as the end of the last span.
            for (auto* barrier : {span.start, span.end}) {
                if (barrier && barrier->Alive()) {
                    barrier->Destroy();
                }
            }
        }
    }

  private:
    /// @returns the span of instructions that starts with @p barrier
    BarrierSpan SpanFrom(core::ir::Instruction* barrier) {
        BarrierSpan span;
        span.start = barrier;
        for (auto* inst = barrier->next; inst; inst = inst->next) {
            if (IsWorkgroupBarrier(inst)) {
                span.end = inst;
                break;
            }
            if (IsUniformWorkgroupLoad(inst)) {
                span.loads.Push(inst);
                continue;
            }
            if (!CanMoveAcrossBarrier(inst)) {
                break;
            }
        }
        return span;
    }

    /// @returns true if the loads of @p span should be replaced with workgroupUniformLoad
    bool ShouldReplace(const BarrierSpan& span) {
        if (span.loads.IsEmpty()) {
            return false;
        }
        // workgroupUniformLoad performs the barriers on both sides of the load, so replacing a
        // single load between two barriers costs nothing. This is also how the WGSL reader lowers
        // workgroupUniformLoad.
        if (span.end && span.loads.Length() == 1) {
            return true;
        }
        // Otherwise the replacement adds barriers, which is only worth it if the uniformity
        // analysis needs the loaded value to be uniform. This is approximated by checking whether
        // the value decides control flow. That can add a barrier that isn't needed, for example
        // when nothing in the branch requires uniformity. Being precise would mean searching the
        // branches for a barrier or other builtin that requires uniform control flow, which is
        // not done for now.
        for (auto* load : span.loads) {
            if (DecidesControlFlow(load->Result())) {
                return true;
            }
        }
        return false;
    }

    /// Replaces @p load with a call to workgroupUniformLoad.
    void ReplaceLoad(core::ir::Instruction* load) {
        auto* call = b.CallWithResult<wgsl::ir::BuiltinCall>(
            load->DetachResult(), wgsl::BuiltinFn::kWorkgroupUniformLoad, load->Operand(0));
        load->ReplaceWith(call);
        load->Destroy();
    }

    /// @returns true if @p inst is a call to the builtin @p fn
    static bool IsBuiltinCall(const core::ir::Instruction* inst, core::BuiltinFn fn) {
        auto* call = inst->As<core::ir::CoreBuiltinCall>();
        return call && call->Func() == fn;
    }

    /// @returns true if @p inst is a call to workgroupBarrier
    static bool IsWorkgroupBarrier(const core::ir::Instruction* inst) {
        return IsBuiltinCall(inst, core::BuiltinFn::kWorkgroupBarrier);
    }

    /// @returns true if @p inst is a load or atomicLoad through a pointer to workgroup memory
    /// that is the same for every invocation.
    static bool IsUniformWorkgroupLoad(const core::ir::Instruction* inst) {
        if (!inst->Is<core::ir::Load>() && !IsBuiltinCall(inst, core::BuiltinFn::kAtomicLoad)) {
            return false;
        }
        // workgroupUniformLoad requires its argument to be uniform, so only accept constant
        // indices into the variable.
        auto* ptr = inst->Operand(0);
        return AddressSpaceOf(ptr) == core::AddressSpace::kWorkgroup &&
               RootVar(ptr, /* constant_indices_only */ true);
    }

    /// @returns true if moving @p inst to the other side of a workgroupBarrier cannot change what
    /// any invocation observes. workgroupBarrier only orders memory accesses in the workgroup
    /// address space, so the instruction must not branch, must not access workgroup memory, and
    /// must not be another barrier.
    static bool CanMoveAcrossBarrier(const core::ir::Instruction* inst) {
        if (inst->IsAnyOf<core::ir::ControlInstruction, core::ir::Terminator>()) {
            return false;
        }
        if (inst->IsAnyOf<core::ir::Load, core::ir::LoadVectorElement, core::ir::Store,
                          core::ir::StoreVectorElement>()) {
            return !HasWorkgroupPointerOperand(inst);
        }
        if (auto* call = inst->As<core::ir::CoreBuiltinCall>();
            call && (core::IsAtomic(call->Func()) || core::IsTexture(call->Func()))) {
            return !HasWorkgroupPointerOperand(inst);
        }
        // Everything else, including the other barriers and calls to user functions, is only
        // moved if it has no side effects.
        return inst->GetSideEffects().Empty();
    }

    /// @returns true if any operand of @p inst is a pointer to workgroup memory
    static bool HasWorkgroupPointerOperand(const core::ir::Instruction* inst) {
        for (auto* operand : inst->Operands()) {
            if (auto* ptr = operand ? operand->Type()->As<core::type::Pointer>() : nullptr;
                ptr && ptr->AddressSpace() == core::AddressSpace::kWorkgroup) {
                return true;
            }
        }
        return false;
    }

    /// @returns the address space of the pointer @p ptr
    static core::AddressSpace AddressSpaceOf(const core::ir::Value* ptr) {
        return ptr->Type()->As<core::type::Pointer>()->AddressSpace();
    }

    /// @returns true if @p ptr points to memory in the function or private address space
    static bool IsInvocationLocal(const core::ir::Value* ptr) {
        auto space = AddressSpaceOf(ptr);
        return space == core::AddressSpace::kFunction || space == core::AddressSpace::kPrivate;
    }

    /// @returns true if @p value, or a value computed from it, is the condition of an `if`,
    /// `switch` or `break_if`. Values stored to function and private variables are followed to
    /// the loads of those variables. Values that leave the function, or are passed between blocks
    /// by a terminator, are assumed to decide control flow.
    static bool DecidesControlFlow(const core::ir::Value* value) {
        Hashset<const core::ir::Value*, 32> seen;
        Vector<const core::ir::Value*, 32> pending{value};
        while (!pending.IsEmpty()) {
            auto* v = pending.Pop();
            if (!seen.Add(v)) {
                continue;
            }
            for (auto& usage : v->UsagesUnsorted()) {
                auto* inst = usage->instruction;
                if (inst->IsAnyOf<core::ir::If, core::ir::Switch, core::ir::Terminator,
                                  core::ir::UserCall>()) {
                    return true;
                }
                if (inst->IsAnyOf<core::ir::Store, core::ir::StoreVectorElement>()) {
                    // A store only passes the value on if `v` is the value being stored.
                    if (usage->operand_index != 0 && IsInvocationLocal(inst->Operand(0))) {
                        auto* var = RootVar(inst->Operand(0));
                        if (!var) {
                            // Stored through a pointer parameter, so the value leaves the
                            // function.
                            return true;
                        }
                        pending.Push(var->Result());
                    }
                    continue;
                }
                for (auto* result : inst->Results()) {
                    pending.Push(result);
                }
            }
        }
        return false;
    }

    /// @returns the variable that @p ptr points into, or nullptr if it cannot be found, or if
    /// @p constant_indices_only is true and the pointer is made with a non-constant index.
    static const core::ir::Var* RootVar(const core::ir::Value* ptr,
                                        bool constant_indices_only = false) {
        while (auto* inst = ptr->AsInstruction()) {
            if (auto* var = inst->As<core::ir::Var>()) {
                return var;
            }
            if (auto* let = inst->As<core::ir::Let>()) {
                ptr = let->Value();
            } else if (auto* access = inst->As<core::ir::Access>()) {
                if (constant_indices_only) {
                    for (auto* index : access->Indices()) {
                        if (!index->Is<core::ir::Constant>()) {
                            return nullptr;
                        }
                    }
                }
                ptr = access->Object();
            } else {
                return nullptr;
            }
        }
        return nullptr;
    }
};

}  // namespace

Result<SuccessType> WorkgroupUniformLoad(core::ir::Module& ir) {
    core::ir::AssertValid(ir, "before wgsl.WorkgroupUniformLoad");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::wgsl::writer::raise
