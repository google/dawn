// Copyright 2020 The Tint Authors.
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

#include "src/tint/resolver/validator.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "src/tint/ast/alias.h"
#include "src/tint/ast/array.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/depth_texture.h"
#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/matrix.h"
#include "src/tint/ast/pointer.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/sampled_texture.h"
#include "src/tint/ast/sampler.h"
#include "src/tint/ast/storage_texture.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/ast/type_name.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/vector.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/sem/abstract_numeric.h"
#include "src/tint/sem/array.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/break_if_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/depth_multisampled_texture.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/if_statement.h"
#include "src/tint/sem/loop_statement.h"
#include "src/tint/sem/materialize.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/pointer.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/sampler.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/sem/type_conversion.h"
#include "src/tint/sem/type_initializer.h"
#include "src/tint/sem/variable.h"
#include "src/tint/sem/while_statement.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/math.h"
#include "src/tint/utils/reverse.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/utils/string.h"
#include "src/tint/utils/transform.h"

namespace tint::resolver {
namespace {

bool IsValidStorageTextureDimension(ast::TextureDimension dim) {
    switch (dim) {
        case ast::TextureDimension::k1d:
        case ast::TextureDimension::k2d:
        case ast::TextureDimension::k2dArray:
        case ast::TextureDimension::k3d:
            return true;
        default:
            return false;
    }
}

bool IsValidStorageTextureTexelFormat(ast::TexelFormat format) {
    switch (format) {
        case ast::TexelFormat::kR32Uint:
        case ast::TexelFormat::kR32Sint:
        case ast::TexelFormat::kR32Float:
        case ast::TexelFormat::kRg32Uint:
        case ast::TexelFormat::kRg32Sint:
        case ast::TexelFormat::kRg32Float:
        case ast::TexelFormat::kRgba8Unorm:
        case ast::TexelFormat::kRgba8Snorm:
        case ast::TexelFormat::kRgba8Uint:
        case ast::TexelFormat::kRgba8Sint:
        case ast::TexelFormat::kRgba16Uint:
        case ast::TexelFormat::kRgba16Sint:
        case ast::TexelFormat::kRgba16Float:
        case ast::TexelFormat::kRgba32Uint:
        case ast::TexelFormat::kRgba32Sint:
        case ast::TexelFormat::kRgba32Float:
            return true;
        default:
            return false;
    }
}

// Helper to stringify a pipeline IO attribute.
std::string attr_to_str(const ast::Attribute* attr,
                        std::optional<uint32_t> location = std::nullopt) {
    std::stringstream str;
    if (auto* builtin = attr->As<ast::BuiltinAttribute>()) {
        str << "builtin(" << builtin->builtin << ")";
    } else if (attr->Is<ast::LocationAttribute>()) {
        str << "location(" << location.value() << ")";
    }
    return str.str();
}

template <typename CALLBACK>
void TraverseCallChain(diag::List& diagnostics,
                       const sem::Function* from,
                       const sem::Function* to,
                       CALLBACK&& callback) {
    for (auto* f : from->TransitivelyCalledFunctions()) {
        if (f == to) {
            callback(f);
            return;
        }
        if (f->TransitivelyCalledFunctions().Contains(to)) {
            TraverseCallChain(diagnostics, f, to, callback);
            callback(f);
            return;
        }
    }
    TINT_ICE(Resolver, diagnostics) << "TraverseCallChain() 'from' does not transitively call 'to'";
}

}  // namespace

Validator::Validator(ProgramBuilder* builder, SemHelper& sem)
    : symbols_(builder->Symbols()), diagnostics_(builder->Diagnostics()), sem_(sem) {}

Validator::~Validator() = default;

void Validator::AddError(const std::string& msg, const Source& source) const {
    diagnostics_.add_error(diag::System::Resolver, msg, source);
}

void Validator::AddWarning(const std::string& msg, const Source& source) const {
    diagnostics_.add_warning(diag::System::Resolver, msg, source);
}

void Validator::AddNote(const std::string& msg, const Source& source) const {
    diagnostics_.add_note(diag::System::Resolver, msg, source);
}

// https://gpuweb.github.io/gpuweb/wgsl/#plain-types-section
bool Validator::IsPlain(const sem::Type* type) const {
    return type->is_scalar() ||
           type->IsAnyOf<sem::Atomic, sem::Vector, sem::Matrix, sem::Array, sem::Struct>();
}

// https://gpuweb.github.io/gpuweb/wgsl/#fixed-footprint-types
bool Validator::IsFixedFootprint(const sem::Type* type) const {
    return Switch(
        type,                                      //
        [&](const sem::Vector*) { return true; },  //
        [&](const sem::Matrix*) { return true; },  //
        [&](const sem::Atomic*) { return true; },
        [&](const sem::Array* arr) {
            return !arr->IsRuntimeSized() && IsFixedFootprint(arr->ElemType());
        },
        [&](const sem::Struct* str) {
            for (auto* member : str->Members()) {
                if (!IsFixedFootprint(member->Type())) {
                    return false;
                }
            }
            return true;
        },
        [&](Default) { return type->is_scalar(); });
}

// https://gpuweb.github.io/gpuweb/wgsl.html#host-shareable-types
bool Validator::IsHostShareable(const sem::Type* type) const {
    if (type->IsAnyOf<sem::I32, sem::U32, sem::F32, sem::F16>()) {
        return true;
    }
    return Switch(
        type,  //
        [&](const sem::Vector* vec) { return IsHostShareable(vec->type()); },
        [&](const sem::Matrix* mat) { return IsHostShareable(mat->type()); },
        [&](const sem::Array* arr) { return IsHostShareable(arr->ElemType()); },
        [&](const sem::Struct* str) {
            for (auto* member : str->Members()) {
                if (!IsHostShareable(member->Type())) {
                    return false;
                }
            }
            return true;
        },
        [&](const sem::Atomic* atomic) { return IsHostShareable(atomic->Type()); });
}

// https://gpuweb.github.io/gpuweb/wgsl.html#storable-types
bool Validator::IsStorable(const sem::Type* type) const {
    return IsPlain(type) || type->IsAnyOf<sem::Texture, sem::Sampler>();
}

const ast::Statement* Validator::ClosestContinuing(bool stop_at_loop,
                                                   sem::Statement* current_statement) const {
    for (const auto* s = current_statement; s != nullptr; s = s->Parent()) {
        if (stop_at_loop && s->Is<sem::LoopStatement>()) {
            break;
        }
        if (s->Is<sem::LoopContinuingBlockStatement>()) {
            return s->Declaration();
        }
        if (auto* f = As<sem::ForLoopStatement>(s->Parent())) {
            if (f->Declaration()->continuing == s->Declaration()) {
                return s->Declaration();
            }
            if (stop_at_loop) {
                break;
            }
        }
        if (Is<sem::WhileStatement>(s->Parent())) {
            if (stop_at_loop) {
                break;
            }
        }
    }
    return nullptr;
}

bool Validator::Atomic(const ast::Atomic* a, const sem::Atomic* s) const {
    // https://gpuweb.github.io/gpuweb/wgsl/#atomic-types
    // T must be either u32 or i32.
    if (!s->Type()->IsAnyOf<sem::U32, sem::I32>()) {
        AddError("atomic only supports i32 or u32 types", a->type ? a->type->source : a->source);
        return false;
    }
    return true;
}

bool Validator::StorageTexture(const ast::StorageTexture* t) const {
    switch (t->access) {
        case ast::Access::kWrite:
            break;
        case ast::Access::kUndefined:
            AddError("storage texture missing access control", t->source);
            return false;
        default:
            AddError("storage textures currently only support 'write' access control", t->source);
            return false;
    }

    if (!IsValidStorageTextureDimension(t->dim)) {
        AddError("cube dimensions for storage textures are not supported", t->source);
        return false;
    }

    if (!IsValidStorageTextureTexelFormat(t->format)) {
        AddError(
            "image format must be one of the texel formats specified for storage "
            "textues in https://gpuweb.github.io/gpuweb/wgsl/#texel-formats",
            t->source);
        return false;
    }
    return true;
}

bool Validator::SampledTexture(const sem::SampledTexture* t, const Source& source) const {
    if (!t->type()->UnwrapRef()->IsAnyOf<sem::F32, sem::I32, sem::U32>()) {
        AddError("texture_2d<type>: type must be f32, i32 or u32", source);
        return false;
    }

    return true;
}

bool Validator::MultisampledTexture(const sem::MultisampledTexture* t, const Source& source) const {
    if (t->dim() != ast::TextureDimension::k2d) {
        AddError("only 2d multisampled textures are supported", source);
        return false;
    }

    if (!t->type()->UnwrapRef()->IsAnyOf<sem::F32, sem::I32, sem::U32>()) {
        AddError("texture_multisampled_2d<type>: type must be f32, i32 or u32", source);
        return false;
    }

    return true;
}

bool Validator::Materialize(const sem::Type* to,
                            const sem::Type* from,
                            const Source& source) const {
    if (sem::Type::ConversionRank(from, to) == sem::Type::kNoConversion) {
        AddError("cannot convert value of type '" + sem_.TypeNameOf(from) + "' to type '" +
                     sem_.TypeNameOf(to) + "'",
                 source);
        return false;
    }
    return true;
}

bool Validator::VariableInitializer(const ast::Variable* v,
                                    ast::AddressSpace address_space,
                                    const sem::Type* storage_ty,
                                    const sem::Expression* initializer) const {
    auto* initializer_ty = initializer->Type();
    auto* value_type = initializer_ty->UnwrapRef();  // Implicit load of RHS

    // Value type has to match storage type
    if (storage_ty != value_type) {
        std::stringstream s;
        s << "cannot initialize " << v->Kind() << " of type '" << sem_.TypeNameOf(storage_ty)
          << "' with value of type '" << sem_.TypeNameOf(initializer_ty) << "'";
        AddError(s.str(), v->source);
        return false;
    }

    if (v->Is<ast::Var>()) {
        switch (address_space) {
            case ast::AddressSpace::kPrivate:
            case ast::AddressSpace::kFunction:
                break;  // Allowed an initializer
            default:
                // https://gpuweb.github.io/gpuweb/wgsl/#var-and-let
                // Optionally has an initializer expression, if the variable is in the private or
                // function address spacees.
                AddError("var of address space '" + utils::ToString(address_space) +
                             "' cannot have an initializer. var initializers are only "
                             "supported for the address spacees "
                             "'private' and 'function'",
                         v->source);
                return false;
        }
    }

    return true;
}

bool Validator::AddressSpaceLayout(const sem::Type* store_ty,
                                   ast::AddressSpace address_space,
                                   Source source,
                                   ValidTypeStorageLayouts& layouts) const {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class-layout-constraints

    auto is_uniform_struct_or_array = [address_space](const sem::Type* ty) {
        return address_space == ast::AddressSpace::kUniform &&
               ty->IsAnyOf<sem::Array, sem::Struct>();
    };

    auto is_uniform_struct = [address_space](const sem::Type* ty) {
        return address_space == ast::AddressSpace::kUniform && ty->Is<sem::Struct>();
    };

    auto required_alignment_of = [&](const sem::Type* ty) {
        uint32_t actual_align = ty->Align();
        uint32_t required_align = actual_align;
        if (is_uniform_struct_or_array(ty)) {
            required_align = utils::RoundUp(16u, actual_align);
        }
        return required_align;
    };

    auto member_name_of = [this](const sem::StructMember* sm) {
        return symbols_.NameFor(sm->Declaration()->symbol);
    };

    // Cache result of type + address space pair.
    if (!layouts.emplace(store_ty, address_space).second) {
        return true;
    }

    if (!ast::IsHostShareable(address_space)) {
        return true;
    }

    // Temporally forbid using f16 types in "uniform" and "storage" address space.
    // TODO(tint:1473, tint:1502): Remove this error after f16 is supported in "uniform" and
    // "storage" address space but keep for "push_constant" address space.
    if (Is<sem::F16>(sem::Type::DeepestElementOf(store_ty))) {
        AddError("using f16 types in '" + utils::ToString(address_space) +
                     "' address space is not implemented yet",
                 source);
        return false;
    }

    if (auto* str = store_ty->As<sem::Struct>()) {
        for (size_t i = 0; i < str->Members().size(); ++i) {
            auto* const m = str->Members()[i];
            uint32_t required_align = required_alignment_of(m->Type());

            // Recurse into the member type.
            if (!AddressSpaceLayout(m->Type(), address_space, m->Declaration()->type->source,
                                    layouts)) {
                AddNote("see layout of struct:\n" + str->Layout(symbols_),
                        str->Declaration()->source);
                return false;
            }

            // Validate that member is at a valid byte offset
            if (m->Offset() % required_align != 0) {
                AddError("the offset of a struct member of type '" +
                             m->Type()->UnwrapRef()->FriendlyName(symbols_) +
                             "' in address space '" + utils::ToString(address_space) +
                             "' must be a multiple of " + std::to_string(required_align) +
                             " bytes, but '" + member_name_of(m) + "' is currently at offset " +
                             std::to_string(m->Offset()) + ". Consider setting @align(" +
                             std::to_string(required_align) + ") on this member",
                         m->Declaration()->source);

                AddNote("see layout of struct:\n" + str->Layout(symbols_),
                        str->Declaration()->source);

                if (auto* member_str = m->Type()->As<sem::Struct>()) {
                    AddNote("and layout of struct member:\n" + member_str->Layout(symbols_),
                            member_str->Declaration()->source);
                }

                return false;
            }

            // For uniform buffers, validate that the number of bytes between the previous member of
            // type struct and the current is a multiple of 16 bytes.
            auto* const prev_member = (i == 0) ? nullptr : str->Members()[i - 1];
            if (prev_member && is_uniform_struct(prev_member->Type())) {
                const uint32_t prev_to_curr_offset = m->Offset() - prev_member->Offset();
                if (prev_to_curr_offset % 16 != 0) {
                    AddError(
                        "uniform storage requires that the number of bytes between the "
                        "start of the previous member of type struct and the current "
                        "member be a multiple of 16 bytes, but there are currently " +
                            std::to_string(prev_to_curr_offset) + " bytes between '" +
                            member_name_of(prev_member) + "' and '" + member_name_of(m) +
                            "'. Consider setting @align(16) on this member",
                        m->Declaration()->source);

                    AddNote("see layout of struct:\n" + str->Layout(symbols_),
                            str->Declaration()->source);

                    auto* prev_member_str = prev_member->Type()->As<sem::Struct>();
                    AddNote("and layout of previous member struct:\n" +
                                prev_member_str->Layout(symbols_),
                            prev_member_str->Declaration()->source);
                    return false;
                }
            }
        }
    }

    // For uniform buffer array members, validate that array elements are aligned to 16 bytes
    if (auto* arr = store_ty->As<sem::Array>()) {
        // Recurse into the element type.
        // TODO(crbug.com/tint/1388): Ideally we'd pass the source for nested element type here, but
        // we can't easily get that from the semantic node. We should consider recursing through the
        // AST type nodes instead.
        if (!AddressSpaceLayout(arr->ElemType(), address_space, source, layouts)) {
            return false;
        }

        if (address_space == ast::AddressSpace::kUniform) {
            // We already validated that this array member is itself aligned to 16 bytes above, so
            // we only need to validate that stride is a multiple of 16 bytes.
            if (arr->Stride() % 16 != 0) {
                // Since WGSL has no stride attribute, try to provide a useful hint for how the
                // shader author can resolve the issue.
                std::string hint;
                if (arr->ElemType()->is_scalar()) {
                    hint =
                        "Consider using a vector or struct as the element type "
                        "instead.";
                } else if (auto* vec = arr->ElemType()->As<sem::Vector>();
                           vec && vec->type()->Size() == 4) {
                    hint = "Consider using a vec4 instead.";
                } else if (arr->ElemType()->Is<sem::Struct>()) {
                    hint =
                        "Consider using the @size attribute on the last struct "
                        "member.";
                } else {
                    hint =
                        "Consider wrapping the element type in a struct and using "
                        "the "
                        "@size attribute.";
                }
                AddError(
                    "uniform storage requires that array elements be aligned to 16 "
                    "bytes, but array element alignment is currently " +
                        std::to_string(arr->Stride()) + ". " + hint,
                    source);
                return false;
            }
        }
    }

    return true;
}

bool Validator::AddressSpaceLayout(const sem::Variable* var,
                                   const ast::Extensions& enabled_extensions,
                                   ValidTypeStorageLayouts& layouts) const {
    if (var->AddressSpace() == ast::AddressSpace::kPushConstant &&
        !enabled_extensions.Contains(ast::Extension::kChromiumExperimentalPushConstant) &&
        IsValidationEnabled(var->Declaration()->attributes,
                            ast::DisabledValidation::kIgnoreAddressSpace)) {
        AddError(
            "use of variable address space 'push_constant' requires enabling extension "
            "'chromium_experimental_push_constant'",
            var->Declaration()->source);
        return false;
    }

    if (auto* str = var->Type()->UnwrapRef()->As<sem::Struct>()) {
        if (!AddressSpaceLayout(str, var->AddressSpace(), str->Declaration()->source, layouts)) {
            AddNote("see declaration of variable", var->Declaration()->source);
            return false;
        }
    } else {
        Source source = var->Declaration()->source;
        if (var->Declaration()->type) {
            source = var->Declaration()->type->source;
        }
        if (!AddressSpaceLayout(var->Type()->UnwrapRef(), var->AddressSpace(), source, layouts)) {
            return false;
        }
    }

    return true;
}

bool Validator::LocalVariable(const sem::Variable* local) const {
    auto* decl = local->Declaration();
    if (IsArrayWithOverrideCount(local->Type())) {
        RaiseArrayWithOverrideCountError(decl->type ? decl->type->source
                                                    : decl->initializer->source);
        return false;
    }
    return Switch(
        decl,  //
        [&](const ast::Var* var) {
            if (IsValidationEnabled(var->attributes,
                                    ast::DisabledValidation::kIgnoreAddressSpace)) {
                if (!local->Type()->UnwrapRef()->IsConstructible()) {
                    AddError("function-scope 'var' must have a constructible type",
                             var->type ? var->type->source : var->source);
                    return false;
                }
            }
            return Var(local);
        },                                            //
        [&](const ast::Let*) { return Let(local); },  //
        [&](const ast::Const*) { return true; },      //
        [&](Default) {
            TINT_ICE(Resolver, diagnostics_)
                << "Validator::Variable() called with a unknown variable type: "
                << decl->TypeInfo().name;
            return false;
        });
}

bool Validator::GlobalVariable(
    const sem::GlobalVariable* global,
    const utils::Hashmap<OverrideId, const sem::Variable*, 8>& override_ids,
    const utils::Hashmap<const sem::Type*, const Source*, 8>& atomic_composite_info) const {
    auto* decl = global->Declaration();
    if (global->AddressSpace() != ast::AddressSpace::kWorkgroup &&
        IsArrayWithOverrideCount(global->Type())) {
        RaiseArrayWithOverrideCountError(decl->type ? decl->type->source
                                                    : decl->initializer->source);
        return false;
    }
    bool ok = Switch(
        decl,  //
        [&](const ast::Var* var) {
            if (auto* init = global->Initializer();
                init && init->Stage() > sem::EvaluationStage::kOverride) {
                AddError("module-scope 'var' initializer must be a constant or override-expression",
                         init->Declaration()->source);
                return false;
            }

            if (global->AddressSpace() == ast::AddressSpace::kNone) {
                AddError("module-scope 'var' declaration must have a address space", decl->source);
                return false;
            }

            for (auto* attr : decl->attributes) {
                bool is_shader_io_attribute =
                    attr->IsAnyOf<ast::BuiltinAttribute, ast::InterpolateAttribute,
                                  ast::InvariantAttribute, ast::LocationAttribute>();
                bool has_io_address_space = global->AddressSpace() == ast::AddressSpace::kIn ||
                                            global->AddressSpace() == ast::AddressSpace::kOut;
                if (!attr->IsAnyOf<ast::BindingAttribute, ast::GroupAttribute,
                                   ast::InternalAttribute>() &&
                    (!is_shader_io_attribute || !has_io_address_space)) {
                    AddError("attribute '" + attr->Name() + "' is not valid for module-scope 'var'",
                             attr->source);
                    return false;
                }
            }

            // https://gpuweb.github.io/gpuweb/wgsl/#variable-declaration
            // The access mode always has a default, and except for variables in the storage address
            // space, must not be written.
            if (var->declared_access != ast::Access::kUndefined) {
                if (global->AddressSpace() == ast::AddressSpace::kStorage) {
                    // The access mode for the storage address space can only be 'read' or
                    // 'read_write'.
                    if (var->declared_access == ast::Access::kWrite) {
                        AddError("access mode 'write' is not valid for the 'storage' address space",
                                 decl->source);
                        return false;
                    }
                } else {
                    AddError("only variables in <storage> address space may declare an access mode",
                             decl->source);
                    return false;
                }
            }

            if (!AtomicVariable(global, atomic_composite_info)) {
                return false;
            }

            return Var(global);
        },
        [&](const ast::Override*) { return Override(global, override_ids); },
        [&](const ast::Const*) {
            if (!decl->attributes.IsEmpty()) {
                AddError("attribute is not valid for module-scope 'const' declaration",
                         decl->attributes[0]->source);
                return false;
            }
            return Const(global);
        },
        [&](Default) {
            TINT_ICE(Resolver, diagnostics_)
                << "Validator::GlobalVariable() called with a unknown variable type: "
                << decl->TypeInfo().name;
            return false;
        });

    if (!ok) {
        return false;
    }

    if (global->AddressSpace() == ast::AddressSpace::kFunction) {
        AddError("module-scope 'var' must not use address space 'function'", decl->source);
        return false;
    }

    switch (global->AddressSpace()) {
        case ast::AddressSpace::kUniform:
        case ast::AddressSpace::kStorage:
        case ast::AddressSpace::kHandle: {
            // https://gpuweb.github.io/gpuweb/wgsl/#resource-interface
            // Each resource variable must be declared with both group and binding attributes.
            if (!decl->HasBindingPoint()) {
                AddError("resource variables require @group and @binding attributes", decl->source);
                return false;
            }
            break;
        }
        default: {
            auto* binding_attr = ast::GetAttribute<ast::BindingAttribute>(decl->attributes);
            auto* group_attr = ast::GetAttribute<ast::GroupAttribute>(decl->attributes);
            if (binding_attr || group_attr) {
                // https://gpuweb.github.io/gpuweb/wgsl/#attribute-binding
                // Must only be applied to a resource variable
                AddError("non-resource variables must not have @group or @binding attributes",
                         decl->source);
                return false;
            }
        }
    }

    return true;
}

// https://gpuweb.github.io/gpuweb/wgsl/#atomic-types
// Atomic types may only be instantiated by variables in the workgroup storage class or by storage
// buffer variables with a read_write access mode.
bool Validator::AtomicVariable(
    const sem::Variable* var,
    const utils::Hashmap<const sem::Type*, const Source*, 8>& atomic_composite_info) const {
    auto address_space = var->AddressSpace();
    auto* decl = var->Declaration();
    auto access = var->Access();
    auto* type = var->Type()->UnwrapRef();
    auto source = decl->type ? decl->type->source : decl->source;

    if (type->Is<sem::Atomic>()) {
        if (address_space != ast::AddressSpace::kWorkgroup &&
            address_space != ast::AddressSpace::kStorage) {
            AddError("atomic variables must have <storage> or <workgroup> address space", source);
            return false;
        }
    } else if (type->IsAnyOf<sem::Struct, sem::Array>()) {
        if (auto* found = atomic_composite_info.Find(type)) {
            if (address_space != ast::AddressSpace::kStorage &&
                address_space != ast::AddressSpace::kWorkgroup) {
                AddError("atomic variables must have <storage> or <workgroup> address space",
                         source);
                AddNote("atomic sub-type of '" + sem_.TypeNameOf(type) + "' is declared here",
                        **found);
                return false;
            } else if (address_space == ast::AddressSpace::kStorage &&
                       access != ast::Access::kReadWrite) {
                AddError(
                    "atomic variables in <storage> address space must have read_write "
                    "access mode",
                    source);
                AddNote("atomic sub-type of '" + sem_.TypeNameOf(type) + "' is declared here",
                        **found);
                return false;
            }
        }
    }

    return true;
}

bool Validator::Var(const sem::Variable* v) const {
    auto* var = v->Declaration()->As<ast::Var>();
    auto* storage_ty = v->Type()->UnwrapRef();

    if (!IsStorable(storage_ty)) {
        AddError(sem_.TypeNameOf(storage_ty) + " cannot be used as the type of a var", var->source);
        return false;
    }

    if (storage_ty->is_handle() && var->declared_address_space != ast::AddressSpace::kNone) {
        // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
        // If the store type is a texture type or a sampler type, then the variable declaration must
        // not have a address space attribute. The address space will always be handle.
        AddError(
            "variables of type '" + sem_.TypeNameOf(storage_ty) + "' must not have a address space",
            var->source);
        return false;
    }

    if (IsValidationEnabled(var->attributes, ast::DisabledValidation::kIgnoreAddressSpace) &&
        (var->declared_address_space == ast::AddressSpace::kIn ||
         var->declared_address_space == ast::AddressSpace::kOut)) {
        AddError("invalid use of input/output address space", var->source);
        return false;
    }
    return true;
}

bool Validator::Let(const sem::Variable* v) const {
    auto* decl = v->Declaration();
    auto* storage_ty = v->Type()->UnwrapRef();

    if (!(storage_ty->IsConstructible() || storage_ty->Is<sem::Pointer>())) {
        AddError(sem_.TypeNameOf(storage_ty) + " cannot be used as the type of a 'let'",
                 decl->source);
        return false;
    }
    return true;
}

bool Validator::Override(
    const sem::GlobalVariable* v,
    const utils::Hashmap<OverrideId, const sem::Variable*, 8>& override_ids) const {
    auto* decl = v->Declaration();
    auto* storage_ty = v->Type()->UnwrapRef();

    if (auto* init = v->Initializer(); init && init->Stage() > sem::EvaluationStage::kOverride) {
        AddError("'override' initializer must be an override-expression",
                 init->Declaration()->source);
        return false;
    }

    for (auto* attr : decl->attributes) {
        if (attr->Is<ast::IdAttribute>()) {
            auto id = v->OverrideId();
            if (auto* var = override_ids.Find(id); var && *var != v) {
                AddError("@id values must be unique", attr->source);
                AddNote(
                    "a override with an ID of " + std::to_string(id.value) +
                        " was previously declared here:",
                    ast::GetAttribute<ast::IdAttribute>((*var)->Declaration()->attributes)->source);
                return false;
            }
        } else {
            AddError("attribute is not valid for 'override' declaration", attr->source);
            return false;
        }
    }

    if (!storage_ty->is_scalar()) {
        AddError(sem_.TypeNameOf(storage_ty) + " cannot be used as the type of a 'override'",
                 decl->source);
        return false;
    }

    if (storage_ty->Is<sem::F16>()) {
        AddError("'override' of type f16 is not implemented yet", decl->source);
        return false;
    }

    return true;
}

bool Validator::Const(const sem::Variable*) const {
    return true;
}

bool Validator::Parameter(const ast::Function* func, const sem::Variable* var) const {
    auto* decl = var->Declaration();

    if (IsValidationDisabled(decl->attributes, ast::DisabledValidation::kFunctionParameter)) {
        return true;
    }

    for (auto* attr : decl->attributes) {
        if (!func->IsEntryPoint() && !attr->Is<ast::InternalAttribute>()) {
            AddError("attribute is not valid for non-entry point function parameters",
                     attr->source);
            return false;
        }
        if (!attr->IsAnyOf<ast::BuiltinAttribute, ast::InvariantAttribute, ast::LocationAttribute,
                           ast::InterpolateAttribute, ast::InternalAttribute>() &&
            (IsValidationEnabled(decl->attributes,
                                 ast::DisabledValidation::kEntryPointParameter))) {
            AddError("attribute is not valid for function parameters", attr->source);
            return false;
        }
    }

    if (auto* ref = var->Type()->As<sem::Pointer>()) {
        auto address_space = ref->AddressSpace();
        if (!(address_space == ast::AddressSpace::kFunction ||
              address_space == ast::AddressSpace::kPrivate) &&
            IsValidationEnabled(decl->attributes, ast::DisabledValidation::kIgnoreAddressSpace)) {
            std::stringstream ss;
            ss << "function parameter of pointer type cannot be in '" << address_space
               << "' address space";
            AddError(ss.str(), decl->source);
            return false;
        }
    }

    if (IsPlain(var->Type())) {
        if (!var->Type()->IsConstructible()) {
            AddError("type of function parameter must be constructible", decl->type->source);
            return false;
        }
    } else if (!var->Type()->IsAnyOf<sem::Texture, sem::Sampler, sem::Pointer>()) {
        AddError("type of function parameter cannot be " + sem_.TypeNameOf(var->Type()),
                 decl->source);
        return false;
    }

    return true;
}

bool Validator::BuiltinAttribute(const ast::BuiltinAttribute* attr,
                                 const sem::Type* storage_ty,
                                 ast::PipelineStage stage,
                                 const bool is_input) const {
    auto* type = storage_ty->UnwrapRef();
    std::stringstream stage_name;
    stage_name << stage;
    bool is_stage_mismatch = false;
    bool is_output = !is_input;
    switch (attr->builtin) {
        case ast::BuiltinValue::kPosition:
            if (stage != ast::PipelineStage::kNone &&
                !((is_input && stage == ast::PipelineStage::kFragment) ||
                  (is_output && stage == ast::PipelineStage::kVertex))) {
                is_stage_mismatch = true;
            }
            if (!(type->is_float_vector() && type->As<sem::Vector>()->Width() == 4)) {
                AddError("store type of " + attr_to_str(attr) + " must be 'vec4<f32>'",
                         attr->source);
                return false;
            }
            break;
        case ast::BuiltinValue::kGlobalInvocationId:
        case ast::BuiltinValue::kLocalInvocationId:
        case ast::BuiltinValue::kNumWorkgroups:
        case ast::BuiltinValue::kWorkgroupId:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kCompute && is_input)) {
                is_stage_mismatch = true;
            }
            if (!(type->is_unsigned_integer_vector() && type->As<sem::Vector>()->Width() == 3)) {
                AddError("store type of " + attr_to_str(attr) + " must be 'vec3<u32>'",
                         attr->source);
                return false;
            }
            break;
        case ast::BuiltinValue::kFragDepth:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kFragment && !is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<sem::F32>()) {
                AddError("store type of " + attr_to_str(attr) + " must be 'f32'", attr->source);
                return false;
            }
            break;
        case ast::BuiltinValue::kFrontFacing:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kFragment && is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<sem::Bool>()) {
                AddError("store type of " + attr_to_str(attr) + " must be 'bool'", attr->source);
                return false;
            }
            break;
        case ast::BuiltinValue::kLocalInvocationIndex:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kCompute && is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<sem::U32>()) {
                AddError("store type of " + attr_to_str(attr) + " must be 'u32'", attr->source);
                return false;
            }
            break;
        case ast::BuiltinValue::kVertexIndex:
        case ast::BuiltinValue::kInstanceIndex:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kVertex && is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<sem::U32>()) {
                AddError("store type of " + attr_to_str(attr) + " must be 'u32'", attr->source);
                return false;
            }
            break;
        case ast::BuiltinValue::kSampleMask:
            if (stage != ast::PipelineStage::kNone && !(stage == ast::PipelineStage::kFragment)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<sem::U32>()) {
                AddError("store type of " + attr_to_str(attr) + " must be 'u32'", attr->source);
                return false;
            }
            break;
        case ast::BuiltinValue::kSampleIndex:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kFragment && is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<sem::U32>()) {
                AddError("store type of " + attr_to_str(attr) + " must be 'u32'", attr->source);
                return false;
            }
            break;
        default:
            break;
    }

    if (is_stage_mismatch) {
        AddError(attr_to_str(attr) + " cannot be used in " +
                     (is_input ? "input of " : "output of ") + stage_name.str() + " pipeline stage",
                 attr->source);
        return false;
    }

    return true;
}

