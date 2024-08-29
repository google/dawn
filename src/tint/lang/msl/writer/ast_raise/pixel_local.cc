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

#include "src/tint/lang/msl/writer/ast_raise/pixel_local.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/utils/containers/transform.h"

TINT_INSTANTIATE_TYPEINFO(tint::msl::writer::PixelLocal);
TINT_INSTANTIATE_TYPEINFO(tint::msl::writer::PixelLocal::Config);

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::msl::writer {

/// PIMPL state for the transform
struct PixelLocal::State {
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};
    /// The transform config
    const Config& cfg;

    /// Constructor
    /// @param program the source program
    /// @param config the transform config
    State(const Program& program, const Config& config) : src(program), cfg(config) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        auto& sem = src.Sem();

        // If the pixel local extension isn't enabled, then there must be no use of pixel_local
        // variables, and so there's nothing for this transform to do.
        if (!sem.Module()->Extensions().Contains(
                wgsl::Extension::kChromiumExperimentalPixelLocal)) {
            return SkipTransform;
        }

        bool made_changes = false;

        // Change all module scope `var<pixel_local>` variables to `var<private>`.
        // We need to do this even if the variable is not referenced by the entry point as later
        // stages do not understand the pixel_local address space.
        for (auto* global : src.AST().GlobalVariables()) {
            if (auto* var = global->As<ast::Var>()) {
                if (sem.Get(var)->AddressSpace() == core::AddressSpace::kPixelLocal) {
                    // Change the 'var<pixel_local>' to 'var<private>'
                    ctx.Replace(var->declared_address_space, b.Expr(core::AddressSpace::kPrivate));
                    made_changes = true;
                }
            }
        }

        /// The pixel local struct
        Hashset<const sem::Struct*, 1> pixel_local_structs;

        // Find the entry points
        for (auto* fn : src.AST().Functions()) {
            if (!fn->IsEntryPoint()) {
                continue;
            }

            auto* entry_point = sem.Get(fn);

            // Look for a `var<pixel_local>` used by the entry point...
            for (auto* global : entry_point->TransitivelyReferencedGlobals()) {
                if (global->AddressSpace() != core::AddressSpace::kPixelLocal) {
                    continue;
                }

                // Obtain struct of the pixel local.
                auto* pixel_local_str = global->Type()->UnwrapRef()->As<sem::Struct>();
                if (pixel_local_structs.Add(pixel_local_str)) {
                    // Add an Color attribute to each member of the pixel_local structure.
                    for (auto* member : pixel_local_str->Members()) {
                        ctx.InsertBack(member->Declaration()->attributes,
                                       b.Color(u32(AttachmentIndex(member->Index()))));
                        ctx.InsertBack(member->Declaration()->attributes,
                                       b.Disable(ast::DisabledValidation::kEntryPointParameter));
                    }
                }

                TransformEntryPoint(entry_point, global, pixel_local_str);
                made_changes = true;
                break;  // Only a single `var<pixel_local>` can be used by an entry point.
            }
        }

        if (!made_changes) {
            return SkipTransform;
        }

        // At this point, the `var<pixel_local>` will have been replaced with `var<private>`, and
        // the entry point will use `@color`, which requires the framebuffer fetch extension.
        // Replace the `chromium_experimental_pixel_local` enable with
        // `chromium_experimental_framebuffer_fetch`.
        for (auto* enable : src.AST().Enables()) {
            for (auto* ext : enable->extensions) {
                if (ext->name == wgsl::Extension::kChromiumExperimentalPixelLocal) {
                    ctx.Replace(ext, b.create<ast::Extension>(
                                         wgsl::Extension::kChromiumExperimentalFramebufferFetch));
                }
            }
        }

        ctx.Clone();
        return resolver::Resolve(b);
    }

    /// Transforms the entry point @p entry_point to handle the direct or transitive usage of the
    /// `var<pixel_local>` @p pixel_local_var.
    /// @param entry_point the entry point
    /// @param pixel_local_var the `var<pixel_local>`
    /// @param pixel_local_str the struct type of the var
    void TransformEntryPoint(const sem::Function* entry_point,
                             const sem::GlobalVariable* pixel_local_var,
                             const sem::Struct* pixel_local_str) {
        auto* fn = entry_point->Declaration();
        auto fn_name = fn->name->symbol.Name();
        auto pixel_local_str_name = ctx.Clone(pixel_local_str->Name());
        auto pixel_local_var_name = ctx.Clone(pixel_local_var->Declaration()->name->symbol);

        // Remove the @fragment attribute from the entry point
        ctx.Remove(fn->attributes, ast::GetAttribute<ast::StageAttribute>(fn->attributes));
        // Rename the entry point
        auto inner_name = b.Symbols().New(fn_name + "_inner");
        ctx.Replace(fn->name, b.Ident(inner_name));

        // Create a new function that wraps the entry point.
        // This function has all the existing entry point parameters and an additional
        // parameter for the input pixel local structure.
        auto params = ctx.Clone(fn->params);
        auto pl_param = b.Symbols().New("pixel_local");
        params.Push(b.Param(pl_param, b.ty(pixel_local_str_name)));

        // Remove any entry-point attributes from the inner function.
        // This must come after `ctx.Clone(fn->params)` as we want these attributes on the outer
        // function.
        for (auto* param : fn->params) {
            for (auto* attr : param->attributes) {
                if (attr->IsAnyOf<ast::BuiltinAttribute, ast::LocationAttribute,
                                  ast::InterpolateAttribute, ast::InvariantAttribute>()) {
                    ctx.Remove(param->attributes, attr);
                }
            }
        }

        // Build the outer function's statements, starting with an assignment of the pixel local
        // parameter to the module scope var.
        Vector<const ast::Statement*, 3> body{
            b.Assign(pixel_local_var_name, pl_param),
        };

        // Build the arguments to call the inner function
        auto call_args =
            tint::Transform(fn->params, [&](auto* p) { return b.Expr(ctx.Clone(p->name)); });

        // Create a structure to hold the combined flattened result of the entry point and the pixel
        // local structure.
        auto str_name = b.Symbols().New(fn_name + "_res");
        Vector<const ast::StructMember*, 8> members;
        Vector<const ast::Expression*, 8> return_args;  // arguments to the final `return` statement

        auto add_member = [&](const core::type::Type* ty, VectorRef<const ast::Attribute*> attrs) {
            members.Push(b.Member("output_" + std::to_string(members.Length()),
                                  CreateASTTypeFor(ctx, ty), std::move(attrs)));
        };
        for (auto* member : pixel_local_str->Members()) {
            add_member(member->Type(), Vector{
                                           b.Location(AInt(AttachmentIndex(member->Index()))),
                                       });
            return_args.Push(b.MemberAccessor(pixel_local_var_name, ctx.Clone(member->Name())));
        }
        if (fn->return_type) {
            Symbol call_result = b.Symbols().New("result");
            if (auto* str = entry_point->ReturnType()->As<sem::Struct>()) {
                // The entry point returned a structure.
                for (auto* member : str->Members()) {
                    auto& member_attrs = member->Declaration()->attributes;
                    add_member(member->Type(), ctx.Clone(member_attrs));
                    return_args.Push(b.MemberAccessor(call_result, ctx.Clone(member->Name())));
                }
            } else {
                // The entry point returned a non-structure
                add_member(entry_point->ReturnType(), ctx.Clone(fn->return_type_attributes));
                return_args.Push(b.Expr(call_result));

                // Remove the @location from the inner function's return type attributes
                ctx.Remove(fn->return_type_attributes,
                           ast::GetAttribute<ast::LocationAttribute>(fn->return_type_attributes));
            }
            body.Push(b.Decl(b.Let(call_result, b.Call(inner_name, std::move(call_args)))));
        } else {
            body.Push(b.CallStmt(b.Call(inner_name, std::move(call_args))));
        }

        // Declare the output structure
        b.Structure(str_name, std::move(members));

        // Return the output structure
        body.Push(b.Return(b.Call(str_name, std::move(return_args))));

        // Declare the new entry point that calls the inner function
        b.Func(fn_name, std::move(params), b.ty(str_name), body,
               Vector{b.Stage(ast::PipelineStage::kFragment)});
    }

    /// @returns the attachment index for the pixel local field with the given index
    /// @param field_index the pixel local field index
    uint32_t AttachmentIndex(uint32_t field_index) {
        auto idx = cfg.attachments.Get(field_index);
        if (DAWN_UNLIKELY(!idx)) {
            b.Diagnostics().AddError(Source{})
                << "PixelLocal::Config::attachments missing entry for field " << field_index;
            return 0;
        }
        return *idx;
    }
};

PixelLocal::PixelLocal() = default;

PixelLocal::~PixelLocal() = default;

ast::transform::Transform::ApplyResult PixelLocal::Apply(const Program& src,
                                                         const ast::transform::DataMap& inputs,
                                                         ast::transform::DataMap&) const {
    auto* cfg = inputs.Get<Config>();
    if (!cfg) {
        ProgramBuilder b;
        b.Diagnostics().AddError(Source{}) << "missing transform data for " << TypeInfo().name;
        return resolver::Resolve(b);
    }

    return State(src, *cfg).Run();
}

PixelLocal::Config::Config() = default;

PixelLocal::Config::Config(const Config&) = default;

PixelLocal::Config::~Config() = default;

}  // namespace tint::msl::writer
