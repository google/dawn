// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or stateied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/lang/spirv/reader/ast_parser/type.h"

#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/containers/unique_allocator.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Type);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Void);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Bool);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::U32);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::F32);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::I32);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Pointer);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Reference);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Vector);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Matrix);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Array);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Sampler);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Texture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::DepthTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::DepthMultisampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::MultisampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::SampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::StorageTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Named);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Alias);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::Struct);

namespace tint::spirv::reader {

namespace {
struct PointerHasher {
    size_t operator()(const Pointer& t) const { return Hash(t.address_space, t.type, t.access); }
};

struct ReferenceHasher {
    size_t operator()(const Reference& t) const { return Hash(t.address_space, t.type, t.access); }
};

struct VectorHasher {
    size_t operator()(const Vector& t) const { return Hash(t.type, t.size); }
};

struct MatrixHasher {
    size_t operator()(const Matrix& t) const { return Hash(t.type, t.columns, t.rows); }
};

struct ArrayHasher {
    size_t operator()(const Array& t) const { return Hash(t.type, t.size, t.stride); }
};

struct AliasHasher {
    size_t operator()(const Alias& t) const { return Hash(t.name); }
};

struct StructHasher {
    size_t operator()(const Struct& t) const { return Hash(t.name); }
};

struct SamplerHasher {
    size_t operator()(const Sampler& s) const { return Hash(s.kind); }
};

struct DepthTextureHasher {
    size_t operator()(const DepthTexture& t) const { return Hash(t.dims); }
};

struct DepthMultisampledTextureHasher {
    size_t operator()(const DepthMultisampledTexture& t) const { return Hash(t.dims); }
};

struct MultisampledTextureHasher {
    size_t operator()(const MultisampledTexture& t) const { return Hash(t.dims, t.type); }
};

struct SampledTextureHasher {
    size_t operator()(const SampledTexture& t) const { return Hash(t.dims, t.type); }
};

struct StorageTextureHasher {
    size_t operator()(const StorageTexture& t) const { return Hash(t.dims, t.format, t.access); }
};
}  // namespace

// Equality operators
//! @cond Doxygen_Suppress
static bool operator==(const Pointer& a, const Pointer& b) {
    return a.type == b.type && a.address_space == b.address_space && a.access == b.access;
}
static bool operator==(const Reference& a, const Reference& b) {
    return a.type == b.type && a.address_space == b.address_space && a.access == b.access;
}
static bool operator==(const Vector& a, const Vector& b) {
    return a.type == b.type && a.size == b.size;
}
static bool operator==(const Matrix& a, const Matrix& b) {
    return a.type == b.type && a.columns == b.columns && a.rows == b.rows;
}
static bool operator==(const Array& a, const Array& b) {
    return a.type == b.type && a.size == b.size && a.stride == b.stride;
}
static bool operator==(const Named& a, const Named& b) {
    return a.name == b.name;
}
static bool operator==(const Sampler& a, const Sampler& b) {
    return a.kind == b.kind;
}
static bool operator==(const DepthTexture& a, const DepthTexture& b) {
    return a.dims == b.dims;
}
static bool operator==(const DepthMultisampledTexture& a, const DepthMultisampledTexture& b) {
    return a.dims == b.dims;
}
static bool operator==(const MultisampledTexture& a, const MultisampledTexture& b) {
    return a.dims == b.dims && a.type == b.type;
}
static bool operator==(const SampledTexture& a, const SampledTexture& b) {
    return a.dims == b.dims && a.type == b.type;
}
static bool operator==(const StorageTexture& a, const StorageTexture& b) {
    return a.dims == b.dims && a.format == b.format;
}
//! @endcond

ast::Type Void::Build(ProgramBuilder& b) const {
    return b.ty.void_();
}

ast::Type Bool::Build(ProgramBuilder& b) const {
    return b.ty.bool_();
}

ast::Type U32::Build(ProgramBuilder& b) const {
    return b.ty.u32();
}

ast::Type F32::Build(ProgramBuilder& b) const {
    return b.ty.f32();
}

ast::Type I32::Build(ProgramBuilder& b) const {
    return b.ty.i32();
}

Type::Type() = default;
Type::Type(const Type&) = default;
Type::~Type() = default;

Texture::~Texture() = default;

Pointer::Pointer(core::AddressSpace s, const Type* t, core::Access a)
    : address_space(s), type(t), access(a) {}
Pointer::Pointer(const Pointer&) = default;

ast::Type Pointer::Build(ProgramBuilder& b) const {
    auto store_type = type->Build(b);
    if (!store_type) {
        // TODO(crbug.com/tint/1838): We should not be constructing pointers with 'void' store
        // types.
        return b.ty("invalid_spirv_ptr_type");
    }
    return b.ty.ptr(address_space, type->Build(b), access);
}

Reference::Reference(core::AddressSpace s, const Type* t, core::Access a)
    : address_space(s), type(t), access(a) {}
Reference::Reference(const Reference&) = default;

ast::Type Reference::Build(ProgramBuilder& b) const {
    return type->Build(b);
}

Vector::Vector(const Type* t, uint32_t s) : type(t), size(s) {}
Vector::Vector(const Vector&) = default;

ast::Type Vector::Build(ProgramBuilder& b) const {
    auto prefix = "vec" + std::to_string(size);
    return Switch(
        type,  //
        [&](const I32*) { return b.ty(prefix + "i"); },
        [&](const U32*) { return b.ty(prefix + "u"); },
        [&](const F32*) { return b.ty(prefix + "f"); },
        [&](Default) { return b.ty.vec(type->Build(b), size); });
}

Matrix::Matrix(const Type* t, uint32_t c, uint32_t r) : type(t), columns(c), rows(r) {}
Matrix::Matrix(const Matrix&) = default;

ast::Type Matrix::Build(ProgramBuilder& b) const {
    if (type->Is<F32>()) {
        std::ostringstream ss;
        ss << "mat" << columns << "x" << rows << "f";
        return b.ty(ss.str());
    }
    return b.ty.mat(type->Build(b), columns, rows);
}

Array::Array(const Type* t, uint32_t sz, uint32_t st) : type(t), size(sz), stride(st) {}
Array::Array(const Array&) = default;

ast::Type Array::Build(ProgramBuilder& b) const {
    if (size > 0) {
        if (stride > 0) {
            return b.ty.array(type->Build(b), u32(size), tint::Vector{b.Stride(stride)});
        } else {
            return b.ty.array(type->Build(b), u32(size));
        }
    } else {
        if (stride > 0) {
            return b.ty.array(type->Build(b), tint::Vector{b.Stride(stride)});
        } else {
            return b.ty.array(type->Build(b));
        }
    }
}

Sampler::Sampler(type::SamplerKind k) : kind(k) {}
Sampler::Sampler(const Sampler&) = default;

ast::Type Sampler::Build(ProgramBuilder& b) const {
    return b.ty.sampler(kind);
}

Texture::Texture(type::TextureDimension d) : dims(d) {}
Texture::Texture(const Texture&) = default;

DepthTexture::DepthTexture(type::TextureDimension d) : Base(d) {}
DepthTexture::DepthTexture(const DepthTexture&) = default;

ast::Type DepthTexture::Build(ProgramBuilder& b) const {
    return b.ty.depth_texture(dims);
}

DepthMultisampledTexture::DepthMultisampledTexture(type::TextureDimension d) : Base(d) {}
DepthMultisampledTexture::DepthMultisampledTexture(const DepthMultisampledTexture&) = default;

ast::Type DepthMultisampledTexture::Build(ProgramBuilder& b) const {
    return b.ty.depth_multisampled_texture(dims);
}

MultisampledTexture::MultisampledTexture(type::TextureDimension d, const Type* t)
    : Base(d), type(t) {}
MultisampledTexture::MultisampledTexture(const MultisampledTexture&) = default;

ast::Type MultisampledTexture::Build(ProgramBuilder& b) const {
    return b.ty.multisampled_texture(dims, type->Build(b));
}

SampledTexture::SampledTexture(type::TextureDimension d, const Type* t) : Base(d), type(t) {}
SampledTexture::SampledTexture(const SampledTexture&) = default;

ast::Type SampledTexture::Build(ProgramBuilder& b) const {
    return b.ty.sampled_texture(dims, type->Build(b));
}

StorageTexture::StorageTexture(type::TextureDimension d, core::TexelFormat f, core::Access a)
    : Base(d), format(f), access(a) {}
StorageTexture::StorageTexture(const StorageTexture&) = default;

ast::Type StorageTexture::Build(ProgramBuilder& b) const {
    return b.ty.storage_texture(dims, format, access);
}

Named::Named(Symbol n) : name(n) {}
Named::Named(const Named&) = default;
Named::~Named() = default;

Alias::Alias(Symbol n, const Type* ty) : Base(n), type(ty) {}
Alias::Alias(const Alias&) = default;

ast::Type Alias::Build(ProgramBuilder& b) const {
    return b.ty(name);
}

Struct::Struct(Symbol n, TypeList m) : Base(n), members(std::move(m)) {}
Struct::Struct(const Struct&) = default;
Struct::~Struct() = default;

ast::Type Struct::Build(ProgramBuilder& b) const {
    return b.ty(name);
}

/// The PIMPL state of the Types object.
struct TypeManager::State {
    /// The allocator of primitive types
    BlockAllocator<Type> allocator_;
    /// The lazily-created Void type
    reader::Void const* void_ = nullptr;
    /// The lazily-created Bool type
    reader::Bool const* bool_ = nullptr;
    /// The lazily-created U32 type
    reader::U32 const* u32_ = nullptr;
    /// The lazily-created F32 type
    reader::F32 const* f32_ = nullptr;
    /// The lazily-created I32 type
    reader::I32 const* i32_ = nullptr;
    /// Unique Pointer instances
    UniqueAllocator<reader::Pointer, PointerHasher> pointers_;
    /// Unique Reference instances
    UniqueAllocator<reader::Reference, ReferenceHasher> references_;
    /// Unique Vector instances
    UniqueAllocator<reader::Vector, VectorHasher> vectors_;
    /// Unique Matrix instances
    UniqueAllocator<reader::Matrix, MatrixHasher> matrices_;
    /// Unique Array instances
    UniqueAllocator<reader::Array, ArrayHasher> arrays_;
    /// Unique Alias instances
    UniqueAllocator<reader::Alias, AliasHasher> aliases_;
    /// Unique Struct instances
    UniqueAllocator<reader::Struct, StructHasher> structs_;
    /// Unique Sampler instances
    UniqueAllocator<reader::Sampler, SamplerHasher> samplers_;
    /// Unique DepthTexture instances
    UniqueAllocator<reader::DepthTexture, DepthTextureHasher> depth_textures_;
    /// Unique DepthMultisampledTexture instances
    UniqueAllocator<reader::DepthMultisampledTexture, DepthMultisampledTextureHasher>
        depth_multisampled_textures_;
    /// Unique MultisampledTexture instances
    UniqueAllocator<reader::MultisampledTexture, MultisampledTextureHasher> multisampled_textures_;
    /// Unique SampledTexture instances
    UniqueAllocator<reader::SampledTexture, SampledTextureHasher> sampled_textures_;
    /// Unique StorageTexture instances
    UniqueAllocator<reader::StorageTexture, StorageTextureHasher> storage_textures_;
};

const Type* Type::UnwrapPtr() const {
    const Type* type = this;
    while (auto* ptr = type->As<Pointer>()) {
        type = ptr->type;
    }
    return type;
}

const Type* Type::UnwrapRef() const {
    const Type* type = this;
    while (auto* ptr = type->As<Reference>()) {
        type = ptr->type;
    }
    return type;
}

const Type* Type::UnwrapAlias() const {
    const Type* type = this;
    while (auto* alias = type->As<Alias>()) {
        type = alias->type;
    }
    return type;
}

const Type* Type::UnwrapAll() const {
    auto* type = this;
    while (true) {
        if (auto* alias = type->As<Alias>()) {
            type = alias->type;
        } else if (auto* ptr = type->As<Pointer>()) {
            type = ptr->type;
        } else {
            break;
        }
    }
    return type;
}

bool Type::IsFloatScalar() const {
    return Is<F32>();
}

bool Type::IsFloatScalarOrVector() const {
    return IsFloatScalar() || IsFloatVector();
}

bool Type::IsFloatVector() const {
    return Is([](const Vector* v) { return v->type->IsFloatScalar(); });
}

bool Type::IsIntegerScalar() const {
    return IsAnyOf<U32, I32>();
}

bool Type::IsIntegerScalarOrVector() const {
    return IsUnsignedScalarOrVector() || IsSignedScalarOrVector();
}

bool Type::IsScalar() const {
    return IsAnyOf<F32, U32, I32, Bool>();
}

bool Type::IsSignedIntegerVector() const {
    return Is([](const Vector* v) { return v->type->Is<I32>(); });
}

bool Type::IsSignedScalarOrVector() const {
    return Is<I32>() || IsSignedIntegerVector();
}

bool Type::IsUnsignedIntegerVector() const {
    return Is([](const Vector* v) { return v->type->Is<U32>(); });
}

bool Type::IsUnsignedScalarOrVector() const {
    return Is<U32>() || IsUnsignedIntegerVector();
}

TypeManager::TypeManager() {
    state = std::make_unique<State>();
}

TypeManager::~TypeManager() = default;

const reader::Void* TypeManager::Void() {
    if (!state->void_) {
        state->void_ = state->allocator_.Create<reader::Void>();
    }
    return state->void_;
}

const reader::Bool* TypeManager::Bool() {
    if (!state->bool_) {
        state->bool_ = state->allocator_.Create<reader::Bool>();
    }
    return state->bool_;
}

const reader::U32* TypeManager::U32() {
    if (!state->u32_) {
        state->u32_ = state->allocator_.Create<reader::U32>();
    }
    return state->u32_;
}

const reader::F32* TypeManager::F32() {
    if (!state->f32_) {
        state->f32_ = state->allocator_.Create<reader::F32>();
    }
    return state->f32_;
}

const reader::I32* TypeManager::I32() {
    if (!state->i32_) {
        state->i32_ = state->allocator_.Create<reader::I32>();
    }
    return state->i32_;
}

const Type* TypeManager::AsUnsigned(const Type* ty) {
    return Switch(
        ty,                                         //
        [&](const reader::I32*) { return U32(); },  //
        [&](const reader::U32*) { return ty; },     //
        [&](const reader::Vector* vec) {
            return Switch(
                vec->type,                                                     //
                [&](const reader::I32*) { return Vector(U32(), vec->size); },  //
                [&](const reader::U32*) { return ty; }                         //
            );
        });
}

const reader::Pointer* TypeManager::Pointer(core::AddressSpace address_space,
                                            const Type* el,
                                            core::Access access) {
    return state->pointers_.Get(address_space, el, access);
}

const reader::Reference* TypeManager::Reference(core::AddressSpace address_space,
                                                const Type* el,
                                                core::Access access) {
    return state->references_.Get(address_space, el, access);
}

const reader::Vector* TypeManager::Vector(const Type* el, uint32_t size) {
    return state->vectors_.Get(el, size);
}

const reader::Matrix* TypeManager::Matrix(const Type* el, uint32_t columns, uint32_t rows) {
    return state->matrices_.Get(el, columns, rows);
}

const reader::Array* TypeManager::Array(const Type* el, uint32_t size, uint32_t stride) {
    return state->arrays_.Get(el, size, stride);
}

const reader::Alias* TypeManager::Alias(Symbol name, const Type* ty) {
    return state->aliases_.Get(name, ty);
}

const reader::Struct* TypeManager::Struct(Symbol name, TypeList members) {
    return state->structs_.Get(name, std::move(members));
}

const reader::Sampler* TypeManager::Sampler(type::SamplerKind kind) {
    return state->samplers_.Get(kind);
}

const reader::DepthTexture* TypeManager::DepthTexture(type::TextureDimension dims) {
    return state->depth_textures_.Get(dims);
}

const reader::DepthMultisampledTexture* TypeManager::DepthMultisampledTexture(
    type::TextureDimension dims) {
    return state->depth_multisampled_textures_.Get(dims);
}

const reader::MultisampledTexture* TypeManager::MultisampledTexture(type::TextureDimension dims,
                                                                    const Type* ty) {
    return state->multisampled_textures_.Get(dims, ty);
}

const reader::SampledTexture* TypeManager::SampledTexture(type::TextureDimension dims,
                                                          const Type* ty) {
    return state->sampled_textures_.Get(dims, ty);
}

const reader::StorageTexture* TypeManager::StorageTexture(type::TextureDimension dims,
                                                          core::TexelFormat fmt,
                                                          core::Access access) {
    return state->storage_textures_.Get(dims, fmt, access);
}

// Debug String() methods for Type classes. Only enabled in debug builds.
#ifndef NDEBUG
std::string Void::String() const {
    return "void";
}

std::string Bool::String() const {
    return "bool";
}

std::string U32::String() const {
    return "u32";
}

std::string F32::String() const {
    return "f32";
}

std::string I32::String() const {
    return "i32";
}

std::string Pointer::String() const {
    StringStream ss;
    ss << "ptr<" << tint::ToString(address_space) << ", " << type->String() + ">";
    return ss.str();
}

std::string Reference::String() const {
    StringStream ss;
    ss << "ref<" + tint::ToString(address_space) << ", " << type->String() << ">";
    return ss.str();
}

std::string Vector::String() const {
    StringStream ss;
    ss << "vec" << size << "<" << type->String() << ">";
    return ss.str();
}

std::string Matrix::String() const {
    StringStream ss;
    ss << "mat" << columns << "x" << rows << "<" << type->String() << ">";
    return ss.str();
}

std::string Array::String() const {
    StringStream ss;
    ss << "array<" << type->String() << ", " << size << ", " << stride << ">";
    return ss.str();
}

std::string Sampler::String() const {
    switch (kind) {
        case type::SamplerKind::kSampler:
            return "sampler";
        case type::SamplerKind::kComparisonSampler:
            return "sampler_comparison";
    }
    return "<unknown sampler>";
}

std::string DepthTexture::String() const {
    StringStream ss;
    ss << "depth_" << dims;
    return ss.str();
}

std::string DepthMultisampledTexture::String() const {
    StringStream ss;
    ss << "depth_multisampled_" << dims;
    return ss.str();
}

std::string MultisampledTexture::String() const {
    StringStream ss;
    ss << "texture_multisampled_" << dims << "<" << type << ">";
    return ss.str();
}

std::string SampledTexture::String() const {
    StringStream ss;
    ss << "texture_" << dims << "<" << type << ">";
    return ss.str();
}

std::string StorageTexture::String() const {
    StringStream ss;
    ss << "texture_storage_" << dims << "<" << format << ", " << access << ">";
    return ss.str();
}

std::string Named::String() const {
    return name.to_str();
}
#endif  // NDEBUG

}  // namespace tint::spirv::reader