bool Validator::InterpolateAttribute(const ast::InterpolateAttribute* attr,
                                     const sem::Type* storage_ty) const {
    auto* type = storage_ty->UnwrapRef();

    if (type->is_integer_scalar_or_vector() && attr->type != ast::InterpolationType::kFlat) {
        AddError("interpolation type must be 'flat' for integral user-defined IO types",
                 attr->source);
        return false;
    }

    if (attr->type == ast::InterpolationType::kFlat &&
        attr->sampling != ast::InterpolationSampling::kUndefined) {
        AddError("flat interpolation attribute must not have a sampling parameter", attr->source);
        return false;
    }

    return true;
}

bool Validator::Function(const sem::Function* func, ast::PipelineStage stage) const {
    auto* decl = func->Declaration();

    for (auto* attr : decl->attributes) {
        if (attr->Is<ast::WorkgroupAttribute>()) {
            if (decl->PipelineStage() != ast::PipelineStage::kCompute) {
                AddError("the workgroup_size attribute is only valid for compute stages",
                         attr->source);
                return false;
            }
        } else if (!attr->IsAnyOf<ast::StageAttribute, ast::InternalAttribute>()) {
            AddError("attribute is not valid for functions", attr->source);
            return false;
        }
    }

    if (decl->params.Length() > 255) {
        AddError("functions may declare at most 255 parameters", decl->source);
        return false;
    }

    if (!func->ReturnType()->Is<sem::Void>()) {
        if (!func->ReturnType()->IsConstructible()) {
            AddError("function return type must be a constructible type",
                     decl->return_type->source);
            return false;
        }

        if (decl->body) {
            sem::Behaviors behaviors{sem::Behavior::kNext};
            if (auto* last = decl->body->Last()) {
                behaviors = sem_.Get(last)->Behaviors();
            }
            if (behaviors.Contains(sem::Behavior::kNext)) {
                AddError("missing return at end of function", decl->source);
                return false;
            }
        } else if (IsValidationEnabled(decl->attributes,
                                       ast::DisabledValidation::kFunctionHasNoBody)) {
            TINT_ICE(Resolver, diagnostics_)
                << "Function " << symbols_.NameFor(decl->symbol) << " has no body";
        }

        for (auto* attr : decl->return_type_attributes) {
            if (!decl->IsEntryPoint()) {
                AddError("attribute is not valid for non-entry point function return types",
                         attr->source);
                return false;
            }
            if (!attr->IsAnyOf<ast::BuiltinAttribute, ast::InternalAttribute,
                               ast::LocationAttribute, ast::InterpolateAttribute,
                               ast::InvariantAttribute>() &&
                (IsValidationEnabled(decl->attributes,
                                     ast::DisabledValidation::kEntryPointParameter) &&
                 IsValidationEnabled(decl->attributes,
                                     ast::DisabledValidation::kFunctionParameter))) {
                AddError("attribute is not valid for entry point return types", attr->source);
                return false;
            }
        }
    }

    if (decl->IsEntryPoint()) {
        if (!EntryPoint(func, stage)) {
            return false;
        }
    }

    // https://www.w3.org/TR/WGSL/#behaviors-rules
    // a function behavior is always one of {}, or {Next}.
    if (func->Behaviors() != sem::Behaviors{} && func->Behaviors() != sem::Behavior::kNext) {
        auto name = symbols_.NameFor(decl->symbol);
        TINT_ICE(Resolver, diagnostics_)
            << "function '" << name << "' behaviors are: " << func->Behaviors();
    }

    return true;
}

