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

#include <variant>

#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/scalar.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/rename_conflicts.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/scope_stack.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"

TINT_INSTANTIATE_TYPEINFO(tint::wgsl::writer::RenameConflicts);

namespace tint::wgsl::writer {

/// PIMPL state for the transform, for a single function.
struct RenameConflicts::State {
    /// Constructor
    /// @param i the IR module
    explicit State(ir::Module* i) : ir(i) {}

    /// Processes the module, renaming all declarations that would prevent an identifier resolving
    /// to the correct declaration.
    void Process() {
        scopes.Push(Scope{});
        TINT_DEFER(scopes.Clear());

        RegisterModuleScopeDecls();

        // Process the module-scope variable declarations
        if (ir->root_block) {
            for (auto* inst : *ir->root_block) {
                Process(inst);
            }
        }

        // Process the functions
        for (auto* fn : ir->functions) {
            scopes.Push(Scope{});
            TINT_DEFER(scopes.Pop());
            for (auto* param : fn->Params()) {
                EnsureResolvable(param->Type());
                if (auto symbol = ir->NameOf(param); symbol.IsValid()) {
                    Declare(scopes.Back(), param, symbol.NameView());
                }
            }
            Process(fn->Block());
        }
    }

  private:
    /// Map of identifier to declaration.
    /// The declarations may be one of an ir::Value or type::Struct.
    using Scope = Hashmap<std::string_view, CastableBase*, 8>;

    /// The IR module.
    ir::Module* ir = nullptr;

    /// Stack of scopes
    Vector<Scope, 8> scopes{};

    /// Registers all the WGSL module-scope declarations in the root-scope.
    /// Duplicate declarations with the same name will renamed.
    void RegisterModuleScopeDecls() {
        // Declare all the user types
        for (auto* ty : ir->Types()) {
            if (auto* str = ty->As<type::Struct>()) {
                auto name = str->Name().NameView();
                if (!IsBuiltinStruct(str)) {
                    Declare(scopes.Front(), const_cast<type::Struct*>(str), name);
                }
            }
        }

        // Declare all the module-scope vars
        if (ir->root_block) {
            for (auto* inst : *ir->root_block) {
                for (auto* result : inst->Results()) {
                    if (auto symbol = ir->NameOf(result)) {
                        Declare(scopes.Front(), result, symbol.NameView());
                    }
                }
            }
        }

        // Declare all the functions
        for (auto* fn : ir->functions) {
            if (auto symbol = ir->NameOf(fn); symbol.IsValid()) {
                Declare(scopes.Back(), fn, symbol.NameView());
            }
        }
    }

    /// Processes the instructions of the block
    void Process(ir::Block* block) {
        for (auto* inst : *block) {
            Process(inst);
        }
    }

    /// Processes an instruction, ensuring that all identifier references resolve to the correct
    /// declaration. This may involve renaming of declarations in the outer scopes.
    void Process(ir::Instruction* inst) {
        // Check resolving of operands
        for (auto* operand : inst->Operands()) {
            if (operand) {
                // Ensure that named operands can be resolved.
                if (auto symbol = ir->NameOf(operand)) {
                    EnsureResolvesTo(symbol.NameView(), operand);
                }
                // If the operand is a constant, then ensure that type name can be resolved.
                if (auto* c = operand->As<ir::Constant>()) {
                    EnsureResolvable(c->Type());
                }
            }
        }

        Switch(
            inst,  //
            [&](ir::Loop* loop) {
                // Initializer's scope encompasses the body and continuing
                scopes.Push(Scope{});
                TINT_DEFER(scopes.Pop());
                Process(loop->Initializer());
                {
                    // Body's scope encompasses the continuing
                    scopes.Push(Scope{});
                    TINT_DEFER(scopes.Pop());
                    Process(loop->Body());
                    {
                        scopes.Push(Scope{});
                        TINT_DEFER(scopes.Pop());
                        Process(loop->Continuing());
                    }
                }
            },
            [&](ir::ControlInstruction* ctrl) {
                // Traverse into the control instruction's blocks
                ctrl->ForeachBlock([&](ir::Block* block) {
                    scopes.Push(Scope{});
                    TINT_DEFER(scopes.Pop());
                    Process(block);
                });
            },
            [&](ir::Var*) {
                // Ensure the var's type is resolvable
                EnsureResolvable(inst->Result()->Type());
            },
            [&](ir::Construct*) {
                // Ensure the type of a type constructor is resolvable
                EnsureResolvable(inst->Result()->Type());
            },
            [&](ir::CoreBuiltinCall* call) {
                // Ensure builtin of a builtin call is resolvable
                auto name = tint::ToString(call->Func());
                EnsureResolvesTo(name, nullptr);
            });

        // Register new operands and check their types can resolve
        for (auto* result : inst->Results()) {
            if (auto symbol = ir->NameOf(result); symbol.IsValid()) {
                Declare(scopes.Back(), result, symbol.NameView());
            }
        }
    }

