
// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/rename_conflicts.h"

#include <algorithm>
#include <array>
#include <string_view>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/scalar.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir::transform {

namespace {

/// @returns the identifier of the vector type with @p width elements
std::string_view VectorName(uint32_t width) {
    static constexpr std::array<std::string_view, 3> kNames{"vec2", "vec3", "vec4"};
    TINT_ASSERT(width >= 2 && width <= 4);
    return kNames[width - 2];
}

/// @returns the identifier of the matrix type with @p columns columns and @p rows rows
std::string_view MatrixName(uint32_t columns, uint32_t rows) {
    static constexpr std::array<std::array<std::string_view, 3>, 3> kNames{{
        {"mat2x2", "mat2x3", "mat2x4"},
        {"mat3x2", "mat3x3", "mat3x4"},
        {"mat4x2", "mat4x3", "mat4x4"},
    }};
    TINT_ASSERT(columns >= 2 && columns <= 4 && rows >= 2 && rows <= 4);
    return kNames[columns - 2][rows - 2];
}

/// PIMPL state for the transform, for a single function.
struct State {
    /// Constructor
    /// @param i the IR module
    explicit State(core::ir::Module& i) : ir(i) {}

    /// Processes the module, renaming all declarations that would prevent an identifier resolving
    /// to the correct declaration.
    void Process() {
        PushScope();
        TINT_DEFER(PopScope());

        RegisterModuleScopeDecls();

        // Process the types
        for (auto* ty : ir.Types()) {
            EnsureResolvable(ty);
        }

        // Process the module-scope variable declarations
        for (auto* inst : *ir.root_block) {
            Process(inst);
        }

        // Process the functions
        for (core::ir::Function* fn : ir.functions) {
            PushScope();
            TINT_DEFER(PopScope());
            for (auto* param : fn->Params()) {
                EnsureResolvable(param->Type());
                if (auto symbol = ir.NameOf(param); symbol.IsValid()) {
                    Declare(param, symbol);
                }
            }
            Process(fn->Block());
        }
    }

  private:
    /// The IR module.
    core::ir::Module& ir;

    /// A declaration and the scope that it was added to.
    struct Declaration {
        /// The declaration: a core::ir::Value or a core::type::Struct.
        CastableBase* thing = nullptr;
        /// The identifier of the scope that the declaration was added to.
        uint32_t scope = 0;
    };

    /// Map of identifier to the last declaration with that name.
    /// The declaration is only in scope while its scope is in #active_scopes. Declare() renames a
    /// declaration that would shadow another, so there is at most one declaration in scope for
    /// each name.
    Hashmap<std::string_view, Declaration, 64> declarations{};

    /// The identifiers of the active scopes, in ascending order.
    Vector<uint32_t, 8> active_scopes{};

    /// The identifier of the next scope.
    uint32_t next_scope = 0;

    /// Begins a new scope.
    void PushScope() { active_scopes.Push(next_scope++); }

    /// Ends the innermost scope.
    void PopScope() { active_scopes.Pop(); }

    /// @returns the declaration in scope with the name @p name, or nullptr if there is none
    Declaration* Lookup(std::string_view name) {
        auto decl = declarations.Get(name);
        if (decl && std::ranges::binary_search(active_scopes, decl->scope)) {
            return &*decl;
        }
        return nullptr;
    }

    /// Registers all the module-scope declarations in the root-scope.
    /// Duplicate declarations with the same name will renamed.
    void RegisterModuleScopeDecls() {
        // Declare all the user types
        for (auto* ty : ir.Types()) {
            if (auto* str = ty->As<core::type::Struct>()) {
                if (!IsBuiltinStruct(str)) {
                    Declare(const_cast<core::type::Struct*>(str), str->Name());
                }
            }
        }

        // Declare all the module-scope vars
        for (auto* inst : *ir.root_block) {
            for (auto* result : inst->Results()) {
                if (auto symbol = ir.NameOf(result)) {
                    Declare(result, symbol);
                }
            }
        }

        // Declare all the functions
        for (core::ir::Function* fn : ir.functions) {
            if (auto symbol = ir.NameOf(fn); symbol.IsValid()) {
                Declare(fn, symbol);
            }
        }
    }

    /// Processes the instructions of the block
    void Process(core::ir::Block* block) {
        for (auto* inst : *block) {
            Process(inst);
        }
    }

