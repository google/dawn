// Copyright 2023 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/ir/validate.h"

#include <utility>

#include "src/tint/ir/access.h"
#include "src/tint/ir/binary.h"
#include "src/tint/ir/bitcast.h"
#include "src/tint/ir/break_if.h"
#include "src/tint/ir/builtin.h"
#include "src/tint/ir/construct.h"
#include "src/tint/ir/continue.h"
#include "src/tint/ir/convert.h"
#include "src/tint/ir/discard.h"
#include "src/tint/ir/exit_if.h"
#include "src/tint/ir/exit_loop.h"
#include "src/tint/ir/exit_switch.h"
#include "src/tint/ir/function.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/next_iteration.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/swizzle.h"
#include "src/tint/ir/unary.h"
#include "src/tint/ir/user_call.h"
#include "src/tint/ir/var.h"
#include "src/tint/switch.h"
#include "src/tint/type/pointer.h"

namespace tint::ir {
namespace {

class Validator {
  public:
    explicit Validator(const Module& mod) : mod_(mod) {}

    ~Validator() {}

    utils::Result<Success, diag::List> IsValid() {
        CheckRootBlock(mod_.root_block);

        for (const auto* func : mod_.functions) {
            CheckFunction(func);
        }

        if (diagnostics_.contains_errors()) {
            return std::move(diagnostics_);
        }
        return Success{};
    }

  private:
    const Module& mod_;
    diag::List diagnostics_;

    void AddError(const std::string& err) { diagnostics_.add_error(tint::diag::System::IR, err); }

    std::string Name(const Value* v) { return mod_.NameOf(v).Name(); }

    void CheckRootBlock(const Block* blk) {
        if (!blk) {
            return;
        }

        for (const auto* inst : *blk) {
            auto* var = inst->As<ir::Var>();
            if (!var) {
                AddError(std::string("root block: invalid instruction: ") + inst->TypeInfo().name);
                continue;
            }
            if (!var->Type()->Is<type::Pointer>()) {
                AddError(std::string("root block: 'var' ") + Name(var) +
                         "type is not a pointer: " + var->Type()->TypeInfo().name);
            }
        }
    }

    void CheckFunction(const Function* func) { CheckBlock(func->StartTarget()); }

    void CheckBlock(const Block* blk) {
        if (!blk->HasBranchTarget()) {
            AddError("block: does not end in a branch");
        }

        for (const auto* inst : *blk) {
            if (inst->Is<ir::Branch>() && inst != blk->Branch()) {
                AddError("block: branch which isn't the final instruction");
                continue;
            }

            CheckInstruction(inst);
        }
    }

    void CheckInstruction(const Instruction* inst) {
        tint::Switch(
            inst,                                          //
            [&](const ir::Access*) {},                     //
            [&](const ir::Binary*) {},                     //
            [&](const ir::Branch* b) { CheckBranch(b); },  //
            [&](const ir::Call* c) { CheckCall(c); },      //
            [&](const ir::Load*) {},                       //
            [&](const ir::Store*) {},                      //
            [&](const ir::Swizzle*) {},                    //
            [&](const ir::Unary*) {},                      //
            [&](const ir::Var*) {},                        //
            [&](Default) {
                AddError(std::string("missing validation of: ") + inst->TypeInfo().name);
            });
    }

    void CheckCall(const ir::Call* call) {
        tint::Switch(
            call,                          //
            [&](const ir::Bitcast*) {},    //
            [&](const ir::Builtin*) {},    //
            [&](const ir::Construct*) {},  //
            [&](const ir::Convert*) {},    //
            [&](const ir::Discard*) {},    //
            [&](const ir::UserCall*) {},   //
            [&](Default) {
                AddError(std::string("missing validation of call: ") + call->TypeInfo().name);
            });
    }

    void CheckBranch(const ir::Branch* b) {
        tint::Switch(
            b,                                 //
            [&](const ir::BreakIf*) {},        //
            [&](const ir::Continue*) {},       //
            [&](const ir::ExitIf*) {},         //
            [&](const ir::ExitLoop*) {},       //
            [&](const ir::ExitSwitch*) {},     //
            [&](const ir::If*) {},             //
            [&](const ir::Loop*) {},           //
            [&](const ir::NextIteration*) {},  //
            [&](const ir::Return* ret) {
                if (ret->Func() == nullptr) {
                    AddError("return: null function");
                }
            },                          //
            [&](const ir::Switch*) {},  //
            [&](Default) {
                AddError(std::string("missing validation of branch: ") + b->TypeInfo().name);
            });
    }
};

}  // namespace

utils::Result<Success, std::string> Validate(const Module& mod) {
    Validator v(mod);
    auto r = v.IsValid();
    if (!r) {
        return r.Failure().str();
    }
    return Success{};
}

}  // namespace tint::ir