    /// Ensures that the type @p type can be resolved given its identifier(s)
    void EnsureResolvable(const type::Type* type) {
        while (type) {
            type = tint::Switch(
                type,  //
                [&](const type::Scalar* s) {
                    EnsureResolvesTo(s->FriendlyName(), nullptr);
                    return nullptr;
                },
                [&](const type::Vector* v) {
                    EnsureResolvesTo("vec" + tint::ToString(v->Width()), nullptr);
                    return v->type();
                },
                [&](const type::Matrix* m) {
                    EnsureResolvesTo(
                        "mat" + tint::ToString(m->columns()) + "x" + tint::ToString(m->rows()),
                        nullptr);
                    return m->type();
                },
                [&](const type::Pointer* p) {
                    EnsureResolvesTo(tint::ToString(p->Access()), nullptr);
                    EnsureResolvesTo(tint::ToString(p->AddressSpace()), nullptr);
                    return p->StoreType();
                },
                [&](const type::Struct* s) {
                    auto name = s->Name().NameView();
                    if (IsBuiltinStruct(s)) {
                        EnsureResolvesTo(name, nullptr);
                    } else {
                        EnsureResolvesTo(name, s);
                    }
                    return nullptr;
                });
        }
    }

    /// Ensures that the identifier @p identifier resolves to the declaration @p thing
    void EnsureResolvesTo(std::string_view identifier, const CastableBase* thing) {
        for (auto& scope : tint::Reverse(scopes)) {
            if (auto decl = scope.Get(identifier)) {
                if (decl.value() == thing) {
                    return;  // Resolved to the right thing.
                }

                // Operand is shadowed
                scope.Remove(identifier);
                Rename(decl.value(), identifier);
            }
        }
    }

    /// Registers the declaration @p thing in the scope @p scope with the name @p name
    /// If there is an existing declaration with the given name in @p scope then @p thing will be
    /// renamed.
    void Declare(Scope& scope, CastableBase* thing, std::string_view name) {
        auto add = scope.Add(name, thing);
        if (!add && *add.value != thing) {
            // Multiple declarations with the same name in the same scope.
            // Rename the later declaration.
            Rename(thing, name);
        }
    }

    /// Rename changes the name of @p thing with the old name of @p old_name
    void Rename(CastableBase* thing, std::string_view old_name) {
        Symbol new_name = ir->symbols.New(old_name);
        Switch(
            thing,  //
            [&](ir::Value* value) { ir->SetName(value, new_name); },
            [&](type::Struct* str) { str->SetName(new_name); },
            [&](Default) {
                TINT_ICE() << "unhandled type for renaming: " << thing->TypeInfo().name;
            });
    }

    /// @return true if @p s is a builtin (non-user declared) structure.
    bool IsBuiltinStruct(const type::Struct* s) {
        // TODO(bclayton): Need to do better than this.
        return tint::HasPrefix(s->Name().NameView(), "_");
    }
};

RenameConflicts::RenameConflicts() = default;
RenameConflicts::~RenameConflicts() = default;

void RenameConflicts::Run(ir::Module* ir) const {
    State{ir}.Process();
}

}  // namespace tint::wgsl::writer