bool Validator::EntryPoint(const sem::Function* func, ast::PipelineStage stage) const {
    auto* decl = func->Declaration();

    // Use a lambda to validate the entry point attributes for a type.
    // Persistent state is used to track which builtins and locations have already been seen, in
    // order to catch conflicts.
    // TODO(jrprice): This state could be stored in sem::Function instead, and then passed to
    // sem::Function since it would be useful there too.
    utils::Hashset<ast::BuiltinValue, 4> builtins;
    utils::Hashset<uint32_t, 8> locations;
    enum class ParamOrRetType {
        kParameter,
        kReturnType,
    };

    // Inner lambda that is applied to a type and all of its members.
    auto validate_entry_point_attributes_inner = [&](utils::VectorRef<const ast::Attribute*> attrs,
                                                     const sem::Type* ty, Source source,
                                                     ParamOrRetType param_or_ret,
                                                     bool is_struct_member,
                                                     std::optional<uint32_t> location) {
        // Temporally forbid using f16 types in entry point IO.
        // TODO(tint:1473, tint:1502): Remove this error after f16 is supported in entry point IO.
        if (Is<sem::F16>(sem::Type::DeepestElementOf(ty))) {
            AddError("entry point IO of f16 types is not implemented yet", source);
            return false;
        }

        // Scan attributes for pipeline IO attributes.
        // Check for overlap with attributes that have been seen previously.
        const ast::Attribute* pipeline_io_attribute = nullptr;
        const ast::InterpolateAttribute* interpolate_attribute = nullptr;
        const ast::InvariantAttribute* invariant_attribute = nullptr;
        for (auto* attr : attrs) {
            auto is_invalid_compute_shader_attribute = false;

            if (auto* builtin = attr->As<ast::BuiltinAttribute>()) {
                if (pipeline_io_attribute) {
                    AddError("multiple entry point IO attributes", attr->source);
                    AddNote("previously consumed " + attr_to_str(pipeline_io_attribute, location),
                            pipeline_io_attribute->source);
                    return false;
                }
                pipeline_io_attribute = attr;

                if (builtins.Contains(builtin->builtin)) {
                    AddError(attr_to_str(builtin) +
                                 " attribute appears multiple times as pipeline " +
                                 (param_or_ret == ParamOrRetType::kParameter ? "input" : "output"),
                             decl->source);
                    return false;
                }

                if (!BuiltinAttribute(builtin, ty, stage,
                                      /* is_input */ param_or_ret == ParamOrRetType::kParameter)) {
                    return false;
                }
                builtins.Add(builtin->builtin);
            } else if (auto* loc_attr = attr->As<ast::LocationAttribute>()) {
                if (pipeline_io_attribute) {
                    AddError("multiple entry point IO attributes", attr->source);
                    AddNote("previously consumed " + attr_to_str(pipeline_io_attribute),
                            pipeline_io_attribute->source);
                    return false;
                }
                pipeline_io_attribute = attr;

                bool is_input = param_or_ret == ParamOrRetType::kParameter;

                if (!location.has_value()) {
                    TINT_ICE(Resolver, diagnostics_) << "Location has no value";
                    return false;
                }

                if (!LocationAttribute(loc_attr, location.value(), ty, locations, stage, source,
                                       is_input)) {
                    return false;
                }
            } else if (auto* interpolate = attr->As<ast::InterpolateAttribute>()) {
                if (decl->PipelineStage() == ast::PipelineStage::kCompute) {
                    is_invalid_compute_shader_attribute = true;
                } else if (!InterpolateAttribute(interpolate, ty)) {
                    return false;
                }
                interpolate_attribute = interpolate;
            } else if (auto* invariant = attr->As<ast::InvariantAttribute>()) {
                if (decl->PipelineStage() == ast::PipelineStage::kCompute) {
                    is_invalid_compute_shader_attribute = true;
                }
                invariant_attribute = invariant;
            }
            if (is_invalid_compute_shader_attribute) {
                std::string input_or_output =
                    param_or_ret == ParamOrRetType::kParameter ? "inputs" : "output";
                AddError("attribute is not valid for compute shader " + input_or_output,
                         attr->source);
                return false;
            }
        }

        if (IsValidationEnabled(attrs, ast::DisabledValidation::kEntryPointParameter)) {
            if (is_struct_member && ty->Is<sem::Struct>()) {
                AddError("nested structures cannot be used for entry point IO", source);
                return false;
            }

            if (!ty->Is<sem::Struct>() && !pipeline_io_attribute) {
                std::string err = "missing entry point IO attribute";
                if (!is_struct_member) {
                    err += (param_or_ret == ParamOrRetType::kParameter ? " on parameter"
                                                                       : " on return type");
                }
                AddError(err, source);
                return false;
            }

            if (pipeline_io_attribute && pipeline_io_attribute->Is<ast::LocationAttribute>()) {
                if (ty->is_integer_scalar_or_vector() && !interpolate_attribute) {
                    if (decl->PipelineStage() == ast::PipelineStage::kVertex &&
                        param_or_ret == ParamOrRetType::kReturnType) {
                        AddError(
                            "integral user-defined vertex outputs must have a flat "
                            "interpolation attribute",
                            source);
                        return false;
                    }
                    if (decl->PipelineStage() == ast::PipelineStage::kFragment &&
                        param_or_ret == ParamOrRetType::kParameter) {
                        AddError(
                            "integral user-defined fragment inputs must have a flat "
                            "interpolation attribute",
                            source);
                        return false;
                    }
                }
            }

            if (interpolate_attribute) {
                if (!pipeline_io_attribute ||
                    !pipeline_io_attribute->Is<ast::LocationAttribute>()) {
                    AddError("interpolate attribute must only be used with @location",
                             interpolate_attribute->source);
                    return false;
                }
            }

            if (invariant_attribute) {
                bool has_position = false;
                if (pipeline_io_attribute) {
                    if (auto* builtin = pipeline_io_attribute->As<ast::BuiltinAttribute>()) {
                        has_position = (builtin->builtin == ast::BuiltinValue::kPosition);
                    }
                }
                if (!has_position) {
                    AddError(
                        "invariant attribute must only be applied to a position "
                        "builtin",
                        invariant_attribute->source);
                    return false;
                }
            }
        }
        return true;
    };

    // Outer lambda for validating the entry point attributes for a type.
    auto validate_entry_point_attributes = [&](utils::VectorRef<const ast::Attribute*> attrs,
                                               const sem::Type* ty, Source source,
                                               ParamOrRetType param_or_ret,
                                               std::optional<uint32_t> location) {
        if (!validate_entry_point_attributes_inner(attrs, ty, source, param_or_ret,
                                                   /*is_struct_member*/ false, location)) {
            return false;
        }

        if (auto* str = ty->As<sem::Struct>()) {
            for (auto* member : str->Members()) {
                if (!validate_entry_point_attributes_inner(
                        member->Declaration()->attributes, member->Type(),
                        member->Declaration()->source, param_or_ret,
                        /*is_struct_member*/ true, member->Location())) {
                    AddNote("while analyzing entry point '" + symbols_.NameFor(decl->symbol) + "'",
                            decl->source);
                    return false;
                }
            }
        }

        return true;
    };

    for (auto* param : func->Parameters()) {
        auto* param_decl = param->Declaration();
        if (!validate_entry_point_attributes(param_decl->attributes, param->Type(),
                                             param_decl->source, ParamOrRetType::kParameter,
                                             param->Location())) {
            return false;
        }
    }

    // Clear IO sets after parameter validation. Builtin and location attributes in return types
    // should be validated independently from those used in parameters.
    builtins.Clear();
    locations.Clear();

    if (!func->ReturnType()->Is<sem::Void>()) {
        if (!validate_entry_point_attributes(decl->return_type_attributes, func->ReturnType(),
                                             decl->source, ParamOrRetType::kReturnType,
                                             func->ReturnLocation())) {
            return false;
        }
    }

    if (decl->PipelineStage() == ast::PipelineStage::kVertex &&
        !builtins.Contains(ast::BuiltinValue::kPosition)) {
        // Check module-scope variables, as the SPIR-V sanitizer generates these.
        bool found = false;
        for (auto* global : func->TransitivelyReferencedGlobals()) {
            if (auto* builtin =
                    ast::GetAttribute<ast::BuiltinAttribute>(global->Declaration()->attributes)) {
                if (builtin->builtin == ast::BuiltinValue::kPosition) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            AddError("a vertex shader must include the 'position' builtin in its return type",
                     decl->source);
            return false;
        }
    }

    if (decl->PipelineStage() == ast::PipelineStage::kCompute) {
        if (!ast::HasAttribute<ast::WorkgroupAttribute>(decl->attributes)) {
            AddError("a compute shader must include 'workgroup_size' in its attributes",
                     decl->source);
            return false;
        }
    }

    // Validate there are no resource variable binding collisions
    utils::Hashmap<sem::BindingPoint, const ast::Variable*, 8> binding_points;
    for (auto* global : func->TransitivelyReferencedGlobals()) {
        auto* var_decl = global->Declaration()->As<ast::Var>();
        if (!var_decl || !var_decl->HasBindingPoint()) {
            continue;
        }
        auto bp = global->BindingPoint();
        if (auto added = binding_points.Add(bp, var_decl);
            !added &&
            IsValidationEnabled(decl->attributes,
                                ast::DisabledValidation::kBindingPointCollision) &&
            IsValidationEnabled((*added.value)->attributes,
                                ast::DisabledValidation::kBindingPointCollision)) {
            // https://gpuweb.github.io/gpuweb/wgsl/#resource-interface
            // Bindings must not alias within a shader stage: two different variables in the
            // resource interface of a given shader must not have the same group and binding values,
            // when considered as a pair of values.
            auto func_name = symbols_.NameFor(decl->symbol);
            AddError(
                "entry point '" + func_name +
                    "' references multiple variables that use the same resource binding @group(" +
                    std::to_string(bp.group) + "), @binding(" + std::to_string(bp.binding) + ")",
                var_decl->source);
            AddNote("first resource binding usage declared here", (*added.value)->source);
            return false;
        }
    }

    return true;
}

bool Validator::EvaluationStage(const sem::Expression* expr,
                                sem::EvaluationStage latest_stage,
                                std::string_view constraint) const {
    if (expr->Stage() > latest_stage) {
        auto stage_name = [](sem::EvaluationStage stage) -> std::string {
            switch (stage) {
                case sem::EvaluationStage::kRuntime:
                    return "a runtime-expression";
                case sem::EvaluationStage::kOverride:
                    return "an override-expression";
                case sem::EvaluationStage::kConstant:
                    return "a const-expression";
            }
            return "<unknown>";
        };

        AddError(std::string(constraint) + " requires " + stage_name(latest_stage) +
                     ", but expression is " + stage_name(expr->Stage()),
                 expr->Declaration()->source);

        if (auto* stmt = expr->Stmt()) {
            if (auto* decl = As<ast::VariableDeclStatement>(stmt->Declaration())) {
                if (decl->variable->Is<ast::Const>()) {
                    AddNote("consider changing 'const' to 'let'", decl->source);
                }
            }
        }
        return false;
    }
    return true;
}

bool Validator::Statements(utils::VectorRef<const ast::Statement*> stmts) const {
    for (auto* stmt : stmts) {
        if (!sem_.Get(stmt)->IsReachable()) {
            /// TODO(https://github.com/gpuweb/gpuweb/issues/2378): This may need to
            /// become an error.
            AddWarning("code is unreachable", stmt->source);
            break;
        }
    }
    return true;
}

bool Validator::Bitcast(const ast::BitcastExpression* cast, const sem::Type* to) const {
    auto* from = sem_.TypeOf(cast->expr)->UnwrapRef();
    if (!from->is_numeric_scalar_or_vector()) {
        AddError("'" + sem_.TypeNameOf(from) + "' cannot be bitcast", cast->expr->source);
        return false;
    }
    if (!to->is_numeric_scalar_or_vector()) {
        AddError("cannot bitcast to '" + sem_.TypeNameOf(to) + "'", cast->type->source);
        return false;
    }

    auto width = [&](const sem::Type* ty) {
        if (auto* vec = ty->As<sem::Vector>()) {
            return vec->Width();
        }
        return 1u;
    };

    if (width(from) != width(to)) {
        AddError(
            "cannot bitcast from '" + sem_.TypeNameOf(from) + "' to '" + sem_.TypeNameOf(to) + "'",
            cast->source);
        return false;
    }

    return true;
}

bool Validator::BreakStatement(const sem::Statement* stmt,
                               sem::Statement* current_statement) const {
    if (!stmt->FindFirstParent<sem::LoopBlockStatement, sem::CaseStatement>()) {
        AddError("break statement must be in a loop or switch case", stmt->Declaration()->source);
        return false;
    }
    if (auto* continuing = ClosestContinuing(/*stop_at_loop*/ true, current_statement)) {
        AddWarning(
            "use of deprecated language feature: `break` must not be used to exit from "
            "a continuing block. Use `break-if` instead.",
            stmt->Declaration()->source);

        auto fail = [&](const char* note_msg, const Source& note_src) {
            constexpr const char* kErrorMsg =
                "break statement in a continuing block must be the single statement of an if "
                "statement's true or false block, and that if statement must be the last statement "
                "of the continuing block";
            AddError(kErrorMsg, stmt->Declaration()->source);
            AddNote(note_msg, note_src);
            return false;
        };

        if (auto* block = stmt->Parent()->As<sem::BlockStatement>()) {
            auto* block_parent = block->Parent();
            auto* if_stmt = block_parent->As<sem::IfStatement>();
            if (!if_stmt) {
                return fail("break statement is not directly in if statement block",
                            stmt->Declaration()->source);
            }
            if (block->Declaration()->statements.Length() != 1) {
                return fail("if statement block contains multiple statements",
                            block->Declaration()->source);
            }

            if (if_stmt->Parent()->Is<sem::IfStatement>()) {
                return fail("else has condition", if_stmt->Declaration()->source);
            }

            bool el_contains_break = block->Declaration() == if_stmt->Declaration()->else_statement;
            if (el_contains_break) {
                if (auto* true_block = if_stmt->Declaration()->body; !true_block->Empty()) {
                    return fail("non-empty true block", true_block->source);
                }
            } else {
                auto* else_stmt = if_stmt->Declaration()->else_statement;
                if (else_stmt) {
                    return fail("non-empty false block", else_stmt->source);
                }
            }

            if (if_stmt->Parent()->Declaration() != continuing) {
                return fail(
                    "if statement containing break statement is not directly in continuing block",
                    if_stmt->Declaration()->source);
            }
            if (auto* cont_block = continuing->As<ast::BlockStatement>()) {
                if (if_stmt->Declaration() != cont_block->Last()) {
                    return fail(
                        "if statement containing break statement is not the last statement of the "
                        "continuing block",
                        if_stmt->Declaration()->source);
                }
            }
        }
    }
    return true;
}

bool Validator::ContinueStatement(const sem::Statement* stmt,
                                  sem::Statement* current_statement) const {
    if (auto* continuing = ClosestContinuing(/*stop_at_loop*/ true, current_statement)) {
        AddError("continuing blocks must not contain a continue statement",
                 stmt->Declaration()->source);
        if (continuing != stmt->Declaration() && continuing != stmt->Parent()->Declaration()) {
            AddNote("see continuing block here", continuing->source);
        }
        return false;
    }

    if (!stmt->FindFirstParent<sem::LoopBlockStatement>()) {
        AddError("continue statement must be in a loop", stmt->Declaration()->source);
        return false;
    }

    return true;
}

bool Validator::Call(const sem::Call* call, sem::Statement* current_statement) const {
    auto* expr = call->Declaration();
    bool is_call_stmt =
        current_statement && Is<ast::CallStatement>(current_statement->Declaration(),
                                                    [&](auto* stmt) { return stmt->expr == expr; });
    if (is_call_stmt) {
        return Switch(
            call->Target(),  //
            [&](const sem::TypeConversion*) {
                AddError("type conversion evaluated but not used", call->Declaration()->source);
                return false;
            },
            [&](const sem::TypeInitializer*) {
                AddError("type initializer evaluated but not used", call->Declaration()->source);
                return false;
            },
            [&](Default) { return true; });
    }

    return true;
}

bool Validator::LoopStatement(const sem::LoopStatement* stmt) const {
    if (stmt->Behaviors().Empty()) {
        AddError("loop does not exit", stmt->Declaration()->source.Begin());
        return false;
    }
    return true;
}

bool Validator::ForLoopStatement(const sem::ForLoopStatement* stmt) const {
    if (stmt->Behaviors().Empty()) {
        AddError("for-loop does not exit", stmt->Declaration()->source.Begin());
        return false;
    }
    if (auto* cond = stmt->Condition()) {
        auto* cond_ty = cond->Type()->UnwrapRef();
        if (!cond_ty->Is<sem::Bool>()) {
            AddError("for-loop condition must be bool, got " + sem_.TypeNameOf(cond_ty),
                     stmt->Condition()->Declaration()->source);
            return false;
        }
    }
    return true;
}

bool Validator::WhileStatement(const sem::WhileStatement* stmt) const {
    if (stmt->Behaviors().Empty()) {
        AddError("while does not exit", stmt->Declaration()->source.Begin());
        return false;
    }
    if (auto* cond = stmt->Condition()) {
        auto* cond_ty = cond->Type()->UnwrapRef();
        if (!cond_ty->Is<sem::Bool>()) {
            AddError("while condition must be bool, got " + sem_.TypeNameOf(cond_ty),
                     stmt->Condition()->Declaration()->source);
            return false;
        }
    }
    return true;
}

bool Validator::BreakIfStatement(const sem::BreakIfStatement* stmt,
                                 sem::Statement* current_statement) const {
    auto* cond_ty = stmt->Condition()->Type()->UnwrapRef();
    if (!cond_ty->Is<sem::Bool>()) {
        AddError("break-if statement condition must be bool, got " + sem_.TypeNameOf(cond_ty),
                 stmt->Condition()->Declaration()->source);
        return false;
    }

    for (const auto* s = current_statement; s != nullptr; s = s->Parent()) {
        if (s->Is<sem::LoopStatement>()) {
            break;
        }
        if (auto* continuing = s->As<sem::LoopContinuingBlockStatement>()) {
            if (continuing->Declaration()->statements.Back() != stmt->Declaration()) {
                AddError("break-if must be last statement in a continuing block",
                         stmt->Declaration()->source);
                AddNote("see continuing block here", s->Declaration()->source);
                return false;
            }
            return true;
        }
    }

    AddError("break-if must in a continuing block", stmt->Declaration()->source);
    return false;
}

bool Validator::IfStatement(const sem::IfStatement* stmt) const {
    auto* cond_ty = stmt->Condition()->Type()->UnwrapRef();
    if (!cond_ty->Is<sem::Bool>()) {
        AddError("if statement condition must be bool, got " + sem_.TypeNameOf(cond_ty),
                 stmt->Condition()->Declaration()->source);
        return false;
    }
    return true;
}

bool Validator::BuiltinCall(const sem::Call* call) const {
    if (call->Type()->Is<sem::Void>()) {
        bool is_call_statement = false;
        // Some built-in call are not owned by a statement, e.g. a built-in called in global
        // variable declaration. Calling no-return-value built-in in these context is invalid as
        // well.
        if (auto* call_stmt = call->Stmt()) {
            if (auto* call_stmt_ast = As<ast::CallStatement>(call_stmt->Declaration())) {
                if (call_stmt_ast->expr == call->Declaration()) {
                    is_call_statement = true;
                }
            }
        }
        if (!is_call_statement) {
            // https://gpuweb.github.io/gpuweb/wgsl/#function-call-expr
            // If the called function does not return a value, a function call
            // statement should be used instead.
            auto* ident = call->Declaration()->target.name;
            auto name = symbols_.NameFor(ident->symbol);
            AddError("builtin '" + name + "' does not return a value", call->Declaration()->source);
            return false;
        }
    }

    return true;
}

bool Validator::TextureBuiltinFunction(const sem::Call* call) const {
    auto* builtin = call->Target()->As<sem::Builtin>();
    if (!builtin) {
        return false;
    }

    std::string func_name = builtin->str();
    auto& signature = builtin->Signature();

    auto check_arg_is_constexpr = [&](sem::ParameterUsage usage, int min, int max) {
        auto signed_index = signature.IndexOf(usage);
        if (signed_index < 0) {
            return true;
        }
        auto index = static_cast<size_t>(signed_index);
        std::string name = sem::str(usage);
        auto* arg = call->Arguments()[index];
        if (auto values = arg->ConstantValue()) {
            if (auto* vector = values->Type()->As<sem::Vector>()) {
                for (size_t i = 0; i < vector->Width(); i++) {
                    auto value = values->Index(i)->As<AInt>();
                    if (value < min || value > max) {
                        AddError("each component of the " + name + " argument must be at least " +
                                     std::to_string(min) + " and at most " + std::to_string(max) +
                                     ". " + name + " component " + std::to_string(i) + " is " +
                                     std::to_string(value),
                                 arg->Declaration()->source);
                        return false;
                    }
                }
            } else {
                auto value = values->As<AInt>();
                if (value < min || value > max) {
                    AddError("the " + name + " argument must be at least " + std::to_string(min) +
                                 " and at most " + std::to_string(max) + ". " + name + " is " +
                                 std::to_string(value),
                             arg->Declaration()->source);
                    return false;
                }
            }
            return true;
        }
        AddError("the " + name + " argument must be a const-expression",
                 arg->Declaration()->source);
        return false;
    };

    return check_arg_is_constexpr(sem::ParameterUsage::kOffset, -8, 7) &&
           check_arg_is_constexpr(sem::ParameterUsage::kComponent, 0, 3);
}

bool Validator::RequiredExtensionForBuiltinFunction(
    const sem::Call* call,
    const ast::Extensions& enabled_extensions) const {
    const auto* builtin = call->Target()->As<sem::Builtin>();
    if (!builtin) {
        return true;
    }

    const auto extension = builtin->RequiredExtension();
    if (extension == ast::Extension::kUndefined) {
        return true;
    }

    if (!enabled_extensions.Contains(extension)) {
        AddError("cannot call built-in function '" + std::string(builtin->str()) +
                     "' without extension " + utils::ToString(extension),
                 call->Declaration()->source);
        return false;
    }

    return true;
}

bool Validator::FunctionCall(const sem::Call* call, sem::Statement* current_statement) const {
    auto* decl = call->Declaration();
    auto* target = call->Target()->As<sem::Function>();
    auto sym = decl->target.name->symbol;
    auto name = symbols_.NameFor(sym);

    if (!current_statement) {  // Function call at module-scope.
        AddError("functions cannot be called at module-scope", decl->source);
        return false;
    }

    if (target->Declaration()->IsEntryPoint()) {
        // https://www.w3.org/TR/WGSL/#function-restriction
        // An entry point must never be the target of a function call.
        AddError("entry point functions cannot be the target of a function call", decl->source);
        return false;
    }

    if (decl->args.Length() != target->Parameters().Length()) {
        bool more = decl->args.Length() > target->Parameters().Length();
        AddError("too " + (more ? std::string("many") : std::string("few")) +
                     " arguments in call to '" + name + "', expected " +
                     std::to_string(target->Parameters().Length()) + ", got " +
                     std::to_string(call->Arguments().Length()),
                 decl->source);
        return false;
    }

    for (size_t i = 0; i < call->Arguments().Length(); ++i) {
        const sem::Variable* param = target->Parameters()[i];
        const ast::Expression* arg_expr = decl->args[i];
        auto* param_type = param->Type();
        auto* arg_type = sem_.TypeOf(arg_expr)->UnwrapRef();

        if (param_type != arg_type) {
            AddError("type mismatch for argument " + std::to_string(i + 1) + " in call to '" +
                         name + "', expected '" + sem_.TypeNameOf(param_type) + "', got '" +
                         sem_.TypeNameOf(arg_type) + "'",
                     arg_expr->source);
            return false;
        }

        if (param_type->Is<sem::Pointer>()) {
            // https://gpuweb.github.io/gpuweb/wgsl/#function-restriction
            // Each argument of pointer type to a user-defined function must have the same memory
            // view as its root identifier.
            // We can validate this by just comparing the store type of the argument with that of
            // its root identifier, as these will match iff the memory view is the same.
            auto* arg_store_type = arg_type->As<sem::Pointer>()->StoreType();
            auto* root = call->Arguments()[i]->RootIdentifier();
            auto* root_ptr_ty = root->Type()->As<sem::Pointer>();
            auto* root_ref_ty = root->Type()->As<sem::Reference>();
            TINT_ASSERT(Resolver, root_ptr_ty || root_ref_ty);
            const sem::Type* root_store_type;
            if (root_ptr_ty) {
                root_store_type = root_ptr_ty->StoreType();
            } else {
                root_store_type = root_ref_ty->StoreType();
            }
            if (root_store_type != arg_store_type &&
                IsValidationEnabled(param->Declaration()->attributes,
                                    ast::DisabledValidation::kIgnoreInvalidPointerArgument)) {
                AddError(
                    "arguments of pointer type must not point to a subset of the originating "
                    "variable",
                    arg_expr->source);
                return false;
            }
        }
    }

    if (call->Type()->Is<sem::Void>()) {
        bool is_call_statement = false;
        if (auto* call_stmt = As<ast::CallStatement>(call->Stmt()->Declaration())) {
            if (call_stmt->expr == call->Declaration()) {
                is_call_statement = true;
            }
        }
        if (!is_call_statement) {
            // https://gpuweb.github.io/gpuweb/wgsl/#function-call-expr
            // If the called function does not return a value, a function call
            // statement should be used instead.
            AddError("function '" + name + "' does not return a value", decl->source);
            return false;
        }
    }

    return true;
}

bool Validator::StructureInitializer(const ast::CallExpression* ctor,
                                     const sem::Struct* struct_type) const {
    if (!struct_type->IsConstructible()) {
        AddError("struct initializer has non-constructible type", ctor->source);
        return false;
    }

    if (ctor->args.Length() > 0) {
        if (ctor->args.Length() != struct_type->Members().size()) {
            std::string fm = ctor->args.Length() < struct_type->Members().size() ? "few" : "many";
            AddError("struct initializer has too " + fm + " inputs: expected " +
                         std::to_string(struct_type->Members().size()) + ", found " +
                         std::to_string(ctor->args.Length()),
                     ctor->source);
            return false;
        }
        for (auto* member : struct_type->Members()) {
            auto* value = ctor->args[member->Index()];
            auto* value_ty = sem_.TypeOf(value);
            if (member->Type() != value_ty->UnwrapRef()) {
                AddError(
                    "type in struct initializer does not match struct member type: expected '" +
                        sem_.TypeNameOf(member->Type()) + "', found '" + sem_.TypeNameOf(value_ty) +
                        "'",
                    value->source);
                return false;
            }
        }
    }
    return true;
}

bool Validator::ArrayInitializer(const ast::CallExpression* ctor,
                                 const sem::Array* array_type) const {
    auto& values = ctor->args;
    auto* elem_ty = array_type->ElemType();
    for (auto* value : values) {
        auto* value_ty = sem_.TypeOf(value)->UnwrapRef();
        if (sem::Type::ConversionRank(value_ty, elem_ty) == sem::Type::kNoConversion) {
            AddError("'" + sem_.TypeNameOf(value_ty) +
                         "' cannot be used to construct an array of '" + sem_.TypeNameOf(elem_ty) +
                         "'",
                     value->source);
            return false;
        }
    }

    if (array_type->IsRuntimeSized()) {
        AddError("cannot construct a runtime-sized array", ctor->source);
        return false;
    }

    if (array_type->IsOverrideSized()) {
        AddError("cannot construct an array that has an override-expression count", ctor->source);
        return false;
    }

    if (!elem_ty->IsConstructible()) {
        AddError("array initializer has non-constructible element type", ctor->source);
        return false;
    }

    const auto count = std::get<sem::ConstantArrayCount>(array_type->Count()).value;
    if (!values.IsEmpty() && (values.Length() != count)) {
        std::string fm = values.Length() < count ? "few" : "many";
        AddError("array initializer has too " + fm + " elements: expected " +
                     std::to_string(count) + ", found " + std::to_string(values.Length()),
                 ctor->source);
        return false;
    }
    return true;
}

bool Validator::Vector(const sem::Vector* ty, const Source& source) const {
    if (!ty->type()->is_scalar()) {
        AddError("vector element type must be 'bool', 'f32', 'f16', 'i32' or 'u32'", source);
        return false;
    }
    return true;
}

bool Validator::Matrix(const sem::Matrix* ty, const Source& source) const {
    if (!ty->is_float_matrix()) {
        AddError("matrix element type must be 'f32' or 'f16'", source);
        return false;
    }
    return true;
}

bool Validator::PipelineStages(const utils::VectorRef<sem::Function*> entry_points) const {
    auto backtrace = [&](const sem::Function* func, const sem::Function* entry_point) {
        if (func != entry_point) {
            TraverseCallChain(diagnostics_, entry_point, func, [&](const sem::Function* f) {
                AddNote("called by function '" + symbols_.NameFor(f->Declaration()->symbol) + "'",
                        f->Declaration()->source);
            });
            AddNote("called by entry point '" +
                        symbols_.NameFor(entry_point->Declaration()->symbol) + "'",
                    entry_point->Declaration()->source);
        }
    };

    auto check_workgroup_storage = [&](const sem::Function* func,
                                       const sem::Function* entry_point) {
        auto stage = entry_point->Declaration()->PipelineStage();
        if (stage != ast::PipelineStage::kCompute) {
            for (auto* var : func->DirectlyReferencedGlobals()) {
                if (var->AddressSpace() == ast::AddressSpace::kWorkgroup) {
                    std::stringstream stage_name;
                    stage_name << stage;
                    for (auto* user : var->Users()) {
                        if (func == user->Stmt()->Function()) {
                            AddError("workgroup memory cannot be used by " + stage_name.str() +
                                         " pipeline stage",
                                     user->Declaration()->source);
                            break;
                        }
                    }
                    AddNote("variable is declared here", var->Declaration()->source);
                    backtrace(func, entry_point);
                    return false;
                }
            }
        }
        return true;
    };

    auto check_builtin_calls = [&](const sem::Function* func, const sem::Function* entry_point) {
        auto stage = entry_point->Declaration()->PipelineStage();
        for (auto* builtin : func->DirectlyCalledBuiltins()) {
            if (!builtin->SupportedStages().Contains(stage)) {
                auto* call = func->FindDirectCallTo(builtin);
                std::stringstream err;
                err << "built-in cannot be used by " << stage << " pipeline stage";
                AddError(err.str(),
                         call ? call->Declaration()->source : func->Declaration()->source);
                backtrace(func, entry_point);
                return false;
            }
        }
        return true;
    };

    auto check_no_discards = [&](const sem::Function* func, const sem::Function* entry_point) {
        if (auto* discard = func->DiscardStatement()) {
            auto stage = entry_point->Declaration()->PipelineStage();
            std::stringstream err;
            err << "discard statement cannot be used in " << stage << " pipeline stage";
            AddError(err.str(), discard->Declaration()->source);
            backtrace(func, entry_point);
            return false;
        }
        return true;
    };

    auto check_func = [&](const sem::Function* func, const sem::Function* entry_point) {
        if (!check_workgroup_storage(func, entry_point)) {
            return false;
        }
        if (!check_builtin_calls(func, entry_point)) {
            return false;
        }
        if (entry_point->Declaration()->PipelineStage() != ast::PipelineStage::kFragment) {
            if (!check_no_discards(func, entry_point)) {
                return false;
            }
        }
        return true;
    };

    for (auto* entry_point : entry_points) {
        if (!check_func(entry_point, entry_point)) {
            return false;
        }
        for (auto* func : entry_point->TransitivelyCalledFunctions()) {
            if (!check_func(func, entry_point)) {
                return false;
            }
        }
    }

    return true;
}

bool Validator::PushConstants(const utils::VectorRef<sem::Function*> entry_points) const {
    for (auto* entry_point : entry_points) {
        // State checked and modified by check_push_constant so that it remembers previously seen
        // push_constant variables for an entry-point.
        const sem::Variable* push_constant_var = nullptr;
        const sem::Function* push_constant_func = nullptr;

        auto check_push_constant = [&](const sem::Function* func, const sem::Function* ep) {
            for (auto* var : func->DirectlyReferencedGlobals()) {
                if (var->AddressSpace() != ast::AddressSpace::kPushConstant ||
                    var == push_constant_var) {
                    continue;
                }

                if (push_constant_var == nullptr) {
                    push_constant_var = var;
                    push_constant_func = func;
                    continue;
                }

                AddError("entry point '" + symbols_.NameFor(ep->Declaration()->symbol) +
                             "' uses two different 'push_constant' variables.",
                         ep->Declaration()->source);
                AddNote("first 'push_constant' variable declaration is here",
                        var->Declaration()->source);
                if (func != ep) {
                    TraverseCallChain(diagnostics_, ep, func, [&](const sem::Function* f) {
                        AddNote("called by function '" +
                                    symbols_.NameFor(f->Declaration()->symbol) + "'",
                                f->Declaration()->source);
                    });
                    AddNote("called by entry point '" +
                                symbols_.NameFor(ep->Declaration()->symbol) + "'",
                            ep->Declaration()->source);
                }
                AddNote("second 'push_constant' variable declaration is here",
                        push_constant_var->Declaration()->source);
                if (push_constant_func != ep) {
                    TraverseCallChain(
                        diagnostics_, ep, push_constant_func, [&](const sem::Function* f) {
                            AddNote("called by function '" +
                                        symbols_.NameFor(f->Declaration()->symbol) + "'",
                                    f->Declaration()->source);
                        });
                    AddNote("called by entry point '" +
                                symbols_.NameFor(ep->Declaration()->symbol) + "'",
                            ep->Declaration()->source);
                }
                return false;
            }

            return true;
        };

        if (!check_push_constant(entry_point, entry_point)) {
            return false;
        }
        for (auto* func : entry_point->TransitivelyCalledFunctions()) {
            if (!check_push_constant(func, entry_point)) {
                return false;
            }
        }
    }

    return true;
}

bool Validator::Array(const sem::Array* arr, const Source& el_source) const {
    auto* el_ty = arr->ElemType();

    if (!IsPlain(el_ty)) {
        AddError(sem_.TypeNameOf(el_ty) + " cannot be used as an element type of an array",
                 el_source);
        return false;
    }

    if (!IsFixedFootprint(el_ty)) {
        AddError("an array element type cannot contain a runtime-sized array", el_source);
        return false;
    }

    if (IsArrayWithOverrideCount(el_ty)) {
        RaiseArrayWithOverrideCountError(el_source);
        return false;
    }

    return true;
}

bool Validator::ArrayStrideAttribute(const ast::StrideAttribute* attr,
                                     uint32_t el_size,
                                     uint32_t el_align) const {
    auto stride = attr->stride;
    bool is_valid_stride = (stride >= el_size) && (stride >= el_align) && (stride % el_align == 0);
    if (!is_valid_stride) {
        // https://gpuweb.github.io/gpuweb/wgsl/#array-layout-rules
        // Arrays decorated with the stride attribute must have a stride that is
        // at least the size of the element type, and be a multiple of the
        // element type's alignment value.
        AddError(
            "arrays decorated with the stride attribute must have a stride that is at least the "
            "size of the element type, and be a multiple of the element type's alignment value",
            attr->source);
        return false;
    }
    return true;
}

bool Validator::Alias(const ast::Alias*) const {
    return true;
}

bool Validator::Structure(const sem::Struct* str, ast::PipelineStage stage) const {
    if (str->Members().empty()) {
        AddError("structures must have at least one member", str->Declaration()->source);
        return false;
    }

    utils::Hashset<uint32_t, 8> locations;
    for (auto* member : str->Members()) {
        if (auto* r = member->Type()->As<sem::Array>()) {
            if (r->IsRuntimeSized()) {
                if (member != str->Members().back()) {
                    AddError("runtime arrays may only appear as the last member of a struct",
                             member->Declaration()->source);
                    return false;
                }
            }

            if (IsArrayWithOverrideCount(member->Type())) {
                RaiseArrayWithOverrideCountError(member->Declaration()->type->source);
                return false;
            }
        } else if (!IsFixedFootprint(member->Type())) {
            AddError(
                "a struct that contains a runtime array cannot be nested inside another struct",
                member->Declaration()->source);
            return false;
        }

        auto has_location = false;
        auto has_position = false;
        const ast::InvariantAttribute* invariant_attribute = nullptr;
        const ast::InterpolateAttribute* interpolate_attribute = nullptr;
        for (auto* attr : member->Declaration()->attributes) {
            bool ok = Switch(
                attr,  //
                [&](const ast::InvariantAttribute* invariant) {
                    invariant_attribute = invariant;
                    return true;
                },
                [&](const ast::LocationAttribute* location) {
                    has_location = true;
                    TINT_ASSERT(Resolver, member->Location().has_value());
                    if (!LocationAttribute(location, member->Location().value(), member->Type(),
                                           locations, stage, member->Declaration()->source)) {
                        return false;
                    }
                    return true;
                },
                [&](const ast::BuiltinAttribute* builtin) {
                    if (!BuiltinAttribute(builtin, member->Type(), stage,
                                          /* is_input */ false)) {
                        return false;
                    }
                    if (builtin->builtin == ast::BuiltinValue::kPosition) {
                        has_position = true;
                    }
                    return true;
                },
                [&](const ast::InterpolateAttribute* interpolate) {
                    interpolate_attribute = interpolate;
                    if (!InterpolateAttribute(interpolate, member->Type())) {
                        return false;
                    }
                    return true;
                },
                [&](const ast::StructMemberSizeAttribute*) {
                    if (!member->Type()->HasCreationFixedFootprint()) {
                        AddError(
                            "@size can only be applied to members where the member's type size "
                            "can be fully determined at shader creation time",
                            attr->source);
                        return false;
                    }
                    return true;
                },
                [&](Default) {
                    if (!attr->IsAnyOf<ast::BuiltinAttribute,             //
                                       ast::InternalAttribute,            //
                                       ast::InterpolateAttribute,         //
                                       ast::InvariantAttribute,           //
                                       ast::LocationAttribute,            //
                                       ast::StructMemberOffsetAttribute,  //
                                       ast::StructMemberAlignAttribute>()) {
                        if (attr->Is<ast::StrideAttribute>() &&
                            IsValidationDisabled(member->Declaration()->attributes,
                                                 ast::DisabledValidation::kIgnoreStrideAttribute)) {
                            return true;
                        }
                        AddError("attribute is not valid for structure members", attr->source);
                        return false;
                    }
                    return true;
                });
            if (!ok) {
                return false;
            }
        }

        if (invariant_attribute && !has_position) {
            AddError("invariant attribute must only be applied to a position builtin",
                     invariant_attribute->source);
            return false;
        }

        if (interpolate_attribute && !has_location) {
            AddError("interpolate attribute must only be used with @location",
                     interpolate_attribute->source);
            return false;
        }
    }

    for (auto* attr : str->Declaration()->attributes) {
        if (!(attr->IsAnyOf<ast::InternalAttribute>())) {
            AddError("attribute is not valid for struct declarations", attr->source);
            return false;
        }
    }

    return true;
}

bool Validator::LocationAttribute(const ast::LocationAttribute* loc_attr,
                                  uint32_t location,
                                  const sem::Type* type,
                                  utils::Hashset<uint32_t, 8>& locations,
                                  ast::PipelineStage stage,
                                  const Source& source,
                                  const bool is_input) const {
    std::string inputs_or_output = is_input ? "inputs" : "output";
    if (stage == ast::PipelineStage::kCompute) {
        AddError("attribute is not valid for compute shader " + inputs_or_output, loc_attr->source);
        return false;
    }

    if (!type->is_numeric_scalar_or_vector()) {
        std::string invalid_type = sem_.TypeNameOf(type);
        AddError("cannot apply 'location' attribute to declaration of type '" + invalid_type + "'",
                 source);
        AddNote(
            "'location' attribute must only be applied to declarations of numeric scalar or "
            "numeric vector type",
            loc_attr->source);
        return false;
    }

    if (!locations.Add(location)) {
        AddError(attr_to_str(loc_attr, location) + " attribute appears multiple times",
                 loc_attr->source);
        return false;
    }

    return true;
}

bool Validator::Return(const ast::ReturnStatement* ret,
                       const sem::Type* func_type,
                       const sem::Type* ret_type,
                       sem::Statement* current_statement) const {
    if (func_type->UnwrapRef() != ret_type) {
        AddError("return statement type must match its function return type, returned '" +
                     sem_.TypeNameOf(ret_type) + "', expected '" + sem_.TypeNameOf(func_type) + "'",
                 ret->source);
        return false;
    }

    auto* sem = sem_.Get(ret);
    if (auto* continuing = ClosestContinuing(/*stop_at_loop*/ false, current_statement)) {
        AddError("continuing blocks must not contain a return statement", ret->source);
        if (continuing != sem->Declaration() && continuing != sem->Parent()->Declaration()) {
            AddNote("see continuing block here", continuing->source);
        }
        return false;
    }

    return true;
}

bool Validator::SwitchStatement(const ast::SwitchStatement* s) {
    auto* cond_ty = sem_.TypeOf(s->condition)->UnwrapRef();
    if (!cond_ty->is_integer_scalar()) {
        AddError("switch statement selector expression must be of a scalar integer type",
                 s->condition->source);
        return false;
    }

    const sem::CaseSelector* default_selector = nullptr;
    utils::Hashmap<int64_t, Source, 4> selectors;

    for (auto* case_stmt : s->body) {
        auto* case_sem = sem_.Get<sem::CaseStatement>(case_stmt);
        for (auto* selector : case_sem->Selectors()) {
            if (selector->IsDefault()) {
                if (default_selector != nullptr) {
                    // More than one default clause
                    AddError("switch statement must have exactly one default clause",
                             selector->Declaration()->source);

                    AddNote("previous default case", default_selector->Declaration()->source);
                    return false;
                }
                default_selector = selector;
                continue;
            }

            auto* decl_ty = selector->Value()->Type();
            if (cond_ty != decl_ty) {
                AddError(
                    "the case selector values must have the same type as the selector expression.",
                    selector->Declaration()->source);
                return false;
            }

            auto value = selector->Value()->As<uint32_t>();
            if (auto added = selectors.Add(value, selector->Declaration()->source); !added) {
                AddError("duplicate switch case '" +
                             (decl_ty->IsAnyOf<sem::I32, sem::AbstractNumeric>()
                                  ? std::to_string(i32(value))
                                  : std::to_string(value)) +
                             "'",
                         selector->Declaration()->source);
                AddNote("previous case declared here", *added.value);
                return false;
            }
        }
    }

    if (default_selector == nullptr) {
        // No default clause
        AddError("switch statement must have a default clause", s->source);
        return false;
    }

    return true;
}

bool Validator::Assignment(const ast::Statement* a, const sem::Type* rhs_ty) const {
    const ast::Expression* lhs;
    const ast::Expression* rhs;
    if (auto* assign = a->As<ast::AssignmentStatement>()) {
        lhs = assign->lhs;
        rhs = assign->rhs;
    } else if (auto* compound = a->As<ast::CompoundAssignmentStatement>()) {
        lhs = compound->lhs;
        rhs = compound->rhs;
    } else {
        TINT_ICE(Resolver, diagnostics_) << "invalid assignment statement";
        return false;
    }

    if (lhs->Is<ast::PhonyExpression>()) {
        // https://www.w3.org/TR/WGSL/#phony-assignment-section
        auto* ty = rhs_ty->UnwrapRef();
        if (!ty->IsConstructible() &&
            !ty->IsAnyOf<sem::Pointer, sem::Texture, sem::Sampler, sem::AbstractNumeric>()) {
            AddError("cannot assign '" + sem_.TypeNameOf(rhs_ty) +
                         "' to '_'. '_' can only be assigned a constructible, pointer, texture or "
                         "sampler type",
                     rhs->source);
            return false;
        }
        return true;  // RHS can be anything.
    }

    // https://gpuweb.github.io/gpuweb/wgsl/#assignment-statement
    auto const* lhs_ty = sem_.TypeOf(lhs);

    if (auto* variable = sem_.ResolvedSymbol<sem::Variable>(lhs)) {
        auto* v = variable->Declaration();
        const char* err = Switch(
            v,  //
            [&](const ast::Parameter*) { return "cannot assign to function parameter"; },
            [&](const ast::Let*) { return "cannot assign to 'let'"; },
            [&](const ast::Override*) { return "cannot assign to 'override'"; });
        if (err) {
            AddError(err, lhs->source);
            AddNote("'" + symbols_.NameFor(v->symbol) + "' is declared here:", v->source);
            return false;
        }
    }

    auto* lhs_ref = lhs_ty->As<sem::Reference>();
    if (!lhs_ref) {
        // LHS is not a reference, so it has no storage.
        AddError("cannot assign to value of type '" + sem_.TypeNameOf(lhs_ty) + "'", lhs->source);
        return false;
    }

    auto* storage_ty = lhs_ref->StoreType();
    auto* value_type = rhs_ty->UnwrapRef();  // Implicit load of RHS

    // Value type has to match storage type
    if (storage_ty != value_type) {
        AddError(
            "cannot assign '" + sem_.TypeNameOf(rhs_ty) + "' to '" + sem_.TypeNameOf(lhs_ty) + "'",
            a->source);
        return false;
    }
    if (!storage_ty->IsConstructible()) {
        AddError("storage type of assignment must be constructible", a->source);
        return false;
    }
    if (lhs_ref->Access() == ast::Access::kRead) {
        AddError("cannot store into a read-only type '" + sem_.RawTypeNameOf(lhs_ty) + "'",
                 a->source);
        return false;
    }
    return true;
}

bool Validator::IncrementDecrementStatement(const ast::IncrementDecrementStatement* inc) const {
    const ast::Expression* lhs = inc->lhs;

    // https://gpuweb.github.io/gpuweb/wgsl/#increment-decrement

    if (auto* variable = sem_.ResolvedSymbol<sem::Variable>(lhs)) {
        auto* v = variable->Declaration();
        const char* err = Switch(
            v,  //
            [&](const ast::Parameter*) { return "cannot modify function parameter"; },
            [&](const ast::Let*) { return "cannot modify 'let'"; },
            [&](const ast::Override*) { return "cannot modify 'override'"; });
        if (err) {
            AddError(err, lhs->source);
            AddNote("'" + symbols_.NameFor(v->symbol) + "' is declared here:", v->source);
            return false;
        }
    }

    auto const* lhs_ty = sem_.TypeOf(lhs);
    auto* lhs_ref = lhs_ty->As<sem::Reference>();
    if (!lhs_ref) {
        // LHS is not a reference, so it has no storage.
        AddError("cannot modify value of type '" + sem_.TypeNameOf(lhs_ty) + "'", lhs->source);
        return false;
    }

    if (!lhs_ref->StoreType()->is_integer_scalar()) {
        const std::string kind = inc->increment ? "increment" : "decrement";
        AddError(kind + " statement can only be applied to an integer scalar", lhs->source);
        return false;
    }

    if (lhs_ref->Access() == ast::Access::kRead) {
        AddError("cannot modify read-only type '" + sem_.RawTypeNameOf(lhs_ty) + "'", inc->source);
        return false;
    }
    return true;
}

bool Validator::NoDuplicateAttributes(utils::VectorRef<const ast::Attribute*> attributes) const {
    utils::Hashmap<const TypeInfo*, Source, 8> seen;
    for (auto* d : attributes) {
        auto added = seen.Add(&d->TypeInfo(), d->source);
        if (!added && !d->Is<ast::InternalAttribute>()) {
            AddError("duplicate " + d->Name() + " attribute", d->source);
            AddNote("first attribute declared here", *added.value);
            return false;
        }
    }
    return true;
}

bool Validator::IsValidationDisabled(utils::VectorRef<const ast::Attribute*> attributes,
                                     ast::DisabledValidation validation) const {
    for (auto* attribute : attributes) {
        if (auto* dv = attribute->As<ast::DisableValidationAttribute>()) {
            if (dv->validation == validation) {
                return true;
            }
        }
    }
    return false;
}

bool Validator::IsValidationEnabled(utils::VectorRef<const ast::Attribute*> attributes,
                                    ast::DisabledValidation validation) const {
    return !IsValidationDisabled(attributes, validation);
}

bool Validator::IsArrayWithOverrideCount(const sem::Type* ty) const {
    if (auto* arr = ty->UnwrapRef()->As<sem::Array>()) {
        if (arr->IsOverrideSized()) {
            return true;
        }
    }
    return false;
}

void Validator::RaiseArrayWithOverrideCountError(const Source& source) const {
    AddError(
        "array with an 'override' element count can only be used as the store type of a "
        "'var<workgroup>'",
        source);
}

std::string Validator::VectorPretty(uint32_t size, const sem::Type* element_type) const {
    sem::Vector vec_type(element_type, size);
    return vec_type.FriendlyName(symbols_);
}

}  // namespace tint::resolver
