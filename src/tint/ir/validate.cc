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

#include "src/tint/ir/function.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/switch.h"
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

    void CheckFunction(const Function* func) {
        for (const auto* param : func->Params()) {
            if (param == nullptr) {
                AddError("function '" + Name(func) + "': null parameter");
                continue;
            }
        }

        if (func->StartTarget() == nullptr) {
            AddError("function '" + Name(func) + "': null start target");
        } else {
            CheckBlock(func->StartTarget());
        }
    }

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
            inst,  //
            [&](const ir::Return* ret) {
                if (ret->Func() == nullptr) {
                    AddError("return: null function");
                }
            },
            [&](Default) {
                AddError(std::string("missing validation of: ") + inst->TypeInfo().name);
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
