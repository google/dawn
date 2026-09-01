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

#include "src/tint/lang/core/ir/structural_validator.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/void.h"

namespace tint::core::ir::validator {

void Structural::ValidateShaderIOAnnotations(const CastableBase* msg_anchor,
                                             const core::type::Type* ty,
                                             const std::optional<BindingPoint>& binding_point,
                                             const IOAttributes& attr,
                                             ShaderIOKind kind) {
    EnumSet<IOAnnotation> annotations;

    // Since there is no entries in the set at this point, this should never fail.
    TINT_ASSERT(AddIOAnnotationsFromIOAttributes(annotations, attr) == Success);

    if (binding_point.has_value()) {
        annotations.Add(IOAnnotation::kBindingPoint);
    }

    if (auto* mv = ty->As<core::type::MemoryView>()) {
        if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
            annotations.Add(IOAnnotation::kWorkgroup);
        }
    }

    if (ty->Is<core::type::Void>()) {
        if (!annotations.Empty()) {
            AddError(msg_anchor) << ToString(kind) << " with void type should never be annotated";
        }
        return;  // Early return because later rules assume non-void types.
    }

    if (attr.location.has_value()) {
        if (ir_.properties.Contains(Property::kAllowLocationForNumericComposites)) {
            std::function<bool(const core::type::Type*)> is_numeric =
                [&is_numeric](const core::type::Type* t) -> bool {
                t = t->UnwrapPtrOrRef();
                bool result = false;
                tint::Switch(
                    t,
                    [&](const core::type::Struct* s) {
                        for (auto* m : s->Members()) {
                            if (!is_numeric(m->Type())) {
                                return;
                            }
                        }
                        result = true;
                    },
                    [&](Default) {
                        auto* e = t->DeepestElement()->UnwrapPtrOrRef();
                        tint::Switch(
                            e,  //
                            [&](const core::type::Struct* s) { result = is_numeric(s); },
                            [&](Default) { result = e->IsNumericScalarOrVector(); });
                    });
                return result;
            };
            if (!is_numeric(ty)) {
                AddError(msg_anchor)
                    << ToString(kind)
                    << " with a location attribute must contain only numeric elements "
                    << ty->FriendlyName();
                return;
            }
        } else {
            if (!ty->UnwrapPtrOrRef()->IsNumericScalarOrVector()) {
                AddError(msg_anchor) << ToString(kind)
                                     << " with a location attribute must be a numeric scalar or "
                                        "vector, but has type "
                                     << ty->FriendlyName();
                return;
            }
        }
    }

    if (auto* ty_struct = ty->UnwrapPtrOrRef()->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            EnumSet<IOAnnotation> mem_annotations = annotations;
            auto add_result = AddIOAnnotationsFromIOAttributes(mem_annotations, mem->Attributes());
            if (add_result != Success) {
                AddError(msg_anchor)
                    << ToString(kind)
                    << " struct member has same IO annotation, as top-level struct, '"
                    << ToString(add_result.Failure()) << "'";
                return;
            }

            if (!CheckStructMemberAttributes(mem, [&]() -> diag::Diagnostic& {
                    return AddError(msg_anchor) << ToString(kind) << " ";
                })) {
                return;
            }

            if (ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
                if (auto* mv = mem->Type()->As<core::type::MemoryView>()) {
                    if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
                        mem_annotations.Add(IOAnnotation::kWorkgroup);
                    }
                }
            }

            if (mem_annotations.Empty()) {
                AddError(msg_anchor) << ToString(kind)
                                     << " struct members must have at least one IO annotation, "
                                        "e.g. a binding point, a location, etc";
            } else if (mem_annotations.Size() > 1) {
                AddError(msg_anchor)
                    << ToString(kind) << " struct member has more than one IO annotation, "
                    << ToString(mem_annotations);
            }
        }
    } else {
        if (annotations.Empty()) {
            if (!(ir_.properties.Contains(Property::kAllowUnannotatedModuleIOVariables) &&
                  kind == ShaderIOKind::kModuleScopeVar)) {
                AddError(msg_anchor) << ToString(kind)
                                     << " must have at least one IO annotation, e.g. a binding "
                                        "point, a location, etc";
            }
        } else if (annotations.Size() > 1) {
            AddError(msg_anchor) << ToString(kind) << " has more than one IO annotation, "
                                 << ToString(annotations);
        }
    }
}

