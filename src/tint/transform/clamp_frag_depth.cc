// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/clamp_frag_depth.h"

 #include <utility>

#include "src/tint/ast/attribute.h"
#include "src/tint/ast/builtin_attribute.h"
#include "src/tint/ast/builtin_value.h"
#include "src/tint/ast/function.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/struct.h"
#include "src/tint/ast/type.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/struct.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/utils/vector.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::ClampFragDepth);

namespace tint::transform {

namespace {

bool ContainsFragDepth(utils::VectorRef<const ast::Attribute*> attributes) {
    for (auto* attribute : attributes) {
        if (auto* builtin_attribute = attribute->As<ast::BuiltinAttribute>()) {
            if (builtin_attribute->builtin == ast::BuiltinValue::kFragDepth) {
                return true;
            }
        }
    }

    return false;
}

bool ReturnsFragDepthAsValue(const ast::Function* fn) {
    return ContainsFragDepth(fn->return_type_attributes);
}

bool ReturnsFragDepthInStruct(const sem::Info& sem, const ast::Function* fn) {
    if (auto* struct_ty = sem.Get(fn)->ReturnType()->As<sem::Struct>()) {
        for (auto* member : struct_ty->Members()) {
            if (ContainsFragDepth(member->Declaration()->attributes)) {
                return true;
            }
        }
    }

    return false;
}

}  // anonymous namespace

ClampFragDepth::ClampFragDepth() = default;
ClampFragDepth::~ClampFragDepth() = default;

bool ClampFragDepth::ShouldRun(const Program* program, const DataMap&) const {
    auto& sem = program->Sem();

    for (auto* fn : program->AST().Functions()) {
        if (fn->PipelineStage() == ast::PipelineStage::kFragment &&
            (ReturnsFragDepthAsValue(fn) || ReturnsFragDepthInStruct(sem, fn))) {
            return true;
        }
    }

    return false;
}

void ClampFragDepth::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    // Abort on any use of push constants in the module.
    for (auto* global : ctx.src->AST().GlobalVariables()) {
        if (auto* var = global->As<ast::Var>()) {
            if (var->declared_address_space == ast::AddressSpace::kPushConstant) {
                TINT_ICE(Transform, ctx.dst->Diagnostics())
                    << "ClampFragDepth doesn't know how to handle module that already use push "
                       "constants.";
                return;
            }
        }
    }

    auto& b = *ctx.dst;
    auto& sem = ctx.src->Sem();
    auto& sym = ctx.src->Symbols();

    // At least one entry-point needs clamping. Add the following to the module:
    //
    //   enable chromium_experimental_push_constant;
    //
    //   struct FragDepthClampArgs {
    //       min : f32,
    //       max : f32,
    //   }
    //   var<push_constant> frag_depth_clamp_args : FragDepthClampArgs;
    //
    //   fn clamp_frag_depth(v : f32) -> f32 {
    //       return clamp(v, frag_depth_clamp_args.min, frag_depth_clamp_args.max);
    //   }
    b.Enable(ast::Extension::kChromiumExperimentalPushConstant);

    b.Structure(b.Symbols().New("FragDepthClampArgs"),
                utils::Vector{b.Member("min", b.ty.f32()), b.Member("max", b.ty.f32())});

    auto args_sym = b.Symbols().New("frag_depth_clamp_args");
    b.GlobalVar(args_sym, b.ty.type_name("FragDepthClampArgs"), ast::AddressSpace::kPushConstant);

    auto base_fn_sym = b.Symbols().New("clamp_frag_depth");
    b.Func(base_fn_sym, utils::Vector{b.Param("v", b.ty.f32())}, b.ty.f32(),
           utils::Vector{b.Return(b.Call("clamp", "v", b.MemberAccessor(args_sym, "min"),
                                         b.MemberAccessor(args_sym, "max")))});

    // If true, the currently cloned function returns frag depth directly as a scalar
    bool returns_frag_depth_as_value = false;

    // If valid, the currently cloned function returns frag depth in a struct
    // The symbol is the name of the helper function to apply the depth clamping.
    Symbol returns_frag_depth_as_struct_helper;

    // Map of io struct to helper function to return the structure with the depth clamping applied.
    utils::Hashmap<const ast::Struct*, Symbol, 4u> io_structs_clamp_helpers;

    // Register a callback that will be called for each visted AST function.
    // This call wraps the cloning of the function's statements, and will assign to
    // `returns_frag_depth_as_value` or `returns_frag_depth_as_struct_helper` if the function's
    // return value requires depth clamping.
    ctx.ReplaceAll([&](const ast::Function* fn) {
        if (fn->PipelineStage() != ast::PipelineStage::kFragment) {
            return ctx.CloneWithoutTransform(fn);
        }

        if (ReturnsFragDepthAsValue(fn)) {
            TINT_SCOPED_ASSIGNMENT(returns_frag_depth_as_value, true);
            return ctx.CloneWithoutTransform(fn);
        }

        if (ReturnsFragDepthInStruct(sem, fn)) {
            // At most once per I/O struct, add the conversion function:
            //
            //   fn clamp_frag_depth_S(s : S) -> S {
            //       return S(s.first, s.second, clamp_frag_depth(s.frag_depth), s.last);
            //   }
            auto* struct_ty = sem.Get(fn)->ReturnType()->As<sem::Struct>()->Declaration();
            auto helper = io_structs_clamp_helpers.GetOrCreate(struct_ty, [&] {
                auto* return_ty = fn->return_type;
                auto fn_sym = b.Symbols().New("clamp_frag_depth_" +
                                              sym.NameFor(return_ty->As<ast::TypeName>()->name));

                utils::Vector<const ast::Expression*, 8u> constructor_args;
                for (auto* member : struct_ty->members) {
                    const ast::Expression* arg = b.MemberAccessor("s", ctx.Clone(member->symbol));
                    if (ContainsFragDepth(member->attributes)) {
                        arg = b.Call(base_fn_sym, arg);
                    }
                    constructor_args.Push(arg);
                }
                utils::Vector params{b.Param("s", ctx.Clone(return_ty))};
                utils::Vector body{
                    b.Return(b.Construct(ctx.Clone(return_ty), std::move(constructor_args))),
                };
                b.Func(fn_sym, params, ctx.Clone(return_ty), body);
                return fn_sym;
            });

            TINT_SCOPED_ASSIGNMENT(returns_frag_depth_as_struct_helper, helper);
            return ctx.CloneWithoutTransform(fn);
        }

        return ctx.CloneWithoutTransform(fn);
    });

    // Replace the return statements `return expr` with `return clamp_frag_depth(expr)`.
    ctx.ReplaceAll([&](const ast::ReturnStatement* stmt) -> const ast::ReturnStatement* {
        if (returns_frag_depth_as_value) {
            return b.Return(stmt->source, b.Call(base_fn_sym, ctx.Clone(stmt->value)));
        }
        if (returns_frag_depth_as_struct_helper.IsValid()) {
            return b.Return(stmt->source,
                            b.Call(returns_frag_depth_as_struct_helper, ctx.Clone(stmt->value)));
        }
        return nullptr;
    });

    ctx.Clone();
}

}  // namespace tint::transform