    /// Processes an instruction, ensuring that all identifier references resolve to the correct
    /// declaration. This may involve renaming of declarations in the outer scopes.
    void Process(core::ir::Instruction* inst) {
        // Check resolving of operands
        for (auto* operand : inst->Operands()) {
            if (operand) {
                // If the operand is a constant, then ensure that type name can be resolved.
                if (auto* c = operand->As<core::ir::Constant>()) {
                    EnsureResolvable(c->Type());
                }
            }
        }

        tint::Switch(
            inst,  //
            [&](core::ir::Loop* loop) {
                // Initializer's scope encompasses the body and continuing
                PushScope();
                TINT_DEFER(PopScope());
                Process(loop->Initializer());
                {
                    // Body's scope encompasses the continuing
                    PushScope();
                    TINT_DEFER(PopScope());
                    Process(loop->Body());
                    {
                        PushScope();
                        TINT_DEFER(PopScope());
                        Process(loop->Continuing());
                    }
                }
            },
            [&](core::ir::ControlInstruction* ctrl) {
                // Traverse into the control instruction's blocks
                ctrl->ForeachBlock([&](core::ir::Block* block) {
                    PushScope();
                    TINT_DEFER(PopScope());
                    Process(block);
                });
            },
            [&](core::ir::Var*) {
                // Ensure the var's type is resolvable
                EnsureResolvable(inst->Result()->Type());
            },
            [&](core::ir::Let*) {
                // Ensure the let's type is resolvable
                EnsureResolvable(inst->Result()->Type());
            },
            [&](core::ir::Construct*) {
                // Ensure the type of a type constructor is resolvable
                EnsureResolvable(inst->Result()->Type());
            },
            [&](core::ir::Convert*) {
                // Ensure the type of a type constructor is resolvable
                EnsureResolvable(inst->Result()->Type());
            },
            [&](core::ir::CoreBuiltinCall* call) {
                // Ensure builtin of a builtin call is resolvable
                EnsureResolvesToBuiltin(core::str(call->Func()));
            });

        // Register new operands and check their types can resolve
        for (auto* result : inst->Results()) {
            if (auto symbol = ir.NameOf(result); symbol.IsValid()) {
                Declare(result, symbol);
            }
        }
    }

    /// Ensures that the type @p type can be resolved given its identifier(s)
    void EnsureResolvable(const core::type::Type* type) {
        while (type) {
            type = tint::Switch(
                type,  //
                [&](const core::type::Scalar* s) {
                    EnsureResolvesToBuiltin(s->FriendlyName());
                    return nullptr;
                },
                [&](const core::type::Vector* v) {
                    EnsureResolvesToBuiltin(VectorName(v->Width()));
                    return v->Type();
                },
                [&](const core::type::Matrix* m) {
                    EnsureResolvesToBuiltin(MatrixName(m->Columns(), m->Rows()));
                    return m->Type();
                },
                [&](const core::type::Array* a) -> const core::type::Type* {
                    EnsureResolvesToBuiltin("array");
                    return a->ElemType();
                },
                [&](const core::type::Pointer* p) {
                    EnsureResolvesToBuiltin(core::ToString(p->Access()));
                    EnsureResolvesToBuiltin(core::ToString(p->AddressSpace()));
                    return p->StoreType();
                },
                [&](const core::type::Struct* s) {
                    auto name = s->Name().NameView();
                    if (IsBuiltinStruct(s)) {
                        EnsureResolvesToBuiltin(name);
                    } else {
                        Hashset<std::string_view, 8> member_names;
                        for (auto* mem : s->Members()) {
                            auto mem_name = mem->Name().NameView();
                            if (!member_names.Add(mem_name)) {
                                auto new_name = ir.symbols.New(mem_name);
                                const_cast<core::type::StructMember*>(mem)->SetName(new_name);
                                member_names.Add(new_name.NameView());
                            }
                        }
                    }
                    return nullptr;
                });
        }
    }

    /// Ensures that the identifier @p identifier resolves to a builtin symbol.
    void EnsureResolvesToBuiltin(std::string_view identifier) {
        if (auto* decl = Lookup(identifier)) {
            // Operand is shadowed
            auto* thing = decl->thing;
            declarations.Remove(identifier);
            Rename(thing, identifier);
        }
    }

    /// Registers the declaration @p thing in the current scope with the name @p name
    /// If there is an existing declaration with the given name in any parent scope then @p thing
    /// will be renamed.
    void Declare(CastableBase* thing, Symbol name) {
        // Check if the declaration would shadow another declaration in the current scope or any
        // parent scope, and rename it if so.
        if (auto* decl = Lookup(name.NameView())) {
            if (decl->thing == thing) {
                return;  // Already declared
            }
            name = Rename(thing, name.NameView());
        }
        // Add the declaration to the current scope. This replaces any declaration with the same
        // name that is no longer in scope.
        declarations.Replace(name.NameView(), Declaration{thing, active_scopes.Back()});
    }

    /// Rename changes the name of @p thing with the old name of @p old_name
    /// @returns the new name
    Symbol Rename(CastableBase* thing, std::string_view old_name) {
        Symbol new_name = ir.symbols.New(old_name);
        tint::Switch(
            thing,  //
            [&](core::ir::Value* value) { ir.SetName(value, new_name); },
            [&](core::type::Struct* str) { str->SetName(new_name); },  //
            TINT_ICE_ON_NO_MATCH);
        return new_name;
    }

    /// @return true if @p s is a builtin (non-user declared) structure.
    bool IsBuiltinStruct(const core::type::Struct* s) {
        return s->Name().NameView().starts_with("__");
    }
};

}  // namespace

Result<SuccessType> RenameConflicts(core::ir::Module& ir) {
    core::ir::AssertValid(ir, "before core.RenameConflicts");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