void Structural::ValidateIOAttributes(const Function* func) {
    const auto stage = func->Stage();
    struct Task {
        const CastableBase* anchor;
        const core::type::Type* type;
        const IOAttributes& attr;
        IODirection dir;
        ShaderIOKind io_kind;
    };
    Vector<Task, 16> tasks;

    // Gather parameters.
    for (auto* param : func->Params()) {
        tasks.Push({param, param->Type(), param->Attributes(), IODirection::kInput,
                    ShaderIOKind::kInputParam});
    }

    // Gather return value.
    tasks.Push({func, func->ReturnType(), func->ReturnAttributes(), IODirection::kOutput,
                ShaderIOKind::kResultValue});

    // Gather referenced module variables.
    for (auto* var : referenced_module_vars_.TransitiveReferences(func)) {
        auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
        if (mv == nullptr) {
            continue;
        }
        if (mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut ||
            mv->AddressSpace() == AddressSpace::kHandle) {
            tasks.Push({var, mv->StoreType(), var->Attributes(),
                        validator::IODirectionFor(mv->AddressSpace()),
                        ShaderIOKind::kModuleScopeVar});
        }
    }

    if (stage != Function::PipelineStage::kUndefined) {
        // Shared context for blend_src and location validation
        BlendSrcContext input_ctx{func->Stage(), {}, {}, nullptr, IODirection::kInput};
        BlendSrcContext output_ctx{func->Stage(), {}, {}, nullptr, IODirection::kOutput};

        // First pass: pre-populate location hashes for blend_src.
        for (const auto& task : tasks) {
            auto& ctx = task.dir == IODirection::kInput ? input_ctx : output_ctx;
            WalkTypeAndMembers(
                ctx, task.type, task.attr,
                [task](BlendSrcContext& c, const core::type::Type*, const IOAttributes& a) {
                    if (a.blend_src.has_value() && a.location.has_value()) {
                        c.locations.Add(a.location.value(), task.anchor);
                    }
                });
        }

        // Second pass: validate blend_src usages.
        for (const auto& task : tasks) {
            auto& ctx = task.dir == IODirection::kInput ? input_ctx : output_ctx;
            CheckBlendSrc(ctx, task.anchor, task.type, task.attr);
        }

        if (!output_ctx.blend_srcs.IsEmpty()) {
            if (output_ctx.blend_srcs.Count() != 2) {
                AddError(func) << "if any @blend_src is used on an output, then @blend_src(0) and "
                                  "@blend_src(1) must be used";
            }
        }

        // Third pass: validate all non-blend_src location usages.
        for (const auto& task : tasks) {
            if (task.dir == IODirection::kInput) {
                CheckLocation(input_ctx.locations, task.anchor, task.attr, func->Stage(), task.type,
                              task.dir);
            } else if (task.dir == IODirection::kOutput) {
                CheckLocation(output_ctx.locations, task.anchor, task.attr, func->Stage(),
                              task.type, task.dir);
            }
        }
    }

    // Validate all the interpolation usages.
    for (const auto& task : tasks) {
        CheckInterpolation(task.anchor, task.type, task.attr, stage, task.dir);
    }

    if (stage != Function::PipelineStage::kUndefined) {
        // Validate all the binding_point usages, and ensure things that require binding_point have
        // them.
        for (const auto& task : tasks) {
            CheckBindingPoint(task.anchor, task.type, task.attr, task.io_kind);
        }
    }

    IOAttributeContext impl_ctx{.input_builtins = {}, .output_builtins = {}};
    // Validate all remaining attributes on IO objects
    for (const auto& task : tasks) {
        ValidateIOAttributesImpl(impl_ctx, task.anchor, task.type, task.attr, stage, task.dir,
                                 task.io_kind);
    }
}

void Structural::ValidateIOAttributesImpl(IOAttributeContext& ctx,
                                          const CastableBase* msg_anchor,
                                          const core::type::Type* ty,
                                          const IOAttributes& attr,
                                          Function::PipelineStage stage,
                                          IODirection dir,
                                          ShaderIOKind io_kind) {
    bool skip_builtins = ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO) &&
                         io_kind == ShaderIOKind::kModuleScopeVar;
    const IOAttributeUsage usage = IOAttributeUsageFor(stage, dir);
    WalkTypeAndMembers(
        *this, ty, attr,
        [&ctx, msg_anchor, usage, io_kind, skip_builtins, dir](
            Structural& v, const core::type::Type* t, const IOAttributes& a) {
            const auto checkers = IOAttributeCheckersFor(a, skip_builtins);
            if (checkers.IsEmpty()) {
                return;
            }

            if (a.builtin.has_value() && !skip_builtins &&
                usage != IOAttributeUsage::kUndefinedUsage) {
                const auto& builtin = a.builtin.value();

                uint32_t count = 0;
                switch (dir) {
                    case IODirection::kInput:
                        count = ++(ctx.input_builtins.GetOrAddZeroEntry(builtin).value);
                        break;
                    case IODirection::kOutput:
                        count = ++(ctx.output_builtins.GetOrAddZeroEntry(builtin).value);
                        break;
                    default:
                        // This shouldn't ever happen, but this will get caught later in the
                        // checker, so just ignoring
                        break;
                }
                if (v.ir_.properties.Contains(Property::kAllowClipDistancesOnF32ScalarAndVector) &&
                    builtin == BuiltinValue::kClipDistances) {
                    if (count > 2) {
                        v.AddError(msg_anchor)
                            << "too many instances of builtin 'clip_distances' on entry point "
                            << ToString(dir)
                            << ", only two allowed with 'kAllowClipDistancesOnF32ScalarAndVector' "
                               "property enabled";
                    }
                } else {
                    if (count > 1) {
                        v.AddError(msg_anchor)
                            << "duplicate instance of builtin '" << ToString(builtin)
                            << "' on entry point " << ToString(dir)
                            << ", must be unique per entry point i/o direction";
                    }
                }
            }

            auto failed = tint::Hashset<const IOAttributeChecker*, 4>();

            if (usage != IOAttributeUsage::kUndefinedUsage) {
                for (const auto* checker : checkers) {
                    if (!checker->valid_usages.Contains(usage)) {
                        failed.Add(checker);

                        std::stringstream msg;
                        msg << ToString(checker->kind) << " IO attributes cannot be declared for a "
                            << ToString(usage) << ". ";
                        if (checker->valid_usages.Size() == 1) {
                            const auto& u = *checker->valid_usages.begin();
                            msg << "They can only be used for a " << ToString(u) << ".";
                        } else {
                            msg << "They can only be used for " << ToString(checker->valid_usages);
                        }
                        v.AddError(msg_anchor) << msg.str();
                    }
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (!checker->valid_io_kinds.Contains(io_kind)) {
                    failed.Add(checker);

                    std::stringstream msg;
                    msg << ToString(checker->kind) << " IO attributes cannot be declared on a "
                        << ToString(io_kind) << ". ";
                    if (checker->valid_io_kinds.Size() == 1) {
                        const auto& k = *checker->valid_io_kinds.begin();
                        msg << "They can only be used on a " << ToString(k) << ".";
                    } else {
                        msg << "They can only be used on " << ToString(checker->valid_io_kinds);
                    }
                    v.AddError(msg_anchor) << msg.str();
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (!checker->type_check(t, v.ir_.properties)) {
                    failed.Add(checker);
                    v.AddError(msg_anchor) << ToString(checker->kind) << " " << checker->type_error;
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (auto res = checker->check(t, a, v.ir_.properties, usage); res != Success) {
                    failed.Add(checker);
                    v.AddError(msg_anchor) << res.Failure();
                }
            }
        });
}

void Structural::CheckNotBool(const CastableBase* msg_anchor,
                              const core::type::Type* ty,
                              const std::string& err) {
    if (ty->Is<core::type::Bool>()) {
        AddError(msg_anchor) << err;
    }
}

void Structural::CheckFrontFacingIfBool(const CastableBase* msg_anchor,
                                        const IOAttributes& attr,
                                        const core::type::Type* ty,
                                        const std::string& err) {
    if (ty->Is<core::type::Bool>() && attr.builtin != BuiltinValue::kFrontFacing) {
        AddError(msg_anchor) << err;
    }
}

void Structural::CheckBlendSrc(BlendSrcContext& ctx,
                               const CastableBase* target,
                               const core::type::Type* ty,
                               const IOAttributes& attr) {
    if (attr.blend_src.has_value()) {
        if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
            AddError(target) << "blend_src cannot be used on non-struct-member types";
        }
        CheckBlendSrcImpl(ctx, target, ty, attr);
    }

    if (auto* s = ty->As<core::type::Struct>()) {
        if (s->Members().Any([](auto* m) { return m->Attributes().blend_src.has_value(); })) {
            auto location_count = 0u;
            for (const auto* mem : s->Members()) {
                auto& mem_attr = mem->Attributes();
                if (mem_attr.location.has_value()) {
                    location_count++;
                }
                CheckBlendSrcImpl(ctx, target, mem->Type(), mem_attr);
            }

            if (location_count != 2) {
                AddError(target)
                    << "structs with blend_src members must have exactly 2 members with "
                       "location annotations";
            }
            return;
        }
    }

    // Reject blend_src on nested members
    if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
        WalkTypeAndMembers(
            ctx, ty, attr,
            [&target, this](BlendSrcContext&, const core::type::Type*, const IOAttributes& a) {
                if (a.blend_src.has_value()) {
                    AddError(target)
                        << "blend_src cannot be used on members of non-top level structs";
                }
            });
    }
}

void Structural::CheckBlendSrcImpl(BlendSrcContext& ctx,
                                   const CastableBase* target,
                                   const core::type::Type* ty,
                                   const IOAttributes& attr) {
    if (!attr.blend_src.has_value()) {
        return;
    }

    auto bs_val = attr.blend_src.value();
    if (bs_val != 0 && bs_val != 1) {
        AddError(target) << "blend_src value must be 0 or 1";
    }
    if (!ctx.blend_srcs.Add(bs_val)) {
        AddError(target) << "duplicate blend_src(" << bs_val << ") on entry point "
                         << ToString(ctx.dir);
    }

    if (ctx.dir != IODirection::kOutput || ctx.stage != Function::PipelineStage::kFragment) {
        AddError(target) << "blend_src can only be used on fragment shader outputs";
        return;
    }
    if (!attr.location.has_value() || attr.location.value() != 0) {
        AddError(target) << "struct members with blend_src must be located at 0";
    }

    if (!ctx.blend_src_type) {
        if (!ty->IsNumericScalarOrVector()) {
            AddError(target) << "blend_src must be a numeric scalar or vector, but has type "
                             << ty->FriendlyName();
        }
        ctx.blend_src_type = ty;
    } else if (ctx.blend_src_type != ty) {
        AddError(target) << "blend_src type " << ty->FriendlyName()
                         << " does not match other blend_src type "
                         << ctx.blend_src_type->FriendlyName();
    }
}

void Structural::CheckLocation(Hashmap<uint32_t, const CastableBase*, 4>& locations,
                               const CastableBase* target,
                               const IOAttributes& attr,
                               const Function::PipelineStage stage,
                               const core::type::Type* type,
                               const IODirection dir) {
    struct WalkContext {
        Structural* validator;
        Hashmap<uint32_t, const CastableBase*, 4>& locations;
        const CastableBase* target;
        const Function::PipelineStage stage;
        const IODirection dir;
    };
    WalkContext ctx{this, locations, target, stage, dir};

    WalkTypeAndMembers(
        ctx, type, attr,
        [](WalkContext& context, const core::type::Type* ty, const IOAttributes& attribute) {
            if (ty->Is<core::type::Struct>()) {
                return;
            }

            if (attribute.blend_src) {
                // locations associated with a blend_src usage should already be
                // pre-populated in locations
                return;
            }

            if (attribute.location.has_value()) {
                if (context.stage == Function::PipelineStage::kCompute &&
                    context.dir == IODirection::kInput) {
                    context.validator->AddError(context.target)
                        << "location attribute is not valid for compute shader inputs";
                }

                auto loc = attribute.location.value();
                if (const auto conflict = context.locations.Get(loc)) {
                    context.validator->AddError(context.target)
                        << "duplicate location(" << loc << ") on entry point "
                        << ToString(context.dir);
                    context.validator->AddDeclarationNote(*conflict.value);
                } else {
                    context.locations.Add(loc, context.target);
                }
            }
        });
}

void Structural::CheckInterpolation(const CastableBase* anchor,
                                    const core::type::Type* ty,
                                    const IOAttributes& attr,
                                    const Function::PipelineStage stage,
                                    const IODirection dir) {
    if (!ty) {
        return;
    }

    bool ctx = false;

    WalkTypeAndMembers(
        ctx, ty, attr,
        [this, anchor, stage, dir](bool& in_location_composite, const core::type::Type* t,
                                   const IOAttributes& a) {
            bool has_location = a.location.has_value() || in_location_composite;
            if (!has_location) {
                if (auto* str = t->As<core::type::Struct>()) {
                    has_location |= str->Members().All(
                        [](const auto* mem) { return mem->Attributes().location.has_value(); });
                }
            }

            if (a.interpolation.has_value()) {
                has_location |= (ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO) &&
                                 a.builtin.has_value());

                if (!ir_.properties.Contains(Property::kAllowLocationForNumericComposites) &&
                    t->As<core::type::Struct>()) {
                    AddError(anchor) << "interpolation cannot be applied to a struct without "
                                        "'kAllowLocationForNumericComposites' property";
                }

                if (t->IsIntegerScalarOrVector()) {
                    if (a.interpolation.value().type != InterpolationType::kFlat) {
                        AddError(anchor)
                            << "interpolation attribute type must be flat for integral types";
                    }
                }

                auto interp_type = a.interpolation.value().type;
                auto interp_sampling = a.interpolation.value().sampling;
                if (interp_sampling != InterpolationSampling::kUndefined) {
                    switch (interp_type) {
                        case InterpolationType::kFlat:
                            if (interp_sampling != InterpolationSampling::kFirst &&
                                interp_sampling != InterpolationSampling::kEither) {
                                AddError(anchor) << "flat interpolation can only use 'first', "
                                                    "'either' or undefined sampling parameters";
                            }
                            break;
                        case InterpolationType::kLinear:
                        case InterpolationType::kPerspective:
                            if (interp_sampling != InterpolationSampling::kCenter &&
                                interp_sampling != InterpolationSampling::kCentroid &&
                                interp_sampling != InterpolationSampling::kSample) {
                                AddError(anchor) << "linear and perspective interpolation can only "
                                                    "use 'center', 'centroid', 'sample', or "
                                                    "undefined sampling parameters";
                            }
                            break;
                        case InterpolationType::kUndefined:
                            AddError(anchor) << "undefined interpolation should on have an "
                                                "undefined sampling parameter";
                            break;
                        default:
                            TINT_UNREACHABLE();
                    }
                }

                if (!has_location) {
                    if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
                        AddError(anchor) << "interpolation attribute requires a location attribute";
                    } else {
                        AddError(anchor) << "interpolation attribute requires a location attribute "
                                            "(or location-like shader I/O annotation)";
                    }
                }
            } else if (has_location && t->IsIntegerScalarOrVector()) {
                // Integral vertex outputs and fragment inputs require flat interpolation.
                const bool needs_flat =
                    (stage == Function::PipelineStage::kVertex && dir == IODirection::kOutput) ||
                    (stage == Function::PipelineStage::kFragment && dir == IODirection::kInput);
                if (needs_flat) {
                    AddError(anchor) << "integral user-defined inputs and outputs must have an "
                                        "@interpolate(flat) attribute";
                }
            }

            if (t->IsAnyOf<core::type::Array, core::type::Struct>()) {
                in_location_composite |= a.location.has_value();
            }
        });
}

void Structural::CheckBindingPoint(const CastableBase* anchor,
                                   const core::type::Type* ty,
                                   const IOAttributes& attr,
                                   const ShaderIOKind& io_kind) {
    const auto& binding_point = attr.binding_point;
    auto address_space = AddressSpace::kUndefined;
    if (const auto* mv = ty->As<core::type::MemoryView>()) {
        address_space = mv->AddressSpace();
    } else {
        // ModuleScopeVars transform in MSL backends unwraps pointers to handles
        if (ty->IsHandle()) {
            address_space = AddressSpace::kHandle;
        }
    }

    if (binding_point.has_value() && io_kind != ShaderIOKind::kModuleScopeVar &&
        !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
        AddError(anchor) << "binding_points are only valid on resource variables";
    }

    switch (address_space) {
        case AddressSpace::kHandle:
            if (!binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " resource requires a binding point";
            }
            break;
        case AddressSpace::kStorage:
        case AddressSpace::kUniform:
            if (!binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " resource requires a binding point";
            }
            break;
        default:
            if (binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " non-resource cannot have a binding point";
            }
            break;
    }
}

}  // namespace tint::core::ir::validator
